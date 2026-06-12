#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<csnl.h>
#define TCP_HEADER_SIZE 4
#define TCP_SEND_LIMIT 4294967295-TCP_HEADER_SIZE
#define TCP_RW_BUFFER_SIZE 16384 

typedef struct _tcp_connection 
{
int socket_descriptor;
int socket_descriptor_closed;
unsigned short int port;
int error_number;
int error_type;
}tcp_connection;

tcp_connection *tcp_connect(const char *server_ip, unsigned short int port)
{
char port_str[6];
struct addrinfo hints;
struct addrinfo *ptr2top_node, *t;
int failed_state;
tcp_connection *connection;
connection=(tcp_connection*)malloc(sizeof(tcp_connection));
if(connection==NULL) return connection;
connection->socket_descriptor=0;
connection->socket_descriptor_closed=1;
connection->port=0;
connection->error_number=0;
connection->error_type=' ';
if(port<=0)
{
connection->error_number=801;
connection->error_type='C';
return connection;
}
connection->port=port;
memset(&hints,0,sizeof(hints));
hints.ai_family=AF_INET;
hints.ai_socktype=SOCK_STREAM;
sprintf(port_str,"%u",port);
failed_state=getaddrinfo(server_ip,port_str,&hints,&ptr2top_node);
if(failed_state==-1)
{
connection->error_number=errno;
connection->error_type='G';
return connection;
}
for(t=ptr2top_node;t!=NULL;t=t->ai_next)
{
connection->socket_descriptor=socket(t->ai_family,t->ai_socktype,t->ai_protocol);
if(connection->socket_descriptor==-1) continue;
connection->socket_descriptor_closed=0;
failed_state=connect(connection->socket_descriptor,t->ai_addr,t->ai_addrlen);
if(failed_state==-1)
{
close(connection->socket_descriptor);
connection->socket_descriptor=0;
connection->socket_descriptor_closed=1;
continue;
}
break;
}
freeaddrinfo(ptr2top_node);
if(t==NULL)
{
connection->error_number=802;
connection->error_type='C';
return connection;
}
return connection;
}

int tcp_connection_failed(tcp_connection *connection)
{
if(connection==NULL || connection->error_number!=0)
	return 1;
return 0;
}

void tcp_connection_error(tcp_connection *connection, char **error_str)
{
char *error800="Connection not established";
char *error801="Invalid port number";
char *error802="Unable to connect";
char *error803="Connection is closed or not yet established";
char *error804="No data to send";
char *error805="Can't send more than 4294967295 bytes in one go";
char *error806="Low memory";
char *error807="Buffer size argument is NULL";
char *error808="Connection closed by peer";
char *error809="Can't determine remote IP";
char *error810="Can't determine remotr port";
char *error811="Can't determine local IP";
char *error812="Can't determine local port";

if(error_str==NULL) return;
if(connection==NULL)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error800)+1));
if(*error_str!=NULL) strcpy(*error_str,error800);
}
if(connection->error_type=='G')
{
*error_str=(char *)malloc(sizeof(char)*(strlen(gai_strerror(errno)+1)));
if(error_str!=NULL) strcpy(*error_str,gai_strerror(errno));
}
else if(connection->error_type=='P')
{
*error_str=(char*)malloc(sizeof(char)*(strlen(strerror(connection->error_number))+1));
if(*error_str!=NULL) strcpy(*error_str,strerror(connection->error_number));
}
else if(connection->error_type=='C')
{
if(connection->error_number==801)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error801)+1));
if(*error_str!=NULL) strcpy(*error_str,error801);
}
else if(connection->error_number==802)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error802)+1));
if(*error_str!=NULL) strcpy(*error_str,error802);
}
else if(connection->error_number==803)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error803)+1));
if(*error_str!=NULL) strcpy(*error_str,error803);
}
else if(connection->error_number==804)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error804)+1));
if(*error_str!=NULL) strcpy(*error_str,error804);
}
else if(connection->error_number==805)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error805)+1));
if(*error_str!=NULL) strcpy(*error_str,error805);
}
else if(connection->error_number==806)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error806)+1));
if(*error_str!=NULL) strcpy(*error_str,error806);
}
else if(connection->error_number==807)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error807)+1));
if(*error_str!=NULL) strcpy(*error_str,error807);
}
else if(connection->error_number==808)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error808)+1));
if(*error_str!=NULL) strcpy(*error_str,error808);
}
else if(connection->error_number==809)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error809)+1));
if(*error_str!=NULL) strcpy(*error_str,error809);
}
else if(connection->error_number==810)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error810)+1));
if(*error_str!=NULL) strcpy(*error_str,error810);
}
else if(connection->error_number==811)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error811)+1));
if(*error_str!=NULL) strcpy(*error_str,error811);
}
else if(connection->error_number==812)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error812)+1));
if(*error_str!=NULL) strcpy(*error_str,error812);
}
else
{
*error_str=NULL;
}
}
else
{
*error_str=NULL;
}
}

void disconnect_tcp_connection(tcp_connection *connection)
{
if(connection==NULL) return;
if(connection->socket_descriptor_closed==0)
{
close(connection->socket_descriptor);
connection->socket_descriptor_closed=1;
connection->socket_descriptor=0;
}
}

void release_tcp_connection(tcp_connection *connection)
{
if(connection==NULL) return;
if(connection->socket_descriptor_closed==0)
	close(connection->socket_descriptor);
free(connection);
}

void tcp_connection_send(tcp_connection *connection, const char *buffer, uint32_t buffer_size)
{
uint32_t our_buffer_size;
uint32_t our_buffer_index;
uint32_t buffer_index;
uint32_t number_of_bytes_to_send;
uint32_t network_byte_order;
uint32_t pick_in_this_cycle;
char *our_buffer;
ssize_t bytes_sent;
if(connection==NULL) return;
if(connection->socket_descriptor==-1)
{
connection->error_number=803;
connection->error_type='C';
return;
}
if(buffer==NULL)
{
connection->error_number=804;
connection->error_type='C';
return;
}
if(buffer_size==0)
{
connection->error_number=804;
connection->error_type='C';
return;
}
if(buffer_size>TCP_SEND_LIMIT)
{
connection->error_number=805;
connection->error_type='C';
return;
}
if(buffer_size+TCP_HEADER_SIZE<=TCP_RW_BUFFER_SIZE)
{
our_buffer_size=buffer_size+TCP_HEADER_SIZE;
}
else
{
our_buffer_size=TCP_RW_BUFFER_SIZE;
}
our_buffer=(char*)malloc(our_buffer_size);
if(our_buffer==NULL)
{
connection->error_number=806;
connection->error_type='C';
return;
}
network_byte_order=htonl(buffer_size);
memcpy(our_buffer,&network_byte_order,sizeof(uint32_t));
number_of_bytes_to_send=buffer_size+TCP_HEADER_SIZE;
our_buffer_index=TCP_HEADER_SIZE;
buffer_index=0;
while(number_of_bytes_to_send>0)
{
if(number_of_bytes_to_send>(our_buffer_size-our_buffer_index))
{
pick_in_this_cycle=our_buffer_size-our_buffer_index;
}
else
{
pick_in_this_cycle=number_of_bytes_to_send-our_buffer_index;
}
memcpy(our_buffer+our_buffer_index,buffer+buffer_index,pick_in_this_cycle);
bytes_sent=send(connection->socket_descriptor,our_buffer,our_buffer_index+pick_in_this_cycle,0);
if(bytes_sent==-1)
{
connection->error_number=errno;
connection->error_type='P';
free(our_buffer);
return;
}
buffer_index=bytes_sent-our_buffer_index;
our_buffer_index=0;
number_of_bytes_to_send-=bytes_sent;
}
free(our_buffer);
}

char* tcp_connection_receive(tcp_connection *connection, uint32_t *received_data_size)
{
uint32_t number_of_bytes_to_receive;
uint32_t network_byte_order;
uint32_t host_byte_order;
char *receive_buffer;
uint32_t receive_buffer_size;
uint32_t receive_buffer_index;
char our_buffer[TCP_RW_BUFFER_SIZE];
uint32_t our_buffer_index;
ssize_t bytes_received;

if(connection==NULL) return NULL;
if(received_data_size==NULL)
{
connection->error_number=801;
connection->error_type='C';
return NULL;
}
*received_data_size=0;
our_buffer_index=0;
while(1)
{
bytes_received=recv(connection->socket_descriptor,our_buffer+our_buffer_index,TCP_RW_BUFFER_SIZE-our_buffer_index,0);
if(bytes_received==0)
{
connection->error_number=808;
connection->error_type='C';
return NULL;
}
else if(bytes_received==-1)
{
connection->error_number=errno;
connection->error_type='P';
return NULL;
}
our_buffer_index=our_buffer_index+bytes_received;
if(our_buffer_index>=4) break;
}
memcpy(&network_byte_order,our_buffer,sizeof(uint32_t));
host_byte_order=ntohl(network_byte_order);
number_of_bytes_to_receive=host_byte_order;
receive_buffer_size=number_of_bytes_to_receive;
receive_buffer=(char*)malloc(receive_buffer_size);
if(receive_buffer==NULL)
{
connection->error_number=806;
connection->error_type='C';
return NULL;
}
receive_buffer_index=0;
if(our_buffer_index>4)
{
memcpy(receive_buffer,our_buffer+TCP_HEADER_SIZE,our_buffer_index-TCP_HEADER_SIZE);
receive_buffer_index=our_buffer_index-TCP_HEADER_SIZE;
}
number_of_bytes_to_receive-=(our_buffer_index-TCP_HEADER_SIZE);
while(number_of_bytes_to_receive>0)
{
bytes_received=recv(connection->socket_descriptor,receive_buffer+receive_buffer_index,TCP_RW_BUFFER_SIZE,0);
if(bytes_received==0)
{
connection->error_number=808;
connection->error_type='C';
return NULL;
}
else if(bytes_received==-1)
{
connection->error_number=errno;
connection->error_type='P';
return NULL;
}
receive_buffer_index=receive_buffer_index+bytes_received;
number_of_bytes_to_receive-=bytes_received;
}
*received_data_size=receive_buffer_size;
return receive_buffer;
}

char* tcp_connection_get_local_ip(tcp_connection* connection)
{
char* local_ip, *t;
int return_value;
struct sockaddr_storage client_sockaddr_storage;
socklen_t client_sockaddr_size;
struct sockaddr_in *client_sockaddr_in;
if(connection==NULL) return NULL;
client_sockaddr_size=sizeof(struct sockaddr_storage);
return_value=getsockname(connection->socket_descriptor,(struct sockaddr *)&client_sockaddr_storage,&client_sockaddr_size);
if(return_value!=0)
{
connection->error_number=811;
connection->error_type='C';
return NULL;
}
if(client_sockaddr_size<=sizeof(struct sockaddr_storage))
{
if(client_sockaddr_storage.ss_family==AF_INET)
{
client_sockaddr_in=(struct sockaddr_in*)&client_sockaddr_storage;
t=inet_ntoa(client_sockaddr_in->sin_addr);
local_ip=(char *)malloc(sizeof(char)*(strlen(t)+1));
if(local_ip==0)
{
connection->error_number=806;
connection->error_type='C';
return NULL;
}
strcpy(local_ip,t);
return local_ip;
}
}
connection->error_number=811;
connection->error_type='C';
return NULL;
}

unsigned short int tcp_connection_get_local_port(tcp_connection* connection)
{
int return_value;
struct sockaddr_storage client_sockaddr_storage;
socklen_t client_sockaddr_size;
struct sockaddr_in* client_sockaddr_in;
if(connection==0) return 0;
client_sockaddr_size=sizeof(struct sockaddr_storage);
return_value=getsockname(connection->socket_descriptor,(struct sockaddr*)&client_sockaddr_storage,&client_sockaddr_size);
if(return_value!=0)
{
connection->error_number=812;
connection->error_type='C';
return 0;
}
if(client_sockaddr_size<=sizeof(struct sockaddr_storage))
{
if(client_sockaddr_storage.ss_family==AF_INET)
{
client_sockaddr_in=(struct sockaddr_in*)&client_sockaddr_storage;
return ntohs(client_sockaddr_in->sin_port);
}
}
connection->error_number=812;
connection->error_type='C';
return 0;
}

char* tcp_connection_get_remote_ip(tcp_connection* connection)
{
char* local_ip, *t;
int return_value;
struct sockaddr_storage client_sockaddr_storage;
socklen_t client_sockaddr_size;
struct sockaddr_in *client_sockaddr_in;
if(connection==NULL) return NULL;
client_sockaddr_size=sizeof(struct sockaddr_storage);
return_value=getpeername(connection->socket_descriptor,(struct sockaddr *)&client_sockaddr_storage,&client_sockaddr_size);
if(return_value!=0)
{
connection->error_number=809;
connection->error_type='C';
return NULL;
}
if(client_sockaddr_size<=sizeof(struct sockaddr_storage))
{
if(client_sockaddr_storage.ss_family==AF_INET)
{
client_sockaddr_in=(struct sockaddr_in*)&client_sockaddr_storage;
t=inet_ntoa(client_sockaddr_in->sin_addr);
local_ip=(char *)malloc(sizeof(char)*(strlen(t)+1));
if(local_ip==0)
{
connection->error_number=806;
connection->error_type='C';
return NULL;
}
strcpy(local_ip,t);
return local_ip;
}
}
connection->error_number=809;
connection->error_type='C';
return NULL;
}

unsigned short int tcp_connection_get_remote_port(tcp_connection* connection)
{
int return_value;
struct sockaddr_storage client_sockaddr_storage;
socklen_t client_sockaddr_size;
struct sockaddr_in* client_sockaddr_in;
if(connection==0) return 0;
client_sockaddr_size=sizeof(struct sockaddr_storage);
return_value=getpeername(connection->socket_descriptor,(struct sockaddr*)&client_sockaddr_storage,&client_sockaddr_size);
if(return_value!=0)
{
connection->error_number=810;
connection->error_type='C';
return 0;
}
if(client_sockaddr_size<=sizeof(struct sockaddr_storage))
{
if(client_sockaddr_storage.ss_family==AF_INET)
{
client_sockaddr_in=(struct sockaddr_in*)&client_sockaddr_storage;
return ntohs(client_sockaddr_in->sin_port);
}
}
connection->error_number=810;
connection->error_type='C';
return 0;
}
