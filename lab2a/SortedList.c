/*
 Name: Sum Yi Li
 Email: sammyli0106@gmail.com
 ID: 505146702
 */

#include "SortedList.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>

/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *	The specified element will be inserted in to
 *	the specified list, which will be kept sorted
 *	in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 */

//Based from resources: https://www.geeksforgeeks.org/given-a-linked-list-which-is-sorted-how-will-you-insert-in-sorted-way/
void insertFunc(SortedListElement_t *element, SortedListElement_t *current)
{
    element->next = current;
    element->prev = current->prev;
    current->prev->next = element;
    current->prev = element;
}

void insert_critical_section()
{
    if (opt_yield & INSERT_YIELD)
    {
        sched_yield();
    }
}

void insert_check_empty(SortedList_t *list, SortedListElement_t *element)
{
    if (list == NULL || element == NULL)
    {
        return;
    }
}

//Based from resources: https://www.geeksforgeeks.org/given-a-linked-list-which-is-sorted-how-will-you-insert-in-sorted-way/
void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
    SortedListElement_t *current;
    
    //check if the list is empty at first
    insert_check_empty(list, element);
    
    current = list->next;
    
    while (current != list)
    {
        //check to stop at the insert position
        int check = strcmp(element->key, current->key);
        if (check <= 0)
        {
            break;
        }
        
        //move on to the next one
        current = current->next;
    }
    
    //critical section
    insert_critical_section();
    
    insertFunc(element, current);
}

/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *	The specified element will be removed from whatever
 *	list it is currently in.
 *
 *	Before doing the deletion, we check to make sure that
 *	next->prev and prev->next both point to this node
 *
 * @param SortedListElement_t *element ... element to be removed
 *
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 */

//Based from resources : https://www.geeksforgeeks.org/linked-list-set-3-deleting-node/
void deleteFunc(SortedListElement_t *element)
{
    element->next->prev = element->prev;
    element->prev->next = element->next;
}

void delete_critical_section()
{
    if (opt_yield & DELETE_YIELD)
    {
        sched_yield();
    }
}

//Based from resources : https://www.geeksforgeeks.org/linked-list-set-3-deleting-node/
int SortedList_delete(SortedListElement_t *element)
{
    //if element to delete is empty, then not successful
    //check the deleted node is being connected in between
    if (element == NULL)
    {
        return 1;
    }
    
    //critical section
    delete_critical_section();
    
    if (element->next->prev != element || element->prev->next != element)
    {
        return 1;
    }
    
    //changing the pointer part
    deleteFunc(element);
    
    return 0;
}

/**
 * SortedList_lookup ... search sorted list for a key
 *
 *	The specified list will be searched for an
 *	element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 */

void lookUp_critical_section()
{
    if (opt_yield & LOOKUP_YIELD)
    {
        sched_yield();
    }
}



//Based from resources : https://www.geeksforgeeks.org/search-an-element-in-a-linked-list-iterative-and-recursive/
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
    SortedListElement_t *current;
    
    if (list == NULL || key == NULL)
    {
        return NULL;
    }
    
    current = list->next;
    
    
    while (current != list)
    {
        if (strcmp(current->key, key) == 0)
        {
            //found the matching element
            return current;
        }
        
        //critical section
        lookUp_critical_section();
        
        current = current->next;
    }
    
    return NULL;
}

/**
 * SortedList_length ... count elements in a sorted list
 *	While enumeratign list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *	   -1 if the list is corrupted
 */

//Based from resources : https://www.geeksforgeeks.org/find-length-of-a-linked-list-iterative-and-recursive/
int SortedList_length(SortedList_t *list)
{
    SortedListElement_t *current;
    
    if (list == NULL)
    {
        return -1;
    }
    
    int count = 0;
    current = list->next;
    
    while(current != list)
    {
        //critical section
        lookUp_critical_section();
        count++;
        current = current->next;
    }
    return count;
    
}





