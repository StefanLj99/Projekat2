#include "communication.h"

int initializeWindowsSockets()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 0;
	}
	return 1;
}

int tcpSend(SOCKET s, char* data, int size)
{
	int sent_size = 0;
	FD_SET write;
	struct timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	// Logic: Send until sends "size" bytes
	while (sent_size < size)
	{
		FD_ZERO(&write);
		FD_SET(s, &write);
		int result = select(0, NULL, &write, NULL, &timeVal);
		if (result == SOCKET_ERROR)
		{
			printf("\nselect failed with error: %d\n", WSAGetLastError());
			return -1;
		}
		else if (result == 0)
		{
			Sleep(SLEEP_TIME);
			continue;
		}
		else if (FD_ISSET(s, &write))
		{
			result = send(s, data + sent_size, size - sent_size, 0);
			if (result == SOCKET_ERROR)
			{
				printf("\nsend failed with error: %d\n", WSAGetLastError());
				return -1;
			}
			else
			{
				sent_size += result; //brojac za velicinu podataka koji su poslati
			}
		}
	}

	return sent_size;
}

int tcpRecv(SOCKET s, char* data, int size, int tryOnce)
{
	int recvd_size = 0;
	int to_recieve = -1;
	FD_SET read;
	struct timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	int messageReceived = 0;

	// Logic: Recieve until recieves "to_recieve" bytes when it recieves message first time
	do
	{
		FD_ZERO(&read);
		FD_SET(s, &read);
		int result = select(0, &read, NULL, NULL, &timeVal);
		if (result == SOCKET_ERROR)
		{
			printf("\nselect failed with error: %d\n", WSAGetLastError());
			return -1;
		}
		else if (result == 0)
		{
			Sleep(SLEEP_TIME);
			continue;
		}
		else if (FD_ISSET(s, &read))
		{
			result = recv(s, data + recvd_size, size - recvd_size, 0);
			if (result == SOCKET_ERROR)
			{
				printf("\nrecv failed with error: %d\n", WSAGetLastError());
				return -1;
			}
			else
			{
				messageReceived = 1; // Set flag to true
				recvd_size += result;

				if (recvd_size >= (int)sizeof(int)) {
					to_recieve = *(int*)data;
				}
			}
		}

	} while (recvd_size != to_recieve && !tryOnce);
	
	if (!messageReceived)
		return -2;
	
	return recvd_size;
}

SOCKET tcpConnect(struct sockaddr* address, int sockAddrLen)
{
	SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (connect(connectSocket, (struct sockaddr*)address, sockAddrLen) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}
   //prebacivanje u neblokirajuci
	unsigned long mode = 1;
	if (ioctlsocket(connectSocket, FIONBIO, &mode) == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}

	return connectSocket;
}