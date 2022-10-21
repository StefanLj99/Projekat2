#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib,"Ws2_32.lib")

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>

#include "../Common/pubsub.h"

#define NUMBER_OF_HANDLES 50

void startPrimaryPubSub();

void startSecondaryPubSub();

int main(int argc, char** argv)
{
	// WinSock Lib Initialization
	if (!initializeWindowsSockets())
		return 1;

	if (argc != 2) {
		printf("Argument '1' or '2' required!\n.");
		return 1;
	}

	if (!strcmp(argv[1], "1")) {
		startPrimaryPubSub();
	}
	else if (!strcmp(argv[1], "2")) {
		startSecondaryPubSub();
	}

	return 0;
}

void startPrimaryPubSub()
{
	// Socket to connect to secondary pub-sub
	SOCKET connectSocket = INVALID_SOCKET;
	// Address info of secondary pub-sub
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
	serverAddress.sin_port = htons(PUBSUB_SECONDARY_PORT);
	int sockAddrLen = sizeof(struct sockaddr);

	// Connect to the secondary pub-sub engine
	connectSocket = tcpConnect((struct sockaddr*)&serverAddress, sockAddrLen);
	if (connectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to secondary pub-sub engine.\n");
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	printf("PubSub connected to secondary pub-sub engine!\n");
	
	// Open connection towards publishers
	SOCKET pubListenSocket = INVALID_SOCKET;

	struct sockaddr_in pubServerAddress;
	pubServerAddress.sin_family = AF_INET;
	pubServerAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
	pubServerAddress.sin_port = htons(PUBSUB_PRIMARY_PORT);
	sockAddrLen = sizeof(struct sockaddr_in);

	// Initialize PubListen socket
	pubListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (pubListenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Bind
	int iResult = bind(pubListenSocket, (struct sockaddr*)&pubServerAddress, sockAddrLen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(pubListenSocket);
		WSACleanup();	
		return 1;
	}

	// Start listening
	iResult = listen(pubListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(pubListenSocket);
		WSACleanup();
		return 1;
	}

	HANDLE threadHandles[NUMBER_OF_HANDLES];
	int handleIt = 0;

	struct sockaddr_in publisherAddress;
	unsigned long mode = 1;

	int id = 0;
	while (1)
	{
		SOCKET publisher = accept(pubListenSocket, (struct sockaddr*)&publisherAddress, &sockAddrLen);
		if (publisher == INVALID_SOCKET) continue;

		if (ioctlsocket(publisher, FIONBIO, &mode) != 0) {
			printf("ioctl failed with error %d\n", WSAGetLastError());
			continue;
		}

		PublisherThreadParameter* parameter = malloc(sizeof(PublisherThreadParameter));
		parameter->publisherSocket = publisher;
		parameter->pubsubSocket = connectSocket;
		threadHandles[handleIt] = CreateThread(NULL, 0, &publisherThread, parameter, 0, &id);
		handleIt++;
	}

	closesocket(pubListenSocket);
	closesocket(connectSocket);
	WSACleanup();

	for (int i = 0; i < handleIt; ++i)
	{
		CloseHandle(threadHandles[i]);
	}

	return;
}

void startSecondaryPubSub()
{
	SOCKET subListenSocket = INVALID_SOCKET;
	struct sockaddr_in subServerAddress;
	subServerAddress.sin_family = AF_INET;
	subServerAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
	subServerAddress.sin_port = htons(PUBSUB_SECONDARY_PORT);
	int sockAddrLen = sizeof(struct sockaddr_in);

	// Initialize PubListen socket
	subListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (subListenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		closesocket(subListenSocket);
		return 1;
	}

	// Bind
	int iResult = bind(subListenSocket, (struct sockaddr*)&subServerAddress, sockAddrLen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(subListenSocket);
		WSACleanup();
		return 1;
	}

	// Start listening
	iResult = listen(subListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	unsigned long mode = 1;
	struct sockaddr_in pubSubAddress;

	SOCKET pubSub = accept(subListenSocket, (struct sockaddr*)&pubSubAddress, &sockAddrLen);
	if (pubSub == INVALID_SOCKET)
	{
		printf("Error connecting to the secondary pubsub engine!\n");
		closesocket(subListenSocket);
		WSACleanup();
		return 1;
	}

	if (ioctlsocket(pubSub, FIONBIO, &mode) != 0)
	{
		printf("ioctl failed with error %d\n", WSAGetLastError());
		closesocket(pubSub);
		closesocket(subListenSocket);
		WSACleanup();
		return 1;
	}

	printf("PubSub connected to primary pub-sub engine!\n");

	HashMapItem hashArray[HASH_ARRAY_SIZE];
	initializeHashMap(hashArray);

	HANDLE threadHandles[NUMBER_OF_HANDLES];
	int handleIt = 0;

	int id = 0;
	PubSubThreadParameter* parameter = (PubSubThreadParameter*)malloc(sizeof(PubSubThreadParameter));
	parameter->pubSub = pubSub;
	parameter->hashArray = hashArray;
	HANDLE h = CreateThread(NULL, 0, &pubSubThread, parameter, 0, &id);

	struct sockaddr_in subscriberAddress;
	id = 0;
	while (1)
	{
		SOCKET subscriber = accept(subListenSocket, (struct sockaddr*)&subscriberAddress, &sockAddrLen);
		if (subscriber == INVALID_SOCKET) continue;

		if (ioctlsocket(subscriber, FIONBIO, &mode) != 0) {
			printf("ioctl failed with error %d\n", WSAGetLastError());
			continue;
		}

		SubscriberThreadParameter* parameter = malloc(sizeof(PublisherThreadParameter));
		parameter->subscriberSocket = subscriber;
		parameter->hashArray = hashArray;
		threadHandles[handleIt] = CreateThread(NULL, 0, &subscriberThread, parameter, 0, &id);
		handleIt++;
	}

	destroyHashMap(hashArray);
	DeleteCriticalSection(&cs);
	closesocket(subListenSocket);
	closesocket(pubSub);
	WSACleanup();

	for (int i = 0; i < handleIt; ++i)
	{
		CloseHandle(threadHandles[i]);
	}
	return;
}
