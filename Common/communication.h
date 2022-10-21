#pragma once
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")

#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <winsock2.h>

#define SERVER_IP "127.0.0.1"
#define PUBSUB_PRIMARY_PORT 27000
#define PUBSUB_SECONDARY_PORT 27001

#define SLEEP_TIME 100

int initializeWindowsSockets();

/**
* Sends data over network using non-blocking tcp socket.
* First 4 bytes of data indicated size to be sent.
* 
* \param s TCP socket
* \param data Data to be sent
* \param size Size of the data
* 
* \returns Number of bytes sent. In case of error, -1 is returned.
* 
*/
int tcpSend(SOCKET s, char* data, int size);

/**
* Recieves data over network using non-blocking tcp socket.
* First 4 bytes in data indicate how much is to be received, and function tries
* to recieve until everything is received. In case of flag tryOnce is 1, it will only
* perform one receive and exit the function.
* 
* \param s TCP socket
* \param data Buffer for data
* \param size Size of the buffer
* \param tryOnce If 1, it will try receive only once. Otherwise, it will perform receive in the loop until all data is received.
* 
* \returns Number of bytes received. In case of error, -1 is returned. In case of shutdown, 0 is returned.
*/
int tcpRecv(SOCKET s, char* data, int size, int tryOnce);

/**
* Connects to the address, creates a socket and puts it in non-blocking mode.
* 
* \param address Address to connect to
* \param sockAddrLen Lenth of address data structure
* 
* \returns socket is successfull, otherwise INVALID_SOCKET is returned.
*/
SOCKET tcpConnect(struct sockaddr* address, int sockAddrLen);

