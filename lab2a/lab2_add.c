/*
 Name: Sum Yi Li
 Email: sammyli0106@gmail.com
 ID: 505146702
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <sched.h>
#include <stdint.h>
#include <time.h>
#include <sched.h>
#include <pthread.h>

#define one_billion 1000000000

//option flags
int threadFlag = 0;
int iterationFlag = 0;
int yieldFlag = 0;
int syncFlag = 0;
char sync_case;

//synchronization flags
int mutexFlag = 0;
int spinlockFlag = 0;
int compareSwapFlag = 0;
pthread_mutex_t mutexLock;
int spin_lock_var = 0;
int compare_and_swap_var = 0;

//check for errors
int check = 0;

//parameter for the number of parallel threads, iterations
//default is 1 start off
int numThread = 1;
int numIterations = 1;

//initializes a counter to zero
long long counter = 0;

char tag[20] = "";

//clock get time errors
void clock_gettime_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "clock_gettime() failed");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void pthread_mutex_init_check(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "pthread_mutex_init_() failed");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }

}

void malloc_error(pthread_t  *threads_ID)
{
    if (threads_ID == NULL)
    {
        fprintf(stderr, "malloc() failed");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void pthread_create_error(int rc)
{
    if (rc) {
        fprintf(stderr, "pthread_create() failed");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void pthread_join_error(int rc)
{
    if (rc)
    {
        fprintf(stderr, "pthread_join() failed");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void yield_func()
{
    if (yieldFlag == 1)
    {
        sched_yield();
    }
}

long long calculate_sum(long long *pointer, long long value)
{
    long long sum_val = *pointer + value;
    yield_func();
    
    return sum_val;
}

void add_func(long long *pointer, long long value)
{
    *pointer = calculate_sum(pointer, value);
}

//Based from reosurces : https://docs.oracle.com/cd/E19455-01/806-5257/sync-12/index.html
void add_one_mutex()
{
    pthread_mutex_lock(&mutexLock);
    add_func(&counter, 1);
    pthread_mutex_unlock(&mutexLock);
}

//Based from resources : https://attractivechaos.wordpress.com/2011/10/06/multi-threaded-programming-efficiency-of-locking/
void add_one_spinLock()
{
    while(__sync_lock_test_and_set(&spin_lock_var, 1));
    add_func(&counter, 1);
    __sync_lock_release(&spin_lock_var);
}

long long add_one_compareSwap_calculate(long long oldVal)
{
    long long sum = oldVal + 1;
    return sum;
}

//Based from resources : http://pages.cs.wisc.edu/~remzi/OSTEP/threads-locks.pdf
void add_one_compareSwap()
{
    long long oldVal = 0;
    long long newVal = 0;
    
    do{
        oldVal = counter;
        newVal = add_one_compareSwap_calculate(oldVal);
        yield_func();
        
    }while (__sync_val_compare_and_swap(&counter, oldVal, newVal) != oldVal);
}

void add_one_func()
{
    //add 1 to the counter the specified number of times
    for (int i = 0; i < numIterations; i++)
    {
        //four cases
        if (mutexFlag == 1)
        {
            add_one_mutex();
        }
        else if (spinlockFlag == 1)
        {
            add_one_spinLock();
        }
        else if (compareSwapFlag == 1)
        {
            add_one_compareSwap();
        }
        else
        {
            //none case, do nothing, race condition here
            add_func(&counter, 1);
        }
    }
}

//Based from reosurces : https://docs.oracle.com/cd/E19455-01/806-5257/sync-12/index.html
void minus_one_mutex()
{
    pthread_mutex_lock(&mutexLock);
    add_func(&counter, -1);
    pthread_mutex_unlock(&mutexLock);
}

//Based from resources : https://attractivechaos.wordpress.com/2011/10/06/multi-threaded-programming-efficiency-of-locking/
void minus_one_spinLock()
{
    while(__sync_lock_test_and_set(&spin_lock_var, 1));
    add_func(&counter, -1);
    __sync_lock_release(&spin_lock_var);

}

long long minus_one_compareSwap_calculate(long long oldVal)
{
    long long sum = oldVal - 1;
    return sum;
}

//Based from resources : http://pages.cs.wisc.edu/~remzi/OSTEP/threads-locks.pdf
void minus_one_compareSwap()
{
    long long oldVal = 0;
    long long newVal = 0;
    
    do{
        oldVal = counter;
        newVal = minus_one_compareSwap_calculate(oldVal);
        yield_func();
        
    }while (__sync_val_compare_and_swap(&counter, oldVal, newVal) != oldVal);
}

void minus_one_func()
{
    //add -1 to the counter the specified number of times
    for (int i = 0; i < numIterations; i++)
    {
        //four cases
        if (mutexFlag == 1)
        {
            minus_one_mutex();
        }
        else if (spinlockFlag == 1)
        {
            minus_one_spinLock();
        }
        else if (compareSwapFlag == 1)
        {
            minus_one_compareSwap();
        }
        else
        {
            //none case, do nothing, race condition here
            add_func(&counter, -1);
        }
    }
}

//add thread function
void * thread_add()
{
    add_one_func();
    minus_one_func();
    
    return NULL;
}

//Based from resources : https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/
void mutex_initalize()
{
    if (mutexFlag == 1)
    {
        check = pthread_mutex_init(&mutexLock, NULL);
        pthread_mutex_init_check(check);
    }
}

//Based from resources : https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/
void create_threads(pthread_t  *threads_ID)
{
    int rc;
    for (int i = 0; i < numThread; i++)
    {
        rc = pthread_create(&threads_ID[i], NULL, thread_add, NULL);
        pthread_create_error(rc);
    }
}

//Based from resources : https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/
void join_threads(pthread_t  *threads_ID)
{
    int rc;
    for (int i = 0; i < numThread; i++)
    {
        rc = pthread_join(threads_ID[i], NULL);
        pthread_join_error(rc);
    }
}

void printData()
{
    if (mutexFlag == 1 && yieldFlag == 0){
        strcat(tag, "add-m");
    }
    else if (spinlockFlag == 1 && yieldFlag == 0){
        strcat(tag, "add-s");
    }
    else if (compareSwapFlag == 1 && yieldFlag == 0){
        strcat(tag, "add-c");
    }
    else if (mutexFlag == 1 && yieldFlag == 1){
        strcat(tag, "add-yield-m");
    }
    else if (spinlockFlag == 1 && yieldFlag == 1){
        strcat(tag, "add-yield-s");
    }
    else if (compareSwapFlag == 1 && yieldFlag == 1){
        strcat(tag, "add-yield-c");
    }
    else if (yieldFlag == 1){
        strcat(tag, "add-yield-none");
    }
    else{
        strcat(tag, "add-none");
    }
}

//Based from resources : https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
int main(int argc, char **argv)
{
    //set up four options
    static struct option long_options[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", no_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {0, 0, 0, 0}
    };
    
    int optionIndex = 0;
    
    while ((optionIndex = getopt_long(argc, argv, "t:i:s:y", long_options, NULL)) != -1)
    {
        switch(optionIndex)
        {
            case 't':
                threadFlag = 1;
                numThread = atoi(optarg);
                break;
            case 'i':
                iterationFlag = 1;
                numIterations = atoi(optarg);
                break;
            case 'y':
                yieldFlag = 1;
                break;
            case 's':
                syncFlag = 1;
                sync_case = optarg[0];
                //there are three more cases
                if (sync_case == 'm')
                {
                    //mutex case
                    mutexFlag = 1;
                }
                else if (sync_case == 's')
                {
                    //spin lock case
                    spinlockFlag = 1;
                }
                else if (sync_case == 'c')
                {
                    //compare and swap case
                    compareSwapFlag = 1;
                }
                
                break;
            default:
                //print the unrecognized error message and exit with code 1
                fprintf(stderr, "Unrecognized argument: %s\n. Proper usage : lab2_add-threads=# --iterations=# --yield --sync=m\n", strerror(optionIndex));
                exit(1);
        }
    }
    
    //initialize the variables for mutex
    mutex_initalize();
    
    //Based from resources : https://www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/gettime.html
    //notes the starting time for the run
    struct timespec start, end;
    check = clock_gettime(CLOCK_MONOTONIC, &start);
    clock_gettime_error(check);
    
    //Based from resources : https://stackoverflow.com/questions/26753957/how-to-dynamically-allocateinitialize-a-pthread-array
    //start the specified number of threads with malloc
    pthread_t  *threads_ID = malloc(numThread * sizeof(pthread_t));
    malloc_error(threads_ID);
    
    //create the threads
    create_threads(threads_ID);
    
    //exit to re-join the parent thread, reuse rc to check errors
    join_threads(threads_ID);
    
    //Based from resources : https://www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/gettime.html
    //notes the ending ending time for the run
    check = clock_gettime(CLOCK_MONOTONIC, &end);
    clock_gettime_error(check);
    
    //Based from resources : https://stackoverflow.com/questions/5248915/execution-time-of-c-program
    //calculate the number of operations, total run time
    //average time peroperations, total at the end of the run
    int numOperations = numThread * numIterations * 2;
    long long time_passed = (end.tv_sec - start.tv_sec) * one_billion;
    time_passed = time_passed + end.tv_nsec;
    time_passed = time_passed - start.tv_nsec;
    int average_time = time_passed / numOperations;
    
    //print the CVS content
    printData();
    
    fprintf(stdout, "%s,%d,%d,%d,%lld,%d,%lld\n", tag, numThread, numIterations, numOperations, time_passed, average_time, counter);
    
    //free the threads
    free(threads_ID);
    
    exit(0);
}
