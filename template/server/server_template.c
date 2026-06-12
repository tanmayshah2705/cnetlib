/** 
 *  @file server_template.c
 *  @brief Minimal server template using the ssnl library.
*/

/*
 * Template: server_template.c
 * Quick start: set the server port (run with: ./app-server <port>),
 * replace placeholder handlers and add logging/error handling as needed.
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<ssnl.h>
/*
 * Note: tcp_client_get_local_ip(), tcp_client_get_remote_ip(), and 
 * tcp_client_error() return malloc'd strings that must be freed by the user.
 */
void print_connection_details(tcp_client *client)
{
	char *remote_ip;
	char *error_str;
	unsigned short int remote_port;

	remote_ip=tcp_client_get_remote_ip(client);
	if(tcp_client_failed(client))
	{
		tcp_client_error(client,&error_str);
		if(error_str!=NULL)
		{
			printf("Unable to get client remote IP, reason: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to get client remote IP\n");
		}
	}
	else
	{
		printf("Client remote IP: %s\n",remote_ip);
		free(remote_ip);
	}

	remote_port=tcp_client_get_remote_port(client);
	if(tcp_client_failed(client))
	{
		tcp_client_error(client,&error_str);
		if(error_str!=NULL)
		{
			printf("Unable to get client remote port, reason: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to get client remote port\n");
		}
	}
	else
	{
		printf("Client remote port: %u\n",remote_port);
	}
	printf("\n");
}

void server_started_handler(unsigned short int port)
{
	printf("Server started on port %u\n",port);
}

void server_stopped_handler(unsigned short int port)
{
	printf("Server stopped on port %u\n",port);
}

void client_connected_handler(unsigned short int port,tcp_server *server,tcp_client *client)
{
	char *request_message;
	uint32_t request_message_length;
	char *error_str;
	char *response_message;

	printf("Client connected on server port %u\n",port);
	
	print_connection_details(client);

	request_message=tcp_client_receive(client,&request_message_length);
	if(tcp_client_failed(client))
	{
		tcp_client_error(client,&error_str);
		if(error_str!=NULL)
		{
			printf("Unable to receive request, reason: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to receive request\n");
		}
		disconnect_tcp_client(client);
		release_tcp_client(client);
		return;
	}
	printf("Client message: %s\n",request_message);
	free(request_message);
	
	response_message="Hello from server";
	tcp_client_send(client,response_message,(uint32_t)strlen(response_message));
	if(tcp_client_failed(client))
	{
		tcp_client_error(client,&error_str);
		if(error_str!=NULL)
		{
			printf("Unable to send response, reason: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to send response\n");
		}
		disconnect_tcp_client(client);
		release_tcp_client(client);
		return;
	}

	request_message=tcp_client_receive(client,&request_message_length);
	if(tcp_client_failed(client))
	{
		tcp_client_error(client,&error_str);
		if(error_str!=NULL)
		{
			printf("Unable to receive stop-server request, reason: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to receive stop-server request\n");
		}
		disconnect_tcp_client(client);
		release_tcp_client(client);
		return;
	}

	if(request_message_length==11 && strncmp(request_message,"STOP_SERVER",11)==0)
	{
		printf("Stop-server request received\n");
		free(request_message);
		tcp_stop_server(server);
		disconnect_tcp_client(client);
		release_tcp_client(client);
		release_tcp_server(server);
		exit(0);
	}
	else
	{
		printf("Unknown request received\n");
		free(request_message);
	}

	if(is_tcp_client_connected(client))
	{
		disconnect_tcp_client(client);
	}
	release_tcp_client(client);
}

int main(int argc,char *argv[])
{
	unsigned short int port;
	tcp_server *server;
	char *error_str;

	if(argc!=2)
	{
		printf("Usage: %s port\n",argv[0]);
		return 0;
	}

	port=(unsigned short int)atoi(argv[1]);

	server=allocate_tcp_server(port);
	if(tcp_server_failed(server))
	{
		tcp_server_error(server,&error_str);
		if(error_str!=NULL)
		{
			printf("Unable to allocate TCP server, reason: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to allocate TCP server\n");
		}
		release_tcp_server(server);
		return TCP_SERVER_ERROR_EXIT_CODE;
	}

	on_tcp_server_started(server,server_started_handler);
	on_tcp_server_stopped(server,server_stopped_handler);
	on_tcp_client_connected(server,client_connected_handler);

	tcp_start_server(server);
	if(tcp_server_failed(server))
	{
		tcp_server_error(server,&error_str);
		if(error_str!=NULL)
		{
			printf("Unable to start TCP server, reason: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to start TCP server\n");
		}
		release_tcp_server(server);
		return TCP_SERVER_ERROR_EXIT_CODE;
	}

	release_tcp_server(server);
	pthread_exit(NULL);
}
