#include "communication.h"
#include <stdlib.h>

#define HASH_ARRAY_SIZE 10
#define INVALID_HASH_KEY -1

// Critical section for hashMap
CRITICAL_SECTION cs;

// List
typedef struct listNode_st
{
    SOCKET s;
    struct listNode_st* next;
}Node;

// HashMap
typedef struct hashMapItem_st
{
    int key;
    Node* socketList; //element liste

}HashMapItem;

/**
* Pushes socket to the list. This function is not thread-safe and should be called only by insertHashMapItem()
* 
* \param head Head of the list
* \param s Socket to be inserted
*/
void push(Node** head, SOCKET s);

/**
* Removes socket from the list. This function is not thread-safe and should be called only by removeHashMapItem() 
* 
* \param head Head of the list
* \param s Socket to be inserted
*/
void removeSocket(Node** head, SOCKET s);


/**
* Calculates hash value of string key.
* 
* \param key String to be hashed
* \param Length of the key
* 
* \returns Hash value.
*/
int hash(char* key, int keySize);

/**
* Initializes hash map to initial values
* 
* \param hashArray HashMap
*/
void initializeHashMap(HashMapItem* hashArray);

/**
* Inserts socket to the hashMap for specific topic.
* 
* \param hashArray HashMap
* \param topic Topic as key of map to be inserted
* \param s Socket as value of map list to be inserted
*/
void insertHashMapItem(HashMapItem* hashArray, char* topic, SOCKET s);

/**
* Removes subscriber from all topics.
* 
* \param hashArray HashMap
* \param s Subscriber to be removed
*/
void removeHashMapItem(HashMapItem* hashArray, SOCKET s);

/**
* Removes all elements from hashMap and initializes it.
*/
void destroyHashMap(HashMapItem* hashArray);

void printHashMapData(HashMapItem* hashArray);

