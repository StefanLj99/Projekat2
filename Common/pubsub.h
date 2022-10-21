#include "communication.h"
#include "hashmap.h"

#define MESSAGE_BUFFER_SIZE 256
#define TOPIC_BUFFER_SIZE 64
#define DATA_HEADERS_SIZE 3*sizeof(int)

#pragma pack(1)
typedef struct publisherThreadParam_st
{
	SOCKET publisherSocket;
	SOCKET pubsubSocket;
}PublisherThreadParameter;

#pragma pack(1)
typedef struct pubSubThreadParam_st
{
	SOCKET pubSub;
	HashMapItem* hashArray;
}PubSubThreadParameter;

#pragma pack(1)
typedef struct subscriberThreadParam_st
{
	SOCKET subscriberSocket;
	HashMapItem* hashArray;
}SubscriberThreadParameter;

void publish(SOCKET s, char* topic, char* message); 

void subscribe(SOCKET pubSub, char* topic, int topicSize);

/**
* Thread used to recieve data from the publisher and forward information to
* the 2nd pub-sub engine.
*/
DWORD WINAPI publisherThread(LPVOID param);

/**
* Thread used to receive data from the 1st pub-sub engine and forward information
* to subscribers.
*/
DWORD WINAPI pubSubThread(LPVOID param);

/**
* Thread used to subscribe subscribers to the topic.
*/ 
DWORD WINAPI subscriberThread(LPVOID param);
