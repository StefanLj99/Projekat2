#include "hashmap.h"

// Add node at the end of the list
void push(Node** head, SOCKET s)
{
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->s = s;
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;
        return;
    }

    Node* temp = *head;
    while (temp->next != NULL) temp = temp->next;

    temp->next = newNode;

    return;
}

void removeSocket(Node** head, SOCKET s)
{
    Node* temp = *head;
    Node* prev = NULL;

    // If head node itself holds the key to be deleted
    if (temp != NULL && temp->s == s) {
        *head = temp->next;
        free(temp);
        return;
    }

    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && temp->s != s) {
        prev = temp;
        temp = temp->next;
    }

    // If key was not present in linked list
    if (temp == NULL)
        return;

    // Unlink the node from linked list
    prev->next = temp->next;

    free(temp); // Free memory
}

int hash(char* key, int keySize)
{
    unsigned long hash = 5381;
    int c;

    for (int i = 0; i < keySize; ++i)
    {
        c = key[i];
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash % HASH_ARRAY_SIZE;
}

void initializeHashMap(HashMapItem* hashArray)
{
    for (int i = 0; i < HASH_ARRAY_SIZE; ++i)
    {
        hashArray[i].key = INVALID_HASH_KEY;
        hashArray[i].socketList = NULL;
    }

    InitializeCriticalSection(&cs);
}

void insertHashMapItem(HashMapItem* hashArray, char* topic, SOCKET s)
{
    unsigned long hashValue = hash(topic, strlen(topic));

    EnterCriticalSection(&cs);

    // If the item already exists
    if (hashArray[hashValue].key != INVALID_HASH_KEY)
    {
        push(&(hashArray[hashValue].socketList), s);
    }
    else
    {
        // Assign key and add socket as first node
        hashArray[hashValue].key = hashValue;
        Node* node = (Node*)malloc(sizeof(Node));
        node->s = s;
        node->next = NULL;
        hashArray[hashValue].socketList = node;
    }

    LeaveCriticalSection(&cs);
}

void removeHashMapItem(HashMapItem* hashArray, SOCKET s)
{
    EnterCriticalSection(&cs);

    for (int i = 0; i < HASH_ARRAY_SIZE; ++i)
    {
        if (hashArray[i].key != INVALID_HASH_KEY) {
            removeSocket(&(hashArray[hashArray[i].key].socketList), s);
        }
    }

    LeaveCriticalSection(&cs);
}

void destroyHashMap(HashMapItem* hashArray)
{
    EnterCriticalSection(&cs);

    for (int i = 0; i < HASH_ARRAY_SIZE; ++i)
    {
        if (hashArray[i].key != INVALID_HASH_KEY) {

            Node* node = hashArray[i].socketList;
            while (node != NULL) {
                removeSocket(&(hashArray[hashArray[i].key].socketList), node->s);
                node = node->next;
            }
        }
    }

    LeaveCriticalSection(&cs);
}

void printHashMapData(HashMapItem* hashArray)
{
    EnterCriticalSection(&cs);

    for (int i = 0; i < HASH_ARRAY_SIZE; ++i)
    {
        if (hashArray[i].key != INVALID_HASH_KEY)
        {
            printf("Key: %d\t", hashArray[i].key);
            Node* node = hashArray[i].socketList;
            printf("Values: ");
            while (node != NULL)
            {
                printf("%d ", node->s);
                node = node->next;
                if (node != NULL) printf("-> ");
            }
        }
    }

    LeaveCriticalSection(&cs);
}