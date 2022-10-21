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

void printMenu() 
{
	printf("0. Exit.\n");
	printf("1. Publish a message.\n");
	
}

void publishMenu(char* topic, char* message)
{
	printf("Insert topic: ");
	gets(topic);
	printf("Insert message: ");
	gets(message);
}

int main()
{
	// WinSock Lib Initialization
	if (!initializeWindowsSockets())
		return 1;

	// Data buffers
	char messageBuffer[MESSAGE_BUFFER_SIZE];
	char topicBuffer[TOPIC_BUFFER_SIZE];

	// Connection data
	SOCKET connectSocket = INVALID_SOCKET;

	// Server's address info
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
	serverAddress.sin_port = htons(PUBSUB_PRIMARY_PORT);
	int sockAddrLen = sizeof(struct sockaddr);
	
	// Connect to the pub-sub engine
	connectSocket = tcpConnect((struct sockaddr*)&serverAddress, sockAddrLen);
	if (connectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	printf("Publisher connected to pub-sub engine!\n");

	// Menu
	int menuOption = 0;
	do {
		printMenu();
		scanf("%d", &menuOption);
		getchar(); //kupi enter

		if (menuOption != 0) {
			publishMenu(topicBuffer, messageBuffer);
			publish(connectSocket, topicBuffer, messageBuffer);
		}

	} while (menuOption != 0);

	// Disconnect and exit
	closesocket(connectSocket);
	WSACleanup();

	return 0;
}