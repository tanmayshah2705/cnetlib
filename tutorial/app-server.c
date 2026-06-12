/**
 *@example app-server.c
 *An example that uses Server API to setup a server and accept client connections.
 *@author Tanmay Shah
*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<ssnl.h>

//Following code should be written by network layer library user to create server and accept client connections 
//Expected output: prints server state and connection metadata, then handles one request per client.
//Note: the server stays running until it receives the shutdown request or the process is terminated.
//Note: each TCP API call is followed by tcp_connection_failed() and tcp_connection_error() for  error handling.


// gets executed when the server starts (one time act only)
void tcp_server_started_handler(unsigned short int server_port)
{
    printf("Server is ready to accept requests on port :%u\n",server_port);
}

//gets executed when server stops (one time act only)
void tcp_server_stopped_handler(unsigned short int server_port)
{
	printf("Server listening on port %u has stopped\n",server_port);
}

//gets executed when a client connection is accepted by server (multiple times act)
void tcp_server_client_connected_handler(unsigned short int server_port,tcp_server *server,tcp_client *connected_client)
{
	char data[250000];
	char* request;
	uint32_t request_size;	
	char* error_str;
	char *local_ip, *remote_ip;
	unsigned short int local_port, remote_port;

	printf("Server listening on port %u has accepted a connection request\n",server_port);

	// code to get remote client details starts here
	remote_ip=tcp_client_get_remote_ip(connected_client);  // returns malloc'd string, free after use
	if(tcp_client_failed(connected_client))
	{
		tcp_client_error(connected_client,&error_str); // Note: tcp_connection_error() allocates memory in error_str, must be freed after use
		if(error_str!=NULL)
		{
			printf("Remote IP : %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Remote IP : Unable to get remote client IP\n");
		}
	}
	else
	{
		printf("Remote IP : %s\n",remote_ip);
		free(remote_ip);
	}

	remote_port=tcp_client_get_remote_port(connected_client);
	if(tcp_client_failed(connected_client))
	{
		tcp_client_error(connected_client,&error_str);
		if(error_str!=NULL)
		{
			printf("Remote port :  %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Remote port : Unable to get remote client port\n");
		}
	}
	else
	{
		printf("Remote port : %u\n",remote_port);
	}
	// code to get remote client details ends here

	// code to get local server details starts here
	local_ip=tcp_client_get_local_ip(connected_client);  // returns malloc'd string, free after use
	if(tcp_client_failed(connected_client))
	{
		tcp_client_error(connected_client,&error_str);
		if(error_str!=NULL)
		{
			printf("Local IP : %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Local IP : Unable to get server local IP\n");
		}
	}
	else
	{
		printf("Local IP : %s\n",local_ip);
		free(local_ip);
	}

	local_port=tcp_client_get_local_port(connected_client);
	if(tcp_client_failed(connected_client))
	{
		tcp_client_error(connected_client,&error_str);
		if(error_str!=NULL)
		{
			printf("Local port : %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Local port : Unable to get server local port\n");
		}
	}
	else
	{
		printf("Local port : %u\n",local_port);
	}
	// code to get local server details ends here

	// code to receive request starts here
	request=tcp_client_receive(connected_client,&request_size);
	if(tcp_client_failed(connected_client))
	{
		tcp_client_error(connected_client,&error_str);
		if(error_str!=NULL)
		{
			printf("Receive error: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to receive request from client\n");
		}
		disconnect_tcp_client(connected_client);
		release_tcp_client(connected_client);
		return;
	}
	printf("Number of bytes received in request %u\n",request_size);
	for(int i=0;i<request_size;++i) printf("%c",request[i]);
	printf("\n");

	if(request_size==15 && strncmp(request,"shutdown-server",request_size)==0)
	{
		tcp_stop_server(server);
		disconnect_tcp_client(connected_client);
		release_tcp_client(connected_client);
		free(request);
		return;
	}
	free(request);
	// code to receive request ends here

	// code to send response starts here
	for(int i=0;i<250000;++i)
		data[i]=(char)((i%10)+48);
	tcp_client_send(connected_client,data,250000);  //can be conn1, conn2, conn3
	if(tcp_client_failed(connected_client))
	{
		tcp_client_error(connected_client,&error_str);
		if(error_str!=NULL)
		{
			printf("Send error: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to send response to client\n");
		}
		disconnect_tcp_client(connected_client);
		release_tcp_client(connected_client);
		return; 
	}
	// code to send response ends here

	// cleanup: close this client connection after handling the request
	// Note: this example closes the client connection when the handler finishes.
	disconnect_tcp_client(connected_client);
	release_tcp_client(connected_client);
	}



	int main()
	{
	unsigned short int port=5406;  //User may pick port number from some file or command line argument
	tcp_server* server;
	char* error_str;

	//allocating server
	server=allocate_tcp_server(port);
	if(tcp_server_failed(server))
	{
		tcp_server_error(server,&error_str);
		if(error_str!=NULL)
		{
			printf("%s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to allocate TCP server\n");
		}
		release_tcp_server(server);
		exit(TCP_SERVER_ERROR_EXIT_CODE);
	}

	//setting up handlers
	on_tcp_server_started(server,tcp_server_started_handler);
	on_tcp_server_stopped(server,tcp_server_stopped_handler);
	on_tcp_client_connected(server,tcp_server_client_connected_handler);

	//starting the server
	tcp_start_server(server);
	if(tcp_server_failed(server))
	{
		tcp_server_error(server,&error_str);
		if(error_str!=NULL)
		{
			printf("%s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to start TCP server\n");
		}
		release_tcp_server(server);
		exit(TCP_SERVER_ERROR_EXIT_CODE);
	}

	// cleanup: release server resources
	release_tcp_server(server);

	pthread_exit(NULL); // Note: use pthread_exit() instead of return or exit() to
						// avoid terminating ongoing client handler threads.
}
