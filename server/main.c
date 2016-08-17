/*
 C socket server application, handles multiple clients using threads
 handling muliple connection
 
 connecting process
 1. login with ID/PW
 2. clients needs sending local time each 7 secs
 3. server needs sending "ACK" back to client
 */

#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread

//the thread function
void *connection_handler(void *);
void *server_TCP_handler(void *temp);
void *server_UDP_handler(void *temp);

int main(int argc , char *argv[])
{
    pthread_t server_tcp_thread;
    pthread_t server_udp_thread;
    char input_msg[128];
    
    if( pthread_create( &server_tcp_thread , NULL ,  server_TCP_handler , (void*) NULL) < 0)
    {
        perror("could not create thread");
        return 1;
    }
    
    if( pthread_create( &server_udp_thread , NULL ,  server_UDP_handler , (void*) NULL) < 0)
    {
        perror("could not create thread");
        return 1;
    }
    
    
    scanf("%s" , input_msg);
    
    return 0;
}
////////login ID/PW table



int check_login(char* pID,char* pPW)
{
    printf("login ID:%s",pID);
    printf("login PW:%s",pPW);
    return 1;
}

void *server_UDP_handler(void *temp)
{
    puts("starting UDP server....");
    
    int udpSocket, nBytes;
    char buffer[1024];
    struct sockaddr_in serverAddr, clientAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size, client_addr_size;
    int i;
    
    /*Create UDP socket*/
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);
    
    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7891);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    
    /*Bind socket with address struct*/
    bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    
    /*Initialize size variable to be used later on*/
    addr_size = sizeof serverStorage;
    
    while(1){
        /* Try to receive any incoming UDP datagram. Address and port of
         requesting client will be stored on serverStorage variable */
        nBytes = recvfrom(udpSocket,buffer,1024,0,(struct sockaddr *)&serverStorage, &addr_size);
        
        /*Convert message received to uppercase*/
        for(i=0;i<nBytes-1;i++)
            buffer[i] = toupper(buffer[i]);
        
        puts(buffer);
        /*Send uppercase message back to client, using serverStorage as the address*/
        sendto(udpSocket,buffer,nBytes,0,(struct sockaddr *)&serverStorage,addr_size);
    }
    //end
    return 0;
}

void *server_TCP_handler(void *temp)
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
    
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("TCP server Socket created");
    
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
    
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 0;
    }
    puts("TCP bind done");
    
    //Listen
    listen(socket_desc , 3);
    
    //Accept and incoming connection
    puts("TCP Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    
    
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
        
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
        
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 0;
        }
        
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
    }
    
    if (client_sock < 0)
    {
        perror("accept failed");
        return 0;
    }

    return 0;
}

/*
 * This will handle connection for each client
 
 [cmd][msg][lenght][checksum]
 [cmd]
 L:login     [msg]:[login ID]:[login PW]
 M:message   [msg]:[message string]
 
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];
    int login_state=0;
    
    //Send some messages to the client
    message = "Greetings! I am your connection handler\n";
    write(sock , message , strlen(message));
    
    message = "Now type something and i shall repeat what you type \n";
    write(sock , message , strlen(message));
    
    //login ID/PW
    
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //Send the message back to client
        //write(sock , client_message , strlen(client_message));
        puts(client_message);
        switch(client_message[0])
        {
                case 'L':
                {
                    
                    
                    char* loginID = strtok(message, ",");
                    char* loginPW=strtok(NULL, message);
                    
                    if (check_login(loginID,loginPW)) {
                        login_state=1;
                        message="login success\n";
                        //write(sock , message , strlen(message));
                    }
                }
                break;
                case 'A':
                  message="ACK\n";
                  write(sock , message , strlen(message));
                break;
                case 'M':
                    write(sock , client_message , strlen(client_message));
                break;
                case 'T':
                //write(sock , client_message , strlen(client_message));
                message="ACK\n";
                break;
        }
        //memset(client_message, 0, 20000);
    }
    
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
    
    //Free the socket pointer
    free(socket_desc);
    
    return 0;
}

//database api
#define DB "database.csv" /* database name */
#define TRY(a)  if (!(a)) {perror(#a);exit(1);}
#define TRY2(a) if((a)<0) {perror(#a);exit(1);}
#define FREE(a) if(a) {free(a);a=NULL;}
#define sort_by(foo) \
static int by_##foo (const void*p1, const void*p2) { \
return strcmp ((*(const pdb_t*)p1)->foo, (*(const pdb_t*)p2)->foo); }
typedef struct db {
    char title[26];
    char first_name[26];
    char last_name[26];
    time_t date;
    char publ[100];
    struct db *next;
}db_t,*pdb_t;
typedef int (sort)(const void*, const void*);
enum {CREATE,PRINT,TITLE,DATE,AUTH,READLINE,READ,SORT,DESTROY};
static pdb_t dao (int cmd, FILE *f, pdb_t db, sort sortby);
static char *time2str (time_t *time);
static time_t str2time (char *date);


