#include "pubsub.h"
#include <stdlib.h>

void publish(SOCKET s, char* topic, char* message)
{
	// Initialize memory to be sent
	int dataSize = strlen(topic) + strlen(message) + 3 * sizeof(int);
	char* data = (char*)malloc(dataSize);
	memset(data, 0, dataSize);

	// Put size of data, size of topic and size of message into data
	*((int*)data) = dataSize;
	*(int*)(data + sizeof(int)) = strlen(topic);
	*(int*)(data + 2*sizeof(int) + strlen(topic)) = strlen(message);

	// Put topic and message into data
	memcpy(data + 2*sizeof(int), topic, strlen(topic));
	memcpy(data + 2*sizeof(int) + strlen(topic) + sizeof(int), message, strlen(message));

	tcpSend(s, data, dataSize);

	free(data);
}

void subscribe(SOCKET pubSub,char* topic, int topicSize)
{
	tcpSend(pubSub, topic, topicSize);
}

void printData(char* data)
{
	char messageBuffer[MESSAGE_BUFFER_SIZE];
	char topicBuffer[TOPIC_BUFFER_SIZE];

	// Put size of data, size of topic and size of message into data
	int dataSize = *((int*)data);
	int topicSize = *(int*)(data + sizeof(int));
	int messageSize = *(int*)(data + 2*sizeof(int) + topicSize);

	memcpy(topicBuffer, data + 2 * sizeof(int), topicSize);
	topicBuffer[topicSize] = '\0';

	memcpy(messageBuffer, data + 3 * sizeof(int) + topicSize, messageSize);
	messageBuffer[messageSize] = '\0';

	printf("%s %s\n", topicBuffer, messageBuffer);
}

DWORD __stdcall publisherThread(LPVOID param)
{
	SOCKET publisher = *(SOCKET*)param;
	SOCKET pubSub = *(SOCKET*)((char*)param + sizeof(SOCKET));

	int dataMaxSize = TOPIC_BUFFER_SIZE + MESSAGE_BUFFER_SIZE + DATA_HEADERS_SIZE;
	char* data = (char*)malloc(dataMaxSize);
	memset(data, 0, dataMaxSize);

	while (1)
	{
		int recieved = tcpRecv(publisher, data, dataMaxSize, 0);
		if (recieved == -1 || recieved == 0) break;
	
		printData(data);

		tcpSend(pubSub, data, recieved);
	}
	
	closesocket(publisher);
	free(data);
	free(param);
	return 0;
}

DWORD __stdcall pubSubThread(LPVOID param)
{
	PubSubThreadParameter* parameter = (PubSubThreadParameter*)param;
	SOCKET pubSub = parameter->pubSub;
	HashMapItem* hashMap = parameter->hashArray;

	int dataMaxSize = TOPIC_BUFFER_SIZE + MESSAGE_BUFFER_SIZE + DATA_HEADERS_SIZE;
	char* data = (char*)malloc(dataMaxSize);
	memset(data, 0, dataMaxSize);

	char topicBuffer[TOPIC_BUFFER_SIZE];
	int topicSize = 0;
	int key = -1;


	while (1)
	{
		// Recieve data from the primary pub-sub
		int recieved = tcpRecv(pubSub, data, dataMaxSize, 0);
		if (recieved == -1 || recieved == 0) break;

		// Get the topic
		topicSize = *(int*)(data + sizeof(int));
		memcpy(topicBuffer, data + 2 * sizeof(int), topicSize);
		topicBuffer[topicSize] = '\0';

		// Get key from the topic
		key = hash(topicBuffer, topicSize);

		printData(data); // To test

		Node* head = hashMap[key].socketList;
		Node* temp = NULL;
		for (temp = head; temp != NULL; temp = temp->next)
		{
			if (temp == NULL) break;
			tcpSend(temp->s, data, recieved);
		}
	}

	closesocket(pubSub);
	free(data);
	free(param);
	return 0;
}

DWORD __stdcall subscriberThread(LPVOID param)
{
	SubscriberThreadParameter* parameter = (SubscriberThreadParameter*)param;
	SOCKET subscriber = parameter->subscriberSocket;
	HashMapItem* hashArray = parameter->hashArray;

	char* topicBuffer = (char*)malloc(TOPIC_BUFFER_SIZE + sizeof(int));
	memset(topicBuffer, 0, TOPIC_BUFFER_SIZE + sizeof(int));

	int recieved = 0;
	while (1)
	{
		recieved = tcpRecv(subscriber, topicBuffer, TOPIC_BUFFER_SIZE, 0);
		if (recieved == -1 || recieved == 0)
			break;

		topicBuffer[recieved] = 0;

		insertHashMapItem(hashArray, topicBuffer + sizeof(int), subscriber);
	}

	// Subscriber sent shutdown signal, or there is an error, remove it from map
	if (recieved == 0 || recieved == -1) {
		removeHashMapItem(hashArray, subscriber);
	}

	closesocket(subscriber);
	free(topicBuffer);
	free(param);
	return 0;
}


