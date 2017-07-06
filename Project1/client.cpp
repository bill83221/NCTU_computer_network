/*Author Chih-Hui Ho*/
/* client*/
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include <unistd.h>   // close socket
#define PORT 5566
#define IP "127.0.0.1"
int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;   //create a sockadd_in  
    char *message , *server_reply;  // store message to send and replied message
    message = new char [1000];
    server_reply = new char [2000];
    char check[1000];               // store authentication message  

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)   //check socket validation
    {
        printf("Could not create socket");
    }
   // puts("Socket created");
     
    // set server info 
    server.sin_addr.s_addr = inet_addr(IP);
    server.sin_family = AF_INET;
    server.sin_port = htons( PORT);
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
    printf("connecting to %s\n",inet_ntoa(server.sin_addr) );

    // client authentication
     while(1)
    {

	int recv_num = 0 ;
      	if( (recv_num = recv(sock , check , 2000 , 0)) < 0 ) // recieve message from server
	{
		        perror("receive error");
        		return 1;
	}
	else  
	{
		check[recv_num] = '\0';
		puts(check);
		if(strcmp(check, "username(10 character)") ==0 || strcmp(check, "password(10 character)") ==0) // enter username and pwd
		{
			
			scanf("%s" , message);
			
			 if( send(sock , message , strlen(message) , 0) < 0)  // send username and pwd to server
        		{
           			 puts("Send failed");
            			return 1;
        		}
		}
		else if(strcmp(check, "server Log in success") ==0)    // log in success
		  break;
	
	}

    }  
 
    //keep communicating with server and server send back what is ever enter
    while(1)
    {
	
	
        printf("Enter message : ");
        scanf("%s" , message);			
         if(message[0] == 'q' && strlen(message) == 1)    // disconnect if meessage = 'q'
	break;
        //Send some data
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");   // send failure
            return 1;
        }
         
        //Receive a reply from the server
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            puts("recv failed");
            break;
        }
         
        puts("Server reply :");
        puts(server_reply);
	memset(server_reply, 0 , sizeof(server_reply));  // clear reply
    }
    // close socket  
    close(sock);
    return 0;
}
