/** 
 *  @file client_template.c
 *  @brief Minimal client template using the csnl library.
*/

/*
 * Template: client_template.c
 * Quick start: set the server address and port (or run with: ./app-client <server> <port>),
 * replace placeholder request/response logic as needed.
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<csnl.h>
/*
 * tcp_connection_get_local_ip(), tcp_connection_get_remote_ip(),
 * and tcp_connection_error() return malloc'd strings that must be freed.
 */
void print_connection_details(tcp_connection *connection)
{
	char *remote_ip;
	char *error_str;
	unsigned short int remote_port;

	remote_ip=tcp_connection_get_remote_ip(connection);
	if(tcp_connection_failed(connection))
	{
		tcp_connection_error(connection,&error_str);
		if(error_str!=NULL)
		{
			printf("Unable to get server remote IP, reason: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to get server remote IP\n");
		}
	}
	else
	{
		printf("Server remote IP: %s\n",remote_ip);
		free(remote_ip);
	}

	remote_port=tcp_connection_get_remote_port(connection);
	if(tcp_connection_failed(connection))
	{
		tcp_connection_error(connection,&error_str);
		if(error_str!=NULL)
		{
			printf("Unable to get server remote port, reason: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to get server remote port\n");
		}
	}
	else
	{
		printf("Server remote port: %u\n",remote_port);
	}
	printf("\n");
}

int main(int argc,char *argv[])
{
	const char *server;
	unsigned short int port;
	tcp_connection *connection;
	char *request_message;
	char *response_message;
	char *error_str;
	uint32_t response_message_length;

	if(argc!=3)
	{
		printf("Usage: %s server_ip_or_name port\n",argv[0]);
		return 0;
	}

	server=argv[1];
	port=(unsigned short int)atoi(argv[2]);

	connection=tcp_connect(server,port);
	if(tcp_connection_failed(connection))
	{
		tcp_connection_error(connection,&error_str);
		if(error_str!=NULL)
		{
			printf("Unable to connect to server, reason: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to connect to server\n");
		}
		release_tcp_connection(connection);
		return TCP_CONNECTION_ERROR_EXIT_CODE;
	}

	print_connection_details(connection);

	request_message="Hello from client";
	tcp_connection_send(connection,request_message,(uint32_t)strlen(request_message));
	if(tcp_connection_failed(connection))
	{
		tcp_connection_error(connection,&error_str);
		if(error_str!=NULL)
		{
			printf("Unable to send request, reason: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to send request\n");
		}
		disconnect_tcp_connection(connection);
		release_tcp_connection(connection);
		return TCP_CONNECTION_ERROR_EXIT_CODE;
	}

	response_message=tcp_connection_receive(connection,&response_message_length);
	if(tcp_connection_failed(connection))
	{
		tcp_connection_error(connection,&error_str);
		if(error_str!=NULL)
		{
			printf("Unable to receive response, reason: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to receive response\n");
		}
		disconnect_tcp_connection(connection);
		release_tcp_connection(connection);
		return TCP_CONNECTION_ERROR_EXIT_CODE;
	}

	printf("Server message: %s\n",response_message);
	free(response_message);

	request_message="STOP_SERVER";
	tcp_connection_send(connection,request_message,(uint32_t)strlen(request_message));
	if(tcp_connection_failed(connection))
	{
		tcp_connection_error(connection,&error_str);
		if(error_str!=NULL)
		{
			printf("Unable to send request, reason: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to send request\n");
		}
		disconnect_tcp_connection(connection);
		release_tcp_connection(connection);
		return TCP_CONNECTION_ERROR_EXIT_CODE;
	}

	disconnect_tcp_connection(connection);
	release_tcp_connection(connection);
	return 0;
}
