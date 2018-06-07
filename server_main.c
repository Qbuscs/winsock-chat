/*
Live Server on port 8888
*/
#include<io.h>
#include<stdio.h>
#include<winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <time.h>

#define TO_GENERATE 1000000
#define THREAD_COUNT 3

#pragma comment(lib,"ws2_32.lib") //Winsock Library

typedef struct {
	int id;
	SOCKET socket;
}client_type;


const char OPTION_VALUE = 1;

client_type clients[4];

DWORD WINAPI serveClient(void* thread_client) {
	client_type* client = (client_type*)thread_client;
	char tempmsg[2000] = "";
	char* client_id;
	char* message = "";
	char loginBuff[100];
	char* login = "";
	char client_id_buff[12] = {'C', 'l', 'i', 'e', 'n', 't', '#', 'N', ':', ' ', '\0'};
	char err_message[24] = { 'C', 'l', 'i', 'e', 'n', 't', '#', 'N', ' ',
							'D', 'i', 's', 'c', 'o', 'n', 'n', 'e', 'c', 't', 'e', 'd', '\0' };


	while (1) {
		if (client->socket != INVALID_SOCKET) {

			for (int i = 0; i < 2000; i++)
				tempmsg[i] = '\0';
			for (int i = 0; i < 100; i++) {
				loginBuff[i] = '\0';
			}


			int iResult = recv(client->socket, loginBuff, 100, 0);
			iResult = recv(client->socket, tempmsg, 2000, 0);

			if (iResult != SOCKET_ERROR)
			{
				if (strcmp("", tempmsg)) {
					client_id_buff[7] = client->id + '0';
					client_id = client_id_buff;
					message = tempmsg;
					login = loginBuff;
				}

				puts(client_id);
				puts(login);
				puts(message);

				if (strcmp("s", message) == 0) {
					show_whole(&log);
					continue;
				}


				insert(&log, message, login);

				//Broadcast that message to the other clients
				for (int i = 0; i < 4; i++)
				{
					if (clients[i].socket != INVALID_SOCKET)
						if (client->id != i) {
							if(strcmp("~", login) == 0) //login jest domyslny
								iResult = send(clients[i].socket, client_id, strlen(client_id), 0);
							else
								iResult = send(clients[i].socket, login, strlen(login), 0);
							iResult = send(clients[i].socket, message, strlen(message), 0);
						}
				}
			}
			else
			{
				err_message[7] = client->id + '0';
				message = err_message;

				puts(message);

				closesocket(client->socket);
				closesocket(clients[client->id].socket);
				clients[client->id].socket = INVALID_SOCKET;

				//Broadcast the disconnection message to the other clients
				for (int i = 0; i < 4; i++)
				{
					if (clients[i].socket != INVALID_SOCKET)
						iResult = send(clients[i].socket, message, strlen(message), 0);
				}

				break;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	log.previous = NULL;
	log.timer = 0;

	WSADATA wsa;
	SOCKET s, new_socket;
	struct sockaddr_in server, client;
	int c;
	char *message;
	char client_message[2000];
	int recv_size;

	int clients_count = 0;

	DWORD thread_id;
	HANDLE threads[4];
	DWORD exitCode[4];

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
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &OPTION_VALUE, sizeof(int)); //Make it possible to re-bind to a port that was used within the last 2 minutes
	setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &OPTION_VALUE, sizeof(int)); //Used for interactive programs

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	puts("Bind done");

	//Listen to incoming connections
	listen(s, 3);

	//Setting up clients
	for (int i = 0; i < 4; i++)
	{
		clients[i].id = -1;
		clients[i].socket = INVALID_SOCKET;
	}

	//Accept and incoming connection
	puts("Waiting for incoming connections...");

	c = sizeof(struct sockaddr_in);

	while (1)
	{

		SOCKET incoming = INVALID_SOCKET;
		incoming = accept(s, NULL, NULL);

		if (incoming == INVALID_SOCKET) continue;

		//Reset the number of clients
		clients_count = -1;

		//Create a temporary id for the next client
		int temp_id = -1;
		for (int i = 0; i < 4; i++)
		{
			if (clients[i].socket == INVALID_SOCKET && temp_id == -1)
			{
				clients[i].socket = incoming;
				clients[i].id = i;
				temp_id = i;
			}

			if (clients[i].socket != INVALID_SOCKET)
				clients_count++;

			//std::cout << client[i].socket << std::endl;
		}

		if (temp_id != -1)
		{
			//Send the id to that client
			printf("Client #%d Accepted\n", clients[temp_id].id);
			send(clients[temp_id].socket, "Welcome to the chat :-)", 24, 0);

			//Create a thread process for that client
			//my_thread[temp_id] = std::thread(process_client, std::ref(client[temp_id]), std::ref(client), std::ref(my_thread[temp_id]));
			//....
			threads[temp_id] = CreateThread(NULL, 0, serveClient,
				(void*)&clients[temp_id], 0, &thread_id);
			if (threads[temp_id] != INVALID_HANDLE_VALUE)
				SetThreadPriority(threads[temp_id], THREAD_PRIORITY_NORMAL);
		}
		else
		{
			send(incoming, "Server is full", 15, 0);
		}
	} //end while

	puts("Closing");
	closesocket(s);

	//Close client socket
	for (int i = 0; i < 4; i++)
	{
		//my_thread[i].detach();
		closesocket(clients[i].socket);
	}

	WSACleanup();

	return 0;
}