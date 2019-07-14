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
pthread_mutex_t mutexLock;
int spin_lock_var = 0;
int opt_yield = 0;

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

void printData(int numOperations, long long time_passed, int average_time)
{
    getName();
    
    int numOfList = 1;
    
    printf("%s,%d,%d,%d,%d,%lld,%d\n", CVS, numThread, numIterations, numOfList, numOperations, time_passed, average_time);
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

//Based from resources : https://www.geeksforgeeks.org/generic-linked-list-in-c-2/
void createEmptyList()
{
    list = malloc(sizeof(SortedList_t));
    malloc_error_list(list);

    list->key = NULL;
    list->prev = list;
    list->next = list;
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
        fprintf(stderr, "malloc() failed");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void threadIDMallocError(int *threadsID)
{
    if (threadsID == NULL)
    {
        fprintf(stderr, "malloc() failed");
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

//Based from reosurces : https://docs.oracle.com/cd/E19455-01/806-5257/sync-12/index.html
void list_insert_mutex(int i, SortedListElement_t* listElem)
{
    pthread_mutex_lock(&mutexLock);
    SortedList_insert(list, &listElem[i]);
    pthread_mutex_unlock(&mutexLock);

}

//Based from resources : https://attractivechaos.wordpress.com/2011/10/06/multi-threaded-programming-efficiency-of-locking/
void list_insert_spinLock(int i, SortedListElement_t* listElem)
{
    while(__sync_lock_test_and_set(&spin_lock_var, 1));
    SortedList_insert(list, &listElem[i]);
    __sync_lock_release(&spin_lock_var);
}

void insertElement(void* threadsID)
{
    int i = 0;
    
    int number = *(int*)threadsID;
    
    for (i = number; i < numElements; i += numThread)
    {
        if (mutexFlag == 1)
        {
            list_insert_mutex(i, listElem);
        }
        else if (spinlockFlag == 1)
        {
            list_insert_spinLock(i, listElem);
        }
        else
        {
            SortedList_insert(list, &listElem[i]);
        }
    }
}

//Based from reosurces : https://docs.oracle.com/cd/E19455-01/806-5257/sync-12/index.html
void list_length_mutex(SortedList_t *list, int listLength)
{
    pthread_mutex_lock(&mutexLock);
    listLength = SortedList_length(list);
    listLengthError(listLength);
    pthread_mutex_unlock(&mutexLock);
}

//Based from resources : https://attractivechaos.wordpress.com/2011/10/06/multi-threaded-programming-efficiency-of-locking/
void list_length_spinLock(SortedList_t *list, int listLength)
{
    while(__sync_lock_test_and_set(&spin_lock_var, 1));
    listLength = SortedList_length(list);
    listLengthError(listLength);
    __sync_lock_release(&spin_lock_var);
}

void getListLength()
{
    int listLength = 0;
    
    if (mutexFlag == 1)
    {
        list_length_mutex(list, listLength);
    }
    else if (spinlockFlag == 1)
    {
        list_length_spinLock(list, listLength);
    }
    else
    {
        listLength = SortedList_length(list);
        listLengthError(listLength);
    }
}

//Based from reosurces : https://docs.oracle.com/cd/E19455-01/806-5257/sync-12/index.html
void list_delete_mutex(int j, SortedListElement_t* listElem, SortedListElement_t* targetElement)
{
    pthread_mutex_lock(&mutexLock);
    
    targetElement = SortedList_lookup(list, listElem[j].key);
    targetElementError(targetElement);
    
    check = SortedList_delete(targetElement);
    deleteElementError(check);
    
    pthread_mutex_unlock(&mutexLock);
}

//Based from resources : https://attractivechaos.wordpress.com/2011/10/06/multi-threaded-programming-efficiency-of-locking/
void list_delete_spinLock(int j, SortedListElement_t* listElem, SortedListElement_t* targetElement)
{
    while(__sync_lock_test_and_set(&spin_lock_var, 1));
    
    targetElement = SortedList_lookup(list, listElem[j].key);
    targetElementError(targetElement);
    
    check = SortedList_delete(targetElement);
    deleteElementError(check);
    
    __sync_lock_release(&spin_lock_var);

}

void list_delete_none(int j, SortedListElement_t* listElem, SortedListElement_t* targetElement)
{
    targetElement = SortedList_lookup(list, listElem[j].key);
    targetElementError(targetElement);
    
    check = SortedList_delete(targetElement);
    deleteElementError(check);
}

void deleteElement(void* threadsID)
{
    int j = 0;
    
    int number  = *(int*)threadsID;
    
    for (j = number; j < numElements; j+= numThread)
    {
        if (mutexFlag == 1)
        {
            list_delete_mutex(j, listElem, targetElement);
            
        }
        else if (spinlockFlag == 1)
        {
            list_delete_spinLock(j, listElem, targetElement);
        }
        else
        {
            list_delete_none(j, listElem, targetElement);
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

//Based from resources : https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/
void initializeMutex()
{
    if (mutexFlag == 1)
    {
        check = pthread_mutex_init(&mutexLock, NULL);
        pthread_mutex_init_check(check);
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
    free(list);
    free(threadsID);
    free(threads);
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

    //create an empty list
    createEmptyList();

    //first create list elements before initalize with content
    createListElements();

    //initializes the required number of list elements
    initializeElement();

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
    int average_time = time_passed / numOperations;

    //print to stdout with CVS record
    printData(numOperations, time_passed, average_time);

    //free the allocated memory
    freeMem(threadsID, threads);
    
    exit(0);
}
