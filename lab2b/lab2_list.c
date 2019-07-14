/*
 Name: Sum Yi Li
 Email: sammyli0106@gmail.com
 ID: 505146702
 */

#include "SortedList.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

#define one_billion 1000000000

//option flags
int threadFlag = 0;
int iterationFlag = 0;
int yieldFlag = 0;
int syncFlag = 0;
char sync_case;

//causing error here
//pthread_mutex_t mutexLock
pthread_mutex_t* mutexLock;

//here also
//int spin_lock_var = 0;
int* spin_lock_var;
int opt_yield = 0;
int listFlag = 0;
//number of list default to one at first
int number_of_lists = 1;
int* sublist_num;

//parameter for the number of parallel threads, iterations
//default is 1 start off
int numThread = 1;
int numIterations = 1;
int numElements = 0;

//synchronization flags
int mutexFlag = 0;
int spinlockFlag = 0;

char yieldOption[15] = "-";
size_t length = 0;

//check for errors
int check = 0;

//declare list and element variables
SortedList_t *list;
SortedListElement_t* listElem;
SortedListElement_t* targetElement;

char CVS[40] = "list";

//additional time variable
long long lock_time_nsecs = 0;

void getLock()
{
    //different lock type
    if (spinlockFlag == 1)
    {
        strcat(CVS, "-s");
    }
    else if (mutexFlag == 1)
    {
        strcat(CVS, "-m");
    }
    else
    {
        strcat(CVS, "-none");
    }
}

void getName()
{
    int length = strlen(yieldOption);
    
    if (length == 1)
    {
        strcat(yieldOption, "none");
    }
    
    strcat(CVS, yieldOption);
    
    getLock();
}

void printData(int numOperations, long long time_passed, int average_operation_time, int average_lock_time)
{
    getName();
    
    printf("%s,%d,%d,%d,%d,%lld,%d,%d\n", CVS, numThread, numIterations, number_of_lists, numOperations, time_passed, average_operation_time, average_lock_time);
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

void strlenError(int length)
{
    if (length > 3)
    {
        fprintf(stderr, "Incorrect number of yielding options. The available number is only 3");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void segFaultHanldeFunc()
{
    fprintf(stderr, "Exit with segmentation fault occurs.");
    fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
    exit(2);
}

//Based from resources : https://www.geeksforgeeks.org/signals-c-language/
void handleSignal()
{
    signal(SIGSEGV, segFaultHanldeFunc);
}

void malloc_error_list(SortedList_t *list)
{
    if (list == NULL)
    {
        fprintf(stderr, "malloc() failed to allocate memory.");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void fill_empty_list(int i)
{
    list[i].prev = &list[i];
    list[i].next = &list[i];
    list[i].key = NULL;
}

void loop_empty_list()
{
    for (int i = 0; i < number_of_lists; i++)
    {
        fill_empty_list(i);
    }
}

//Based from resources : https://www.geeksforgeeks.org/generic-linked-list-in-c-2/
void createEmptyList()
{
    //need to create more sublists
    list = malloc(sizeof(SortedList_t) * number_of_lists);
    malloc_error_list(list);
    
    loop_empty_list();
}

void malloc_error_listElem(SortedListElement_t* listElem)
{
    if (listElem == NULL)
    {
        fprintf(stderr, "malloc() failed to allocate memory.");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

//Based from resources : https://www.geeksforgeeks.org/generic-linked-list-in-c-2/
void createListElements()
{
    numElements = numIterations * numThread;
    listElem = malloc(sizeof(SortedListElement_t) * numElements);
    malloc_error_listElem(listElem);
}

void malloc_error_key(char* randomKey)
{
    if (randomKey == NULL)
    {
        fprintf(stderr, "malloc() failed to allocate memory.");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

//Based from resources : https://stackoverflow.com/questions/19724346/generate-random-characters-in-c
char* randomKeyGenerator(char* randomKey)
{
    int randomLetter = 'a' + (random() % 26);
    
    randomKey[0] = randomLetter;
    randomKey[1] = '\0';
    
    return randomKey;
}

//Based from resources : https://stackoverflow.com/questions/19724346/generate-random-characters-in-c
//Based from resources : https://www.geeksforgeeks.org/rand-and-srand-in-ccpp/
void initializeElement()
{
    //use a different seed value to prevent get the same result each time run the program
    srand(time(NULL));
    //looping through list
    for (int i = 0; i < numElements; i++)
    {
        //allocate space for the keys
        char* randomKey = malloc(sizeof(char) * 2);
        malloc_error_key(randomKey);
        
        listElem[i].key = randomKeyGenerator(randomKey);
    }
}

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

void threadMallocError(pthread_t  *threads)
{
    if (threads == NULL)
    {
        fprintf(stderr, "malloc() failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void threadIDMallocError(int *threadsID)
{
    if (threadsID == NULL)
    {
        fprintf(stderr, "malloc() failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void listLengthError(int listLength)
{
    if (listLength == -1)
    {
        fprintf(stderr, "List is corrupted and cannot find the length of the list\n");
        exit(2);
    }
}

void targetElementError(SortedListElement_t* targetElement)
{
    if (targetElement == NULL)
    {
        fprintf(stderr, "SortedList_lookup failed\n");
        exit(2);
    }
}

void deleteElementError(int check)
{
    if (check == 1)
    {
        fprintf(stderr, "SortedList_delete failed\n");
        exit(2);
    }
}

void mutex_gettime_start_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "clock_gettime() for starting mutex(insert) failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void mutex_gettime_end_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "clock_gettime() for ending mutex(insert) failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void spinLock_gettime_start_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "clock_gettime() for starting spinlock(insert) failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void spinLock_gettime_end_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "clock_gettime() for ending spinlock(insert) failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void clock_get_start_time_mutex(struct timespec start)
{
    //struct timespec start;
    check = clock_gettime(CLOCK_MONOTONIC, &start);
    mutex_gettime_start_error(check);
}

void clock_get_end_time_mutex(struct timespec end)
{
    //struct timespec end;
    check = clock_gettime(CLOCK_MONOTONIC, &end);
    mutex_gettime_end_error(check);
}

void calculate_lock_time(struct timespec start, struct timespec end)
{
    long long time_sec = end.tv_sec - start.tv_sec;
    long long time_nsec = end.tv_nsec - start.tv_nsec;
    
    lock_time_nsecs += one_billion * time_sec + time_nsec;
}

//Based from reosurces : https://docs.oracle.com/cd/E19455-01/806-5257/sync-12/index.html
void list_insert_mutex(int i, SortedListElement_t* listElem, int sublist_head)
{
    //wrap the mutex with clock get_time
    struct timespec start, end;
    
    clock_get_start_time_mutex(start);
    pthread_mutex_lock(&mutexLock[sublist_head]);
    clock_get_end_time_mutex(end);
    
    //find the addtional lock time
    calculate_lock_time(start, end);
    
    SortedList_insert(&list[sublist_head], &listElem[i]);
    pthread_mutex_unlock(&mutexLock[sublist_head]);
}

void clock_get_start_time_spinLock(struct timespec start)
{
    //struct timespec start;
    check = clock_gettime(CLOCK_MONOTONIC, &start);
    spinLock_gettime_start_error(check);
}

void clock_get_end_time_spinLock(struct timespec end)
{
    //struct timespec end;
    check = clock_gettime(CLOCK_MONOTONIC, &end);
    spinLock_gettime_end_error(check);
}

//Based from resources : https://attractivechaos.wordpress.com/2011/10/06/multi-threaded-programming-efficiency-of-locking/
void list_insert_spinLock(int i, SortedListElement_t* listElem, int sublist_head)
{
    //wrap the spin lock with clock get_time
    struct timespec start, end;
    
    clock_get_start_time_spinLock(start);
    while(__sync_lock_test_and_set(&spin_lock_var[sublist_head], 1));
    clock_get_end_time_spinLock(end);
    
    //find the additional lock time
    calculate_lock_time(start, end);
    
    SortedList_insert(&list[sublist_head], &listElem[i]);
    __sync_lock_release(&spin_lock_var[sublist_head]);
}

void insertElement(void* threadsID)
{
    int i = 0;
    
    int sublist_head = 0;
    
    int number = *(int*)threadsID;
    
    for (i = number; i < numElements; i += numThread)
    {
        sublist_head = sublist_num[i];
        
        if (mutexFlag == 1)
        {
            list_insert_mutex(i, listElem, sublist_head);
        }
        else if (spinlockFlag == 1)
        {
            list_insert_spinLock(i, listElem, sublist_head);
        }
        else
        {
            
            SortedList_insert(&list[sublist_head], &listElem[i]);
        }
    }
}

void mutex_length_gettime_start_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "clock_gettime() for starting mutex(length) failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void mutex_length_gettime_end_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "clock_gettime() for ending mutex(length) failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void spinLock_length_gettime_start_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "clock_gettime() for starting spinlock(length) failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void spinLock_length_gettime_end_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "clock_gettime() for ending spinlock(length) failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void calculate_sublength(SortedList_t *list, int listLength, int i, int sub_list_length)
{
    sub_list_length = SortedList_length(&list[i]);
    listLengthError(sub_list_length);
    
    listLength = listLength + sub_list_length;
}


//Based from reosurces : https://docs.oracle.com/cd/E19455-01/806-5257/sync-12/index.html
void list_length_mutex(SortedList_t *list, int listLength, int i, int sub_list_length)
{
    //wrap the mutex lock with clock_gettime
    struct timespec start, end;
    
    check = clock_gettime(CLOCK_MONOTONIC, &start);
    mutex_length_gettime_start_error(check);
    
    pthread_mutex_lock(&mutexLock[i]);
    
    check = clock_gettime(CLOCK_MONOTONIC, &end);
    mutex_length_gettime_end_error(check);
    
    //calculate the lock time
    calculate_lock_time(start, end);
    
    //find the sublength
    calculate_sublength(list, listLength, i, sub_list_length);
    
    //need to add the sublength into the total length
    pthread_mutex_unlock(&mutexLock[i]);
}


//Based from resources : https://attractivechaos.wordpress.com/2011/10/06/multi-threaded-programming-efficiency-of-locking/
void list_length_spinLock(SortedList_t *list, int listLength, int i, int sub_list_length)
{
    //wrap the spinlock with clock_gettime
    struct timespec start, end;
    check = clock_gettime(CLOCK_MONOTONIC, &start);
    spinLock_length_gettime_start_error(check);
    
    while(__sync_lock_test_and_set(&spin_lock_var[i], 1));
    
    check = clock_gettime(CLOCK_MONOTONIC, &end);
    spinLock_length_gettime_end_error(check);
    
    //calculate the lock time
    calculate_lock_time(start, end);
    
    //find the sublength
    calculate_sublength(list, listLength, i, sub_list_length);
    
    __sync_lock_release(&spin_lock_var[i]);
}

void getListLength()
{
    //variable to check the length error
    int listLength = 0;
    int sub_list_length = 0;
    
    for (int i = 0; i < number_of_lists; i++)
    {
        if (mutexFlag == 1)
        {
            list_length_mutex(list, listLength, i, sub_list_length);
        }
        else if (spinlockFlag == 1)
        {
            list_length_spinLock(list, listLength, i, sub_list_length);
        }
        else
        {
            sub_list_length = SortedList_length(&list[i]);
            listLengthError(sub_list_length);
            
            //add up the length from the sublist
            listLength = listLength + sub_list_length;
        }
    }
}

void mutex_delete_gettime_start_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "clock_gettime() for starting mutex(delete) failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void mutex_delete_gettime_end_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "clock_gettime() for ending mutex(delete) failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void spinLock_delete_gettime_start_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "clock_gettime() for starting spinlock(delete) failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void spinLock_delete_gettime_end_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "clock_gettime() for ending spinlock(delete) failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

//Based from reosurces : https://docs.oracle.com/cd/E19455-01/806-5257/sync-12/index.html
void list_delete_mutex(int j, SortedListElement_t* listElem, SortedListElement_t* targetElement, int sublist_head)
{
    //wrap the mutex with clock_gettime
    struct timespec start, end;
    check = clock_gettime(CLOCK_MONOTONIC, &start);
    mutex_delete_gettime_start_error(check);
    
    pthread_mutex_lock(&mutexLock[sublist_head]);
    
    check = clock_gettime(CLOCK_MONOTONIC, &end);
    mutex_delete_gettime_end_error(check);
    
    //calculate the lock time
    calculate_lock_time(start, end);
    
    targetElement = SortedList_lookup(&list[sublist_head], listElem[j].key);
    targetElementError(targetElement);
    
    check = SortedList_delete(targetElement);
    deleteElementError(check);
    
    pthread_mutex_unlock(&mutexLock[sublist_head]);
}

//Based from resources : https://attractivechaos.wordpress.com/2011/10/06/multi-threaded-programming-efficiency-of-locking/
void list_delete_spinLock(int j, SortedListElement_t* listElem, SortedListElement_t* targetElement, int sublist_head)
{
    //wrap the spinlock with clock_gettime
    struct timespec start, end;
    check = clock_gettime(CLOCK_MONOTONIC, &start);
    spinLock_delete_gettime_start_error(check);
    
    while(__sync_lock_test_and_set(&spin_lock_var[sublist_head], 1));
    
    check = clock_gettime(CLOCK_MONOTONIC, &end);
    spinLock_delete_gettime_end_error(check);
    
    //calculate the lock time
    calculate_lock_time(start, end);
    
    targetElement = SortedList_lookup(&list[sublist_head], listElem[j].key);
    targetElementError(targetElement);
    
    check = SortedList_delete(targetElement);
    deleteElementError(check);
    
    __sync_lock_release(&spin_lock_var[sublist_head]);

}

void list_delete_none(int j, SortedListElement_t* listElem, SortedListElement_t* targetElement, int sublist_head)
{
    targetElement = SortedList_lookup(&list[sublist_head], listElem[j].key);
    targetElementError(targetElement);
    
    check = SortedList_delete(targetElement);
    deleteElementError(check);
}

void deleteElement(void* threadsID)
{
    int j = 0;
    
    int sublist_head = 0;
    
    int number  = *(int*)threadsID;
    
    for (j = number; j < numElements; j+= numThread)
    {
        sublist_head = sublist_num[j];
        
        if (mutexFlag == 1)
        {
            list_delete_mutex(j, listElem, targetElement, sublist_head);
            
        }
        else if (spinlockFlag == 1)
        {
            list_delete_spinLock(j, listElem, targetElement, sublist_head);
        }
        else
        {
            list_delete_none(j, listElem, targetElement, sublist_head);
        }
    }
}


void* list_func(void* threadsID)
{
    //insert element into list
    insertElement(threadsID);

    //get the list length
    getListLength();

    //looks up and deletes each of the keys it had previously inserted
    deleteElement(threadsID);
    
    return NULL;
}

void insertString()
{
    strcat(yieldOption, "i");
}

void insertOption()
{
    opt_yield |= INSERT_YIELD;
    insertString();
}

void deleteString()
{
    strcat(yieldOption, "d");
}

void deleteOption()
{
    opt_yield |= DELETE_YIELD;
    deleteString();
}

void lookUpString()
{
    strcat(yieldOption, "l");
}

void lookUpOption()
{
    opt_yield |= LOOKUP_YIELD;
    lookUpString();
}

void mutexLock_malloc_error(pthread_mutex_t* mutexLock)
{
    if (mutexLock == NULL)
    {
        fprintf(stderr, "malloc() for mutex lock has failed");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

//Based from resources : https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/
void initializeMutex()
{
    if (mutexFlag == 1)
    {
        mutexLock = malloc(sizeof(pthread_mutex_t) * number_of_lists);
        mutexLock_malloc_error(mutexLock);
        
        //create the specify number of sublists
        for(int i = 0; i < number_of_lists; i++)
        {
            check = pthread_mutex_init(&mutexLock[i], NULL);
            pthread_mutex_init_check(check);
        }
    }
}

void spinLock_malloc_error(int* spin_lock_var)
{
    if (spin_lock_var == NULL)
    {
        fprintf(stderr, "malloc() for spinlock has failed");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void assign_zeros(int i)
{
    spin_lock_var[i] = 0;
}

void spinLock_fill()
{
    //fill the malloc spin lock with zeros
    for (int i = 0; i < number_of_lists; i++)
    {
        assign_zeros(i);
    }
}

void initializeSpinLock()
{
    if (spinlockFlag == 1)
    {
        spin_lock_var = malloc(sizeof(int) * number_of_lists);
        spinLock_malloc_error(spin_lock_var);
        
        //fill the spin lock with default values zeroes
        spinLock_fill();
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

//Based from resources : http://www.cse.yorku.ca/~oz/hash.html
unsigned long hash(const char *str)
{
    unsigned long hash = 5381;
    int c;
    
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    
    return hash;
}

void sublist_malloc_error(int *sublist_num)
{
    if (sublist_num == NULL)
    {
        fprintf(stderr, "malloc() for sublists have failed");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void hash_assignment(int i, unsigned long hash_value)
{
    hash_value = hash(listElem[i].key) % number_of_lists;
    sublist_num[i] = hash_value;
}

//Based from resources : http://www.cse.yorku.ca/~oz/hash.html
void hash_distribute()
{
    //initialize space for the numbers of sublists
    sublist_num = malloc(sizeof(int) * numElements);
    sublist_malloc_error(sublist_num);
    
    unsigned long hash_value = 0;
    
    //fill the sublists with the numbers from the hash functions
    for (int i = 0; i < numElements; i++)
    {
        //feeding the list element's key to the hash function
        //assigning the output to the slots of sublists
        hash_assignment(i, hash_value);
    }
}

//Based from resources : https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/
void createThreads(pthread_t *threads, int *threadsID)
{
    int rc;
    for (int i = 0; i < numThread; i++)
    {
        threadsID[i] = i;
        rc = pthread_create(&threads[i], NULL, list_func, &threadsID[i]);
        pthread_create_error(rc);
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

//Based from resources : https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/
void joinThreads(pthread_t *threads)
{
    int rc;
    for (int i = 0; i < numThread; i++)
    {
        rc = pthread_join(threads[i], NULL);
        pthread_join_error(rc);
    }
}

void checkSortedList_Length()
{
    check = SortedList_length(list);
    if (check != 0)
    {
        fprintf(stderr, "Length of list is not zero\n");
        exit(2);
    }
}

void free_list_key()
{
    for(int i = 0; i < numElements; i++)
    {
        free((char*)listElem[i].key);
    }
}

void freeMem(int *threadsID, pthread_t *threads)
{
    free_list_key();
    free(listElem);
    
    free(threadsID);
    free(threads);
    
    free(sublist_num);
    free(list);
}

//Based from resources : https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
int main(int argc, char **argv)
{
    //set up four options
    static struct option long_options[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", required_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {"lists", required_argument, NULL, 'l'},
        {0, 0, 0, 0}
    };
    
    int optionIndex = 0;
    
   while ((optionIndex = getopt_long(argc, argv, "t:i:y:s", long_options, NULL)) != -1)
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
               //loop through all the characters to find out the chosen options
               length = strlen(optarg);
               
               for (size_t i = 0; i < length; i++)
               {
                   //three yield options
                   if (optarg[i] == 'i')
                   {
                       insertOption();
                   }
                   else if (optarg[i] == 'd')
                   {
                       deleteOption();
                   }
                   else if (optarg[i] == 'l')
                   {
                       lookUpOption();
                   }
                   else
                   {
                       //invalid argument for yielding
                       fprintf(stderr, "Invalid yielding options. The available options are 'i', 'd', 'l'.\n");
                       fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
                       exit(1);
                   }
               }
               
               break;
           case 's':
               syncFlag = 1;
               
               //there are two cases
               sync_case = optarg[0];
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
               else
               {
                   fprintf(stderr, "Invalid options for sync options.");
                   fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
                   exit(1);
               }
               
               break;
           case 'l':
               listFlag = 1;
               number_of_lists = atoi(optarg);
               break;
           default:
               //print the unrecognized error message and exit with code 1
               fprintf(stderr, "Unrecognized argument: %s\n. Proper usage : lab2_list-threads=# --iterations=# --yield=[idl] --sync=m\n", strerror(optionIndex));
               exit(1);
       }
   }
    
    //Based from resources : https://www.geeksforgeeks.org/signals-c-language/
    handleSignal();
    
    //initialize the variables for mutex
    initializeMutex();

    //initalize the variables for spinLock
    initializeSpinLock();
    
    //create an empty list
    createEmptyList();

    //first create list elements before initalize with content
    createListElements();

    //initializes the required number of list elements
    initializeElement();
    
    //Hash function later to distribute the elements
    hash_distribute();

    //Based from resources : https://www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/gettime.html
    //notes the starting time for the run
    struct timespec start;
    //measure the monotonic time, mark start time
    check = clock_gettime(CLOCK_MONOTONIC, &start);
    clock_gettime_error(check);

    //Based from resources : https://stackoverflow.com/questions/26753957/how-to-dynamically-allocateinitialize-a-pthread-array
    //malloc for the threads
    pthread_t  *threads = malloc(numThread * sizeof(pthread_t));
    threadMallocError(threads);
    //malloc for thread id
    int *threadsID = malloc(numThread * sizeof(int));
    threadIDMallocError(threadsID);
    
    //create the threads
    createThreads(threads, threadsID);

    //wait for all threads to complete
    joinThreads(threads);

    //Based from resources : https://www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/gettime.html
    //notes the ending ending time for the run
    struct timespec end;
    check = clock_gettime(CLOCK_MONOTONIC, &end);
    clock_gettime_error(check);

    //check the length of the list whether it is zero or not
    checkSortedList_Length();
    
    //Based from resources : https://stackoverflow.com/questions/5248915/execution-time-of-c-program
    //calculate the needed information for printing
    int numOperations = numThread * numIterations * 3;
    long long time_passed = (end.tv_sec - start.tv_sec) * one_billion;
    time_passed = time_passed + end.tv_nsec;
    time_passed = time_passed - start.tv_nsec;
    //additional lock time
    int average_lock_time = lock_time_nsecs / numOperations;
    int average_operation_time = time_passed / numOperations;

    //print to stdout with CVS record
    printData(numOperations, time_passed, average_operation_time, average_lock_time);

    //free the allocated memory
    freeMem(threadsID, threads);
    
    exit(0);
}
