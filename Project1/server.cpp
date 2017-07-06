/*Author Chih-Hui Ho*/

/*server*/
 
#include<stdio.h>     // IO     
#include<string.h>    //strlen
#include<sys/socket.h> // define sockaddr structure
#include<arpa/inet.h>  //inet_addr
#define PORT 5566
#define IP "127.0.0.1"
#define MAX_LEN 50
int main(int argc , char *argv[])
{
    //socket_desc is a socket descripter
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;   //create two sockadd_in  
    char *client_message;  //store message from client
    client_message = new char[MAX_LEN];
    char ID[MAX_LEN], pwd[MAX_LEN]; //store ID and pwd from client

    //Create socket

    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)   //check socket validation
    {
        printf("Could not create socket");
    }
    // puts("Socket created");

    //Reuse the Ip if needed

    int on = 1;
    setsockopt( socket_desc, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

    //Prepare the sockaddr_in structure

    memset( &server, 0, sizeof(server) );  /* clear our address */ 
    server.sin_family = AF_INET;  // set address family
    server.sin_addr.s_addr =  inet_addr(IP);  /* this is our address */ 
    server.sin_port = htons( PORT );   /* this is our port number */ 
   

    //Bind

    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    //puts("bind done");
     
    //Listen

    listen(socket_desc , 3);
     
    //Accept and incoming connection

    puts("server : waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
    //accept connection from an incoming client

    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
     printf("server : got connection from %s\n", inet_ntoa(server.sin_addr));

    // authentication 	
	while(  1 )
    	{
                        // send user name message to client
			send(client_sock , "username(10 character)" , 22,0);    
			// recieve message from client
			read_size = recv(client_sock , ID , 2000 , 0);
			ID[read_size] = '\0';
			// send password message to client
			send(client_sock , "password(10 character)" , 22,0);
                        // recieve message from client
			read_size = recv(client_sock , pwd , 2000 , 0) ;
			pwd[read_size] = '\0';		
			// check the correctness of user and pwd
			if(strcmp(ID, "network") == 0 && strcmp(pwd, "network") == 0)
			{
				send(client_sock , "server Log in success" , 21,0);
				break;
			}
			else 
				send(client_sock , "server Log in failure" , 21,0);
			
		
    	}

    printf("Log in as : %s Password %s\n", ID, pwd);
    //Receive a message from client
    while( (read_size = recv(client_sock , client_message , MAX_LEN , 0)) > 0 )
    {
        //Send the message back to client
	
        send(client_sock , client_message , strlen(client_message),0);

	 //Clear message content
	delete[] client_message;
	client_message = new char[MAX_LEN];
	memset(client_message, 0 , sizeof(client_message));
	
    }
    // Client disconnect
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
     
    return 0;
}
