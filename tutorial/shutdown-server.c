/**
 *@example shutdown-server.c
 *An example that Client API to send shutdown request.
 *@author Tanmay Shah
*/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<csnl.h>

//Following code should be written by network layer library user to connect to server and send a shutdown request
//Expected output: either successful send or error details.
//Note: each TCP API call is followed by tcp_connection_failed() and tcp_connection_error() for  error handling.

int main()
{
    char data[16];
    char* error_str;

    // connect to server
    tcp_connection* connection=tcp_connect("localhost",5046);
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

    // prepare shutdown command
    strcpy(data,"shutdown-server");
    tcp_connection_send(connection,data,15);  //can be conn1, conn2, conn3
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
            printf("Unable to send shutdown request\n");
        }
        disconnect_tcp_connection(connection);
        release_tcp_connection(connection);
        exit(TCP_CONNECTION_ERROR_EXIT_CODE);
    }

    // cleanup: disconnect and release the connection
    disconnect_tcp_connection(connection);
    release_tcp_connection(connection);

    return 0;
}
