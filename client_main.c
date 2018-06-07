/*
Create a TCP socket
*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

DWORD WINAPI listenToServer(void* socket) {
	SOCKET* s = (SOCKET*)socket;
	char server_reply[2000];
	int recv_size;
	int err;
	while (1) {
		if ((recv_size = recv(*s, server_reply, 2000, 0)) == SOCKET_ERROR)
		{
			//puts("recv failed");
			err = WSAGetLastError();
			continue;
		}

		//puts("Reply received\n");

		//Add a NULL terminating character to make it a proper string before printing
		server_reply[recv_size] = '\0';
		puts(server_reply);
	}
}

int main(int argc, char *argv[])
{
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	char server_reply[2000];
	char to_send[2000];
	char* message;
	int recv_size;

	char login[100] = {'~'};

	DWORD thread_id;
	HANDLE thread;
	DWORD exitCode;

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 0), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");


	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);

	//Connect to remote server
	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		puts("connect error");
		return 1;
	}

	puts("Connected");

	thread = CreateThread(NULL, 0, listenToServer, (void*)&s, 0, &thread_id);
	if (thread != INVALID_HANDLE_VALUE)
		SetThreadPriority(thread, THREAD_PRIORITY_NORMAL);

	while (1) {
		//Send some data
		//message = "Hello\0";
		
		fgets(to_send, 2000, stdin);
		for (int i = 0; i < 2000; i++) {
			if (to_send[i] == '\n') {
				to_send[i] = '\0';
				break;
			}
		}
		if (to_send[0] == 'q')
			break;
		if (to_send[0] == 'l') {
			fgets(login, 100, stdin);
			for(int i=0; i<100; i++)
				if (login[i] == '\n') {
					login[i] = ':';
					login[i + 1] = '\0';
					break;
				}
			continue;
		}

		if (send(s, login, strlen(login), 0) < 0)
		{
			puts("Send failed");
			return 1;
		}
		
		if (send(s, to_send, strlen(to_send), 0) < 0)
		{
			puts("Send failed");
			return 1;
		}

		//else puts("Data Send\n");

		//clear buffor
		for (int i = 0; i < 2000; i++)
			to_send[i] = 0;
	}
	closesocket(s);
	WSACleanup();
	return 0;
}