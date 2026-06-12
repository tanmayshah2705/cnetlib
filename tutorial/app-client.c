/**
 *@example app-client.c
 *@author Tanmay Shah
 *@brief An example that uses Client API to send and receive data.
*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<csnl.h>

//Following code should be written by network layer library user to connect to server and send receive data 
//Expected output: prints remote/local IP and port information, then sends and receives a server response.
//Note: each TCP API call is followed by tcp_connection_failed() and tcp_connection_error() for error handling.

int main()
{
    const char *server_ip="localhost";  //User may pick server ip address from some file or command line argument
    unsigned short int server_port=5406;  //User may pick port number from some file or command line argument
    tcp_connection* connection;
    char data[250000];
    char *response;
    uint32_t response_size;
    char *error_str;
    char *local_ip, *remote_ip;
    unsigned short int local_port, remote_port;

    // connect to server
    connection=tcp_connect(server_ip,server_port);
    if(tcp_connection_failed(connection))
    {
        tcp_connection_error(connection,&error_str); // Note: tcp_connection_error() allocates memory in error_str, must be freed after use
        if(error_str!=NULL)
        {
            printf("%s\n",error_str);
            free(error_str);
        }
        else
        {
            printf("Unable to connect to server\n");
        }
        disconnect_tcp_connection(connection);
        release_tcp_connection(connection);
        exit(TCP_CONNECTION_ERROR_EXIT_CODE);
    }

	// code to get server details starts here
	remote_ip=tcp_connection_get_remote_ip(connection);  // returns malloc'd string, free after use
	if(tcp_connection_failed(connection))
	{
		tcp_connection_error(connection,&error_str);
		if(error_str!=NULL)
		{
			printf("Remote IP : %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Remote IP : Unable to get server remote IP\n");
		}
	}
	else
	{
		printf("Remote IP : %s\n",remote_ip);
		free(remote_ip);
	}

	remote_port=tcp_connection_get_remote_port(connection);
	if(tcp_connection_failed(connection))
	{
		tcp_connection_error(connection,&error_str);
		if(error_str!=NULL)
		{
			printf("Remote port : %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Remote port : Unable to get server remote port\n");
		}
	}
	else
	{
		printf("Remote port : %u\n",remote_port);
	}
	// code to get server details ends here

	// code to get local client details starts here
	local_ip=tcp_connection_get_local_ip(connection);  // returns malloc'd string, free after use
	if(tcp_connection_failed(connection))
	{
		tcp_connection_error(connection,&error_str);
		if(error_str!=NULL)
		{
			printf("Local IP : %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Local IP : Unable to get local IP\n");
		}
	}
	else
	{
		printf("Local IP : %s\n",local_ip);
		free(local_ip);
	}

	local_port=tcp_connection_get_local_port(connection);
	if(tcp_connection_failed(connection))
	{
		tcp_connection_error(connection,&error_str);
		if(error_str!=NULL)
		{
			printf("Local port : %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Local port : Unable to get local port\n");
		}
	}
	else
	{
		printf("Local port : %u\n",local_port);
	}
	// code to get local client details ends here

	// code to send starts here
	for(int i=0;i<250000;++i)
		data[i]=(char)((i%10)+48);
	tcp_connection_send(connection,data,250000);  //can be conn1, conn2, conn3
	if(tcp_connection_failed(connection))
	{
		tcp_connection_error(connection,&error_str);
		if(error_str!=NULL)
		{
			printf("Send error: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to send data to server\n");
		}
		disconnect_tcp_connection(connection);
		release_tcp_connection(connection);
		exit(TCP_CONNECTION_ERROR_EXIT_CODE);
	}
	// code to send ends here

	// code to receive starts here
	response=tcp_connection_receive(connection,&response_size);
	if(tcp_connection_failed(connection))
	{
		tcp_connection_error(connection,&error_str);
		if(error_str!=NULL)
		{
			printf("Receive error: %s\n",error_str);
			free(error_str);
		}
		else
		{
			printf("Unable to receive data\n");
		}
		disconnect_tcp_connection(connection);
		release_tcp_connection(connection);
		exit(TCP_CONNECTION_ERROR_EXIT_CODE);
	}

	//extract data from response
	printf("Number of bytes received in response : %u\n",response_size);
	for(int i=0;i<response_size;++i) printf("%c",response[i]);
	printf("\n");
	free(response);									

	// cleanup: close connection and release resources
	// Note: always disconnect and release the connection when finished.
	disconnect_tcp_connection(connection);
	release_tcp_connection(connection);

	return 0;
}
