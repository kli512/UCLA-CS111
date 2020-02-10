/*
NAME: Kevin Li
EMAIL: li.kevin512@gmail.com
ID: 123456789
*/

#define _GNU_SOURCE

#include "SortedList.h"
#include <sched.h>
#include <string.h>

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
void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
  // As soon as we try to look at what list is, it's critical
  // Other threads cannot be allowed to modify list while we are
  //  either iterating through it or modifying it
  if (opt_yield & INSERT_YIELD) sched_yield();

  if (list == NULL) {
    list = element;
    list->next = element;
    list->prev = element;
    return;
  }

  SortedList_t *cur = list;
  while (cur->next != list && strcmp(cur->next->key, element->key) < 0)
    cur = cur->next;

  element->prev = cur;
  element->next = cur->next;

  cur->next->prev = element;
  cur->next = element;
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
int SortedList_delete(SortedListElement_t *element) {
  if (element == NULL || element->next->prev != element ||
      element->prev->next != element)
    return 1;

  // As soon as we do element->prev->next = element->next
  //  we are critical; this line will take multiple ops
  // If element->next changes while we find
  //  element->prev->next, we'll hook to the wrong part
  // The same goes for the next line
  if (opt_yield & DELETE_YIELD) sched_yield();
  element->prev->next = element->next;
  element->next->prev = element->prev;

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
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
  if (list == NULL) {
    return NULL;
  }

  SortedList_t *cur = list->next;

  // The code becomes critical in this function. Another thread could
  //  insert the value we are looking for right when we pass it. It's
  //  notable, however, that this implementation would work even
  //  asynchronously most of the time
  // This function will only be critical right when the next line
  //  makes updates cur and makes cur->key > key
  if (opt_yield & LOOKUP_YIELD) sched_yield();
  while (cur != list) {

    int cmp = strcmp(cur->key, key);
    if (cmp == 0)
      return cur;
    else if (cmp > 0)
      break;

    cur = cur->next;
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
int SortedList_length(SortedList_t *list) {
  int length = 0;
  if (list == NULL) return -1;
  SortedListElement_t *cur = list;

  // This function enters a critical section almost immediately;
  // as soon as we try counting the number of nodes and move past one,
  // we are at risk of another thread inserting a node right after
  // the head node
  if (opt_yield & LOOKUP_YIELD) sched_yield();

  while (cur->next != list) {
    if (cur->next == NULL || cur->next->prev != cur || cur->prev->next != cur)
      return -1;

    length += 1;
    cur = cur->next;
  }
  return length;
}