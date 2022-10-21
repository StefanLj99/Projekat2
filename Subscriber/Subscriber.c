#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib,"Ws2_32.lib")

#include "../Common/pubsub.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>
#include <string.h>

int subscribeToTopic(SOCKET pubSub)
{
	char* topicBuffer = (char*)malloc(TOPIC_BUFFER_SIZE);
	memset(topicBuffer, 0, TOPIC_BUFFER_SIZE);
	
	printf("Insert topic. Type 'exit' to exit: ");
	gets(topicBuffer);

	if (!strcmp(topicBuffer, "exit"))
	{
		return 0;
	}

	char* data = (char*)malloc(strlen(topicBuffer) + sizeof(int));
	*((int*)data) = strlen(topicBuffer) + sizeof(int);
	memcpy(data + sizeof(int), topicBuffer, strlen(topicBuffer));

	subscribe(pubSub, data, strlen(topicBuffer) + sizeof(int));

	free(topicBuffer);
	free(data);

	return 1;
}

int main()
{
	// WinSock Lib Initialization
	if (!initializeWindowsSockets())
		return 1;

	// Data buffer
	char dataSize = MESSAGE_BUFFER_SIZE + TOPIC_BUFFER_SIZE + 3 * sizeof(int);
	char* data = (char*)malloc(dataSize);

	// Connection socket
	SOCKET connectSocket = INVALID_SOCKET;

	// Server's address info
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
	serverAddress.sin_port = htons(PUBSUB_SECONDARY_PORT);
	int sockAddrLen = sizeof(struct sockaddr);

	// Connect to the pub-sub engine
	connectSocket = tcpConnect((struct sockaddr*)&serverAddress, sockAddrLen);
	if (connectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server.\n");
		WSACleanup();
		return 1;
	}

	printf("Subscriber connected to pub-sub engine!\n");
	printf("Press any key to subscribe to topic.\n");

	int recieved = -2;
	while (1)
	{
		if (_kbhit()) {
			
			if (subscribeToTopic(connectSocket)) {
				printf("Subscribed to topic.\n");
			}
			else {
				printf("Exit.\n");
				break;
			}
		}
		
		recieved = tcpRecv(connectSocket, data, dataSize, 1);

		if (recieved == -1 || recieved == 0)
		{
			printf("Connection closed!\n");
			closesocket(connectSocket);
			WSACleanup();
			return 1;
		}

		if (recieved > (int)sizeof(int))
		{
			printf("PubSub sent: ");
			printData(data);
		}
	}

	printf("Subscriber closed!");
	shutdown(connectSocket, SD_BOTH);
	closesocket(connectSocket);
	return 0;
}