#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<pthread.h>
#include<netdb.h>
#include<ssnl.h>
#define BACKLOGUE_QUEUE_SIZE 1000
#define TCP_HEADER_SIZE 4
#define TCP_RW_BUFFER_SIZE 16384
#define TCP_SEND_LIMIT 4294967295-TCP_HEADER_SIZE

typedef struct _tcp_server
{
unsigned short int port;
int socket_descriptor;
int socket_descriptor_closed;
int keep_running;
int error_number;
int error_type;
void (*on_tcp_server_started)(unsigned short int);
void (*on_tcp_server_stopped)(unsigned short int);
void (*on_tcp_client_connected)(unsigned short int,tcp_server *,tcp_client *);
}tcp_server;

typedef struct _tcp_client
{
int socket_descriptor;
int socket_descriptor_closed;
int error_number;
int error_type;
struct sockaddr_storage client_sockaddr_storage;
socklen_t client_sockaddr_size;
struct _tcp_server *server;
}tcp_client;

void *request_processor(void *);
void raise_tcp_server_stopped_event(tcp_server *);
void raise_tcp_server_started_event(tcp_server *);

tcp_server* allocate_tcp_server(unsigned short int port)
{
char port_str[6];
struct addrinfo hints;
int failed_state;
struct addrinfo* ptr2top_node,*t;
tcp_server* server;
server=(tcp_server *)malloc(sizeof(tcp_server));
if(server==NULL) return NULL;
server->port=0;
server->socket_descriptor=0;
server->socket_descriptor_closed=1;
server->keep_running=0;
server->error_number=0;
server->error_type=' ';
server->on_tcp_server_started=NULL;
server->on_tcp_server_stopped=NULL;
server->on_tcp_client_connected=NULL;
if(port<=0)
{
server->error_number=901;
server->error_type='C';
return server;
}
server->port=port;
memset(&hints,0,sizeof(hints));
hints.ai_family=AF_INET;
hints.ai_socktype=SOCK_STREAM;
hints.ai_flags=AI_PASSIVE;
sprintf(port_str,"%u",server->port);
failed_state=getaddrinfo(NULL,port_str,&hints,&ptr2top_node);
if(failed_state==-1)
{
server->error_number=errno;
server->error_type='G';
return server;
}
for(t=ptr2top_node;t!=NULL;t=t->ai_next)
{
server->socket_descriptor=socket(t->ai_family,t->ai_socktype,t->ai_protocol);
if(server->socket_descriptor==-1) continue;
server->socket_descriptor_closed=0; 
failed_state=bind(server->socket_descriptor,t->ai_addr,t->ai_addrlen);
if(failed_state==-1)
{
close(server->socket_descriptor);
server->socket_descriptor_closed=1;
server->socket_descriptor=0;
continue;
}
break;
}
freeaddrinfo(ptr2top_node);
if(t==NULL)
{
server->error_number=902;
server->error_type='C';
return server;
}
failed_state=listen(server->socket_descriptor,BACKLOGUE_QUEUE_SIZE);
if(failed_state==-1)
{
server->error_number=errno;
server->error_type='P';
close(server->socket_descriptor);
server->socket_descriptor_closed=1;
return server;
}
return server;
}

void release_tcp_server(tcp_server *server)
{
if(server==NULL) return;
if(server->socket_descriptor_closed==0) close(server->socket_descriptor);
free(server);
}

void tcp_start_server(tcp_server *server)
{
int error_number;
int low_memory_counter;
int accept_failure_counter;
pthread_attr_t thread_attr;
pthread_t thread_id;
tcp_client *client;
if(server==NULL) return;
if(server->on_tcp_client_connected==NULL)
{
server->error_number=903;
server->error_type='C';
return;
}
server->keep_running=1;
pthread_attr_init(&thread_attr);
pthread_attr_setdetachstate(&thread_attr,PTHREAD_CREATE_DETACHED);
if(server->on_tcp_server_started!=NULL) raise_tcp_server_started_event(server);
low_memory_counter=0;
accept_failure_counter=0;
while(server->keep_running)
{
client=(tcp_client *)malloc(sizeof(tcp_client));
if(client==NULL)
{
if(low_memory_counter==60*3)
{
server->keep_running=0;
break;
}
sleep(1);
low_memory_counter++;
continue;
}
low_memory_counter=0;
client->client_sockaddr_size=sizeof(struct sockaddr_storage);
client->socket_descriptor=accept(server->socket_descriptor,(struct sockaddr *)&(client->client_sockaddr_storage),&(client->client_sockaddr_size));
if(client->socket_descriptor==-1)
{
free(client);
if(accept_failure_counter==60*3)
{
break;
}
sleep(1);
accept_failure_counter++;
continue;
}
accept_failure_counter=0;
if(server->keep_running==0)
{
close(client->socket_descriptor);
free(client);
break;
}
client->socket_descriptor_closed=0;
client->server=server;
client->error_number=0;
client->error_type=' ';
error_number=pthread_create(&thread_id,&thread_attr,request_processor,(void *)client);
if(error_number!=0)
{
close(client->socket_descriptor);
free(client);
continue;
}
}
if(server->on_tcp_server_stopped!=NULL) raise_tcp_server_stopped_event(server);
}

void tcp_stop_server(tcp_server *server)
{
int socket_descriptor;
char port_str[6];
struct addrinfo hints;
struct addrinfo *ptr2top_node;
struct addrinfo *t;
int failed_state;

if(server==NULL) return;
server->keep_running=0;
memset(&hints,0,sizeof(hints));
hints.ai_family=AF_INET;
hints.ai_socktype=SOCK_STREAM;
sprintf(port_str,"%u",server->port);
failed_state=getaddrinfo("localhost",port_str,&hints,&ptr2top_node);
if(failed_state==-1) return;
for(t=ptr2top_node;t!=NULL;t=t->ai_next)
{
socket_descriptor=socket(t->ai_family,t->ai_socktype,t->ai_protocol);
if(socket_descriptor==-1) continue;
failed_state=connect(socket_descriptor,t->ai_addr,t->ai_addrlen);
if(failed_state==-1)
{
close(socket_descriptor);
continue;
}
break;
}
freeaddrinfo(ptr2top_node);
if(t==NULL)
{
return;
}
close(socket_descriptor);
}

void *request_processor(void *gen_ptr)
{
tcp_client *client;
if(gen_ptr==NULL) return NULL;
client=(tcp_client *)gen_ptr;
client->server->on_tcp_client_connected(client->server->port,client->server,client);
return NULL;
}

int tcp_server_failed(tcp_server *server)
{
if(server==NULL || server->error_number!=0)
	return 1;
return 0;
}

void tcp_server_error(tcp_server *server, char **error_str)
{
char *error901="Invalid port number";
char *error902="Unable to create socket";
char *error903="Client handler not configured via on_tcp_client_connected function. Refer documentation";
char *error904="Low memory resources. Can't perform operations";
char *error905="Unable to create thread: low resources or insufficient thread permissions";

if(server==NULL || error_str==NULL) return;
if(server->error_number==0)
{
*error_str=NULL;
return;
}
if(server->error_type=='G')
{
*error_str=(char*)malloc(sizeof(char)*(strlen(gai_strerror(server->error_number))+1));
if(*error_str!=NULL) strcpy(*error_str,gai_strerror(server->error_number));
}
else if(server->error_type=='P')
{
*error_str=(char*)malloc(sizeof(char)*(strlen(strerror(server->error_number))+1));
if(*error_str!=NULL) strcpy(*error_str,strerror(server->error_number));
}
else if(server->error_type=='C')
{
if(server->error_number==901)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error901)+1));
if(*error_str!=NULL) strcpy(*error_str,error901);
}
else if(server->error_number==902)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error902)+1));
if(*error_str!=NULL) strcpy(*error_str,error902);
}
else if(server->error_number==903)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error903)+1));
if(*error_str!=NULL) strcpy(*error_str,error903);
}
else if(server->error_number==904)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error904)+1));
if(*error_str!=NULL) strcpy(*error_str,error904);
}
else if(server->error_number==905)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error905)+1));
if(*error_str!=NULL) strcpy(*error_str,error905);
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

void on_tcp_server_started(tcp_server *server, void (*handler)(unsigned short int))
{
if(server==NULL) return;
server->on_tcp_server_started=handler;
}

void on_tcp_server_stopped(tcp_server *server, void (*handler)(unsigned short int))
{
if(server==NULL) return;
server->on_tcp_server_stopped=handler;
}

void on_tcp_client_connected(tcp_server *server, void (*handler)(unsigned short int,tcp_server *,tcp_client *))
{
if(server==NULL) return;
server->on_tcp_client_connected=handler;
}

char* tcp_client_get_local_ip(tcp_client* client)
{
char* local_ip, *t;
int return_value;
struct sockaddr_storage client_sockaddr_storage;
socklen_t client_sockaddr_size;
struct sockaddr_in *client_sockaddr_in;
if(client==NULL) return NULL;
client_sockaddr_size=sizeof(struct sockaddr_storage);
return_value=getsockname(client->socket_descriptor,(struct sockaddr *)&client_sockaddr_storage,&client_sockaddr_size);
if(return_value!=0)
{
client->error_number=811;
client->error_type='C';
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
client->error_number=806;
client->error_type='C';
return NULL;
}
strcpy(local_ip,t);
return local_ip;
}
}
client->error_number=811;
client->error_type='C';
return NULL;
}

unsigned short int tcp_client_get_local_port(tcp_client* client)
{
int return_value;
struct sockaddr_storage client_sockaddr_storage;
socklen_t client_sockaddr_size;
struct sockaddr_in* client_sockaddr_in;
if(client==0) return 0;
client_sockaddr_size=sizeof(struct sockaddr_storage);
return_value=getsockname(client->socket_descriptor,(struct sockaddr*)&client_sockaddr_storage,&client_sockaddr_size);
if(return_value!=0)
{
client->error_number=812;
client->error_type='C';
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
client->error_number=812;
client->error_type='C';
return 0;
}

char* tcp_client_get_remote_ip(tcp_client* client)
{
struct sockaddr_in* client_sockaddr_in;
char *t, *remote_ip;
if(client==NULL) return NULL;
if(client->client_sockaddr_size<=sizeof(struct sockaddr_storage))
{
if(client->client_sockaddr_storage.ss_family==AF_INET)
{
client_sockaddr_in=(struct sockaddr_in*)&(client->client_sockaddr_storage);
t=inet_ntoa(client_sockaddr_in->sin_addr);
remote_ip=(char*)malloc(sizeof(char)*(strlen(t)+1));
if(remote_ip==NULL)
{
client->error_number=806;
client->error_type='C';
return NULL;
}
strcpy(remote_ip,t);
return remote_ip;
}
}
client->error_number=809;
client->error_type='C';
return NULL;
}

unsigned short int tcp_client_get_remote_port(tcp_client* client)
{
struct sockaddr_in* client_sockaddr_in;
if(client==0) return 0;
if(client->client_sockaddr_size<=sizeof(struct sockaddr_storage))
{
if(client->client_sockaddr_storage.ss_family==AF_INET)
{
client_sockaddr_in=(struct sockaddr_in*)&(client->client_sockaddr_storage);
return ntohs(client_sockaddr_in->sin_port);
}
}
client->error_number=810;
client->error_type='C';
return 0;
}

void raise_tcp_server_started_event(tcp_server *server)
{
if(server==NULL) return;
if(server->on_tcp_server_started!=NULL) server->on_tcp_server_started(server->port);
}

void raise_tcp_server_stopped_event(tcp_server *server)
{
if(server==NULL) return;
if(server->on_tcp_server_stopped!=NULL) server->on_tcp_server_stopped(server->port);
}

int tcp_client_failed(tcp_client *client)
{
if(client==NULL || client->error_number!=0)
	return 1;
return 0; 
}

void tcp_client_error(tcp_client *client, char **error_str)
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
if(client==NULL)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error800)+1));
if(*error_str!=NULL) strcpy(*error_str,error800);
}
if(client->error_type=='G')
{
*error_str=(char *)malloc(sizeof(char)*(strlen(gai_strerror(errno)+1)));
if(error_str!=NULL) strcpy(*error_str,gai_strerror(errno));
}
else if(client->error_type=='P')
{
*error_str=(char*)malloc(sizeof(char)*(strlen(strerror(client->error_number))+1));
if(*error_str!=NULL) strcpy(*error_str,strerror(client->error_number));
}
else if(client->error_type=='C')
{
if(client->error_number==801)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error801)+1));
if(*error_str!=NULL) strcpy(*error_str,error801);
}
else if(client->error_number==802)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error802)+1));
if(*error_str!=NULL) strcpy(*error_str,error802);
}
else if(client->error_number==803)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error803)+1));
if(*error_str!=NULL) strcpy(*error_str,error803);
}
else if(client->error_number==804)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error804)+1));
if(*error_str!=NULL) strcpy(*error_str,error804);
}
else if(client->error_number==805)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error805)+1));
if(*error_str!=NULL) strcpy(*error_str,error805);
}
else if(client->error_number==806)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error806)+1));
if(*error_str!=NULL) strcpy(*error_str,error806);
}
else if(client->error_number==807)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error807)+1));
if(*error_str!=NULL) strcpy(*error_str,error807);
}
else if(client->error_number==808)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error808)+1));
if(*error_str!=NULL) strcpy(*error_str,error808);
}
else if(client->error_number==809)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error809)+1));
if(*error_str!=NULL) strcpy(*error_str,error809);
}
else if(client->error_number==810)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error810)+1));
if(*error_str!=NULL) strcpy(*error_str,error810);
}
else if(client->error_number==811)
{
*error_str=(char*)malloc(sizeof(char)*(strlen(error811)+1));
if(*error_str!=NULL) strcpy(*error_str,error811);
}
else if(client->error_number==812)
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

int is_tcp_client_connected(tcp_client *client)
{
if(client==NULL) return 0; // not connected
if(client->socket_descriptor_closed==0) return 1; // still open
return 0; //closed
}

void disconnect_tcp_client(tcp_client *client)
{
if(client==NULL) return;
if(client->socket_descriptor_closed==0)
{
close(client->socket_descriptor);
client->socket_descriptor_closed=1;
client->socket_descriptor=0;
}
}

void release_tcp_client(tcp_client *client)
{
if(client==NULL) return;
if(client->socket_descriptor_closed==0)
	close(client->socket_descriptor);
free(client);
}

void tcp_client_send(tcp_client *client, const char *buffer, uint32_t buffer_size)
{
uint32_t our_buffer_size;
uint32_t our_buffer_index;
uint32_t buffer_index;
uint32_t number_of_bytes_to_send;
uint32_t network_byte_order;
uint32_t pick_in_this_cycle;
char *our_buffer;
ssize_t bytes_sent;

if(client==NULL) return;
if(client->socket_descriptor==-1)
{
client->error_number=803;
client->error_type='C';
return;
}
if(buffer==NULL)
{
client->error_number=804;
client->error_type='C';
return;
}
if(buffer_size==0)
{
client->error_number=804;
client->error_type='C';
return;
}
if(buffer_size>TCP_SEND_LIMIT)
{
client->error_number=805;
client->error_type='C';
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
client->error_number=806;
client->error_type='C';
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
bytes_sent=send(client->socket_descriptor,our_buffer,our_buffer_index+pick_in_this_cycle,0);
if(bytes_sent==-1)
{
client->error_number=errno;
client->error_type='P';
free(our_buffer);
return;
}
buffer_index=bytes_sent-our_buffer_index;
our_buffer_index=0;
number_of_bytes_to_send-=bytes_sent;
}
free(our_buffer);
}

char* tcp_client_receive(tcp_client *client, uint32_t *received_data_size)
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

if(client==NULL) return NULL;
if(received_data_size==NULL)
{
client->error_number=801;
client->error_type='C';
return NULL;
}
*received_data_size=0;
our_buffer_index=0;
while(1)
{
bytes_received=recv(client->socket_descriptor,our_buffer+our_buffer_index,TCP_RW_BUFFER_SIZE-our_buffer_index,0);
if(bytes_received==0)
{
client->error_number=808;
client->error_type='C';
return NULL;
}
else if(bytes_received==-1)
{
client->error_number=errno;
client->error_type='P';
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
client->error_number=806;
client->error_type='C';
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
bytes_received=recv(client->socket_descriptor,receive_buffer+receive_buffer_index,TCP_RW_BUFFER_SIZE,0);
if(bytes_received==0)
{
client->error_number=808;
client->error_type='C';
return NULL;
}
else if(bytes_received==-1)
{
client->error_number=errno;
client->error_type='P';
return NULL;
}
receive_buffer_index=receive_buffer_index+bytes_received;
number_of_bytes_to_receive-=bytes_received;
}
*received_data_size=receive_buffer_size;
return receive_buffer;
}

