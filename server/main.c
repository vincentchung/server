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

#define UID_LENGTH 16
#define MESSAGE_SIZE 256
#define ACCOUNT_NUM 5
#define UNICAST_MSG_NUM 8
#define UNICAST_MSG_SIZE 32

//UDP Multicast message
#define HELLO_PORT 12345
#define HELLO_GROUP "225.0.0.37"
//the thread function
void *connection_handler(void *);
void *server_TCP_handler(void *temp);
void *server_UDP_handler(void *temp);
void send_UDP_Multicast(char *message);

typedef struct
{
    char TUID[UID_LENGTH];//send to
    char msg[UNICAST_MSG_SIZE];
    char SUID[UID_LENGTH];//send from
    int issend;
}unicast_msg;


typedef struct
{
    int idkey;
    char UID[UID_LENGTH];
    char PWD[UID_LENGTH];
    int login;
}account;

int front=-1;
int rear=-1;
int bQueueFull=0;

unicast_msg unicast_queue[UNICAST_MSG_NUM];
account user_tb[]={
    {1,"test1","test1",0},
    {2,"test2","test2",0},
    {3,"test3","test3",0},
    {4,"test4","test4",0},
    {5,"test5","test5",0}};
//account table
char UIDARRAY[ACCOUNT_NUM][UID_LENGTH]=
{
    "test1",
    "test2",
    "test3",
    "test4",
    "test5"
};

char UPWDARRAY[ACCOUNT_NUM][16]=
{
    "test1",
    "test2",
    "test3",
    "test4",
    "test5"
};

void flush_queue()
{
    int counter=0;
    for(int i=front;i<rear;i++)
    {
        if(!unicast_queue[i].issend)
        {
            strcpy(  unicast_queue[counter].TUID,unicast_queue[i].TUID);
            strcpy(  unicast_queue[counter].SUID,unicast_queue[i].SUID);
            strcpy(  unicast_queue[counter].msg,unicast_queue[i].msg);
            unicast_queue[counter].issend=0;
            counter++;
        }
    }
    
    front=0;
    rear=counter;
}


void insert_queue(char* uid,char* msg,char* suid)
{
    if (front==-1)
    {
        front=0;
        //memset(queue,-1,queue_max);
    }
    //check if queue is full
    if(bQueueFull)
        return;
    
    rear++;
    strcpy(  unicast_queue[rear].TUID,uid);
    strcpy(  unicast_queue[rear].SUID,suid);
    strcpy(  unicast_queue[rear].msg,msg);
    unicast_queue[rear].issend=0;
    
    if(rear==(UNICAST_MSG_NUM-1))
    {
        if(front==0)
            bQueueFull=1;
        else
            flush_queue();
    }
}

int dequeue(char* uid)
{
    for(int i=0;i<=rear;i++)
    {
        if(!unicast_queue[i].issend)
        {
            if(!strcmp(unicast_queue[i].TUID,uid))
            {
                return i;
            }
        }
    }
    return -1;
}

void listqueue()
{
    puts("listqueue:");
    for(int i=0;i<rear;i++)
    {
        //sprintf(<#char *restrict#>, <#const char *restrict, ...#>)
        puts(unicast_queue[i].SUID);
    }
}
//create one more thread for sending muticast message UDP sender!!!

int main(int argc , char *argv[])
{
    pthread_t server_tcp_thread;
    pthread_t server_udp_thread;
    char input_msg[MESSAGE_SIZE];
    
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
    //while(1)
        
    return 0;
}
////////login ID/PW table



int check_login(char* pID,char* pPW)
{
    //printf("ID:%s,pwd:%s",pID,pPW);
    //puts(pID);
    //puts(pPW);
    //puts(strlen(pPW));
    
    for(int i=0;i<ACCOUNT_NUM;i++)
    {
        if(0==strcmp(pID,UIDARRAY[i]))
        {
            //puts("ID pass");
            
            if(0==strcmp(pPW,UPWDARRAY[i]))
            {
                puts("login pass");
                return 1;
            }else
            {
                puts("PWD fail");
                return 0;
            }
        }
    }
    return 0;
}

void *server_UDP_handler(void *temp)
{
    puts("starting UDP server....");
    char client_message[MESSAGE_SIZE];
    char* message;
    int udpSocket, nBytes;
    struct sockaddr_in serverAddr, clientAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size, client_addr_size;
    int i,login_state=0,testcounter=0;
    
    /*Create UDP socket*/
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);
    
    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7891);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    
    
    /*Bind socket with address struct*/
    bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    
    
    //create a thread to handle
    
    /*Initialize size variable to be used later on*/
    addr_size = sizeof serverStorage;
    
    //message = "connected ACK\n";
    //sendto(udpSocket,message,nBytes,0,(struct sockaddr *)&serverStorage,addr_size);
    //UDP msg format
    //[UID,TCP command format]
    
    while(1){
        /* Try to receive any incoming UDP datagram. Address and port of
         requesting client will be stored on serverStorage variable */
        memset(client_message, 0, MESSAGE_SIZE);
        nBytes = recvfrom(udpSocket,client_message,MESSAGE_SIZE,0,(struct sockaddr *)&serverStorage, &addr_size);
        
        /*Convert message received to uppercase*/
        /*Send uppercase message back to client, using serverStorage as the address*/
        //sendto(udpSocket,buffer,nBytes,0,(struct sockaddr *)&serverStorage,addr_size);
        //char UID[UID_LENGTH];
        //puts(client_message);
        
        char* UID = strtok(client_message, ",");
        char* UDPclient_message=client_message+strlen(UID)+1;
        //puts(UDPclient_message);
        switch(UDPclient_message[0])
        {
            case 'L':
            {
                char* loginID = strtok(UDPclient_message, ",");
                char* loginPW=strtok(NULL, ",");
                
                if (check_login(loginID+1,loginPW)) {
                    login_state=1;
                    message="login success\n";
                    sendto(udpSocket,message,strlen(message),0,(struct sockaddr *)&serverStorage,addr_size);
                }else
                {
                    message="login error\n";
                    sendto(udpSocket,message,strlen(message),0,(struct sockaddr *)&serverStorage,addr_size);
                    //free(udpSocket);
                    //return 0;
                }
                
            }
                break;
            case 'A':
            {
                char temp[6]="ACK\n";
                //if(testcounter<5)
                sendto(udpSocket,temp,strlen(temp),0,(struct sockaddr *)&serverStorage,addr_size);
                testcounter++;
            }
                break;
            case 'M':
            {
                char temp[6]="MACK\n";
                message=UDPclient_message+1;
                send_UDP_Multicast(message);
                
                //if(testcounter<5)
                sendto(udpSocket,temp,strlen(temp),0,(struct sockaddr *)&serverStorage,addr_size);
            }
                break;
            case 'C':
                message = "UDP connected ACK\n";
                sendto(udpSocket,message,strlen(message),0,(struct sockaddr *)&serverStorage,addr_size);
                break;
            case 'P':
            {
                char temp[MESSAGE_SIZE];
                int id=dequeue(UID);
                if(id!=-1)
                {
                    unicast_queue[id].issend=1;
                    sprintf(temp, "%s:%s",unicast_queue[id].SUID,unicast_queue[id].msg);
                    sendto(udpSocket,temp,strlen(temp),0,(struct sockaddr *)&serverStorage,addr_size);
                    
                }else{
                    char temp[6]="NONE";
                    //if(testcounter<5)
                    sendto(udpSocket,temp,strlen(temp),0,(struct sockaddr *)&serverStorage,addr_size);
                    puts("no message");
                }
            }
                break;
            case 'U':
            {
                //send msg to target UID
                char* sendUID = strtok(UDPclient_message, ",");
                char* msg=strtok(NULL, ",");
                char temp[6]="UACK\n";
                insert_queue(sendUID+1, msg, UID);
                //message="UACK\n";
                sendto(udpSocket,temp,strlen(temp),0,(struct sockaddr *)&serverStorage,addr_size);
                listqueue();
            }
                break;
        }
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
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[MESSAGE_SIZE];
    int login_state=0;
    int testcounter=0;
    char UID[UID_LENGTH];
    
    //Send some messages to the client
    while(1)
    {
        if( (read_size = recv(sock , client_message , MESSAGE_SIZE , 0)) > 0 )
        {
            message = "TCP connected ACK\n";
            write(sock , message , strlen(message));
            break;
        }
    }
    
    //login ID/PW
    
    //Receive a message from client
    while(1)
    {
        //listqueue();
        memset(client_message, 0, MESSAGE_SIZE);
        if( (read_size = recv(sock , client_message , MESSAGE_SIZE , 0)) > 0 )
        {
            //Send the message back to client
            //write(sock , client_message , strlen(client_message));
            puts(client_message);
            switch(client_message[0])
            {
                case 'L':
                {
                    char* loginID = strtok(client_message, ",");
                    char* loginPW=strtok(NULL, ",");
                    
                    if (check_login(loginID+1,loginPW)) {
                        login_state=1;
                        message="login success\n";
                        strcpy(UID,loginID+1);
                        write(sock , message , strlen(message));
                    }else
                    {
                        message="login error\n";
                        write(sock , message , strlen(message));
                        free(socket_desc);
                        return 0;
                    }
                    
                }
                    break;
                case 'A':
                {
                    char temp[6]="ACK\n";
                    write(sock , temp , strlen(message));
                    testcounter++;
                }
                    break;
                case 'M':
                {
                    message=client_message+1;
                    send_UDP_Multicast(message);
                    char temp[6]="MACK\n";
                    write(sock , temp , strlen(message));
                }
                    break;
                case 'T':
                    //write(sock , client_message , strlen(client_message));
                    message="ACK\n";
                    break;
                case 'U':
                {
                    //send msg to target UID
                    char* sendUID = strtok(client_message, ",");
                    char* msg=strtok(NULL, ",");
                    insert_queue(sendUID+1, msg, UID);
                    char temp[6]="UACK\n";
                    write(sock , temp , strlen(message));
                    listqueue();
                                    }
                    break;
            }
            //memset(client_message, 0, 20000);
        }
        //sending unicast message
        {
            char temp[MESSAGE_SIZE];
            int id=dequeue(UID);
            if(id!=-1)
            {
                unicast_queue[id].issend=1;
                sprintf(temp, "%s:%s",unicast_queue[id].SUID,unicast_queue[id].msg);
                write(sock , temp , strlen(temp));
                
            }
        }
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
//handling message
/*
 
 [cmd][msg][lenght][checksum]
 [cmd]
 L:login     format:L[UID]:[login PW]
 M:message   format:M[UID]:[message string]
 B:broadcast format:B[message string]
 */

//sending multicast message

void send_UDP_Multicast(char *message)
{
    struct sockaddr_in addr;
    int fd, cnt;
    struct ip_mreq mreq;
    
    /* create what looks like an ordinary UDP socket */
    if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
        perror("socket");
        return;
    }
    
    /* set up destination address */
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(HELLO_GROUP);
    addr.sin_port=htons(HELLO_PORT);
    
    /* now just sendto() our destination! */
    if (sendto(fd,message,sizeof(message),0,(struct sockaddr *) &addr,
               sizeof(addr)) < 0) {
        perror("sendto");
        return;
    }
}