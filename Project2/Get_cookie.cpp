/*Author Chih-Hui Ho*/
//Header Files
#include<stdio.h> 		//printf
#include<string.h>    		//strlen
#include<stdlib.h>    		//malloc
#include<sys/socket.h>    	//you know what this is for
#include<arpa/inet.h> 		//inet_addr , inet_ntoa , ntohs etc
#include<netinet/in.h>
#include<unistd.h>    		//getpid
#include <iostream> 


 
#define CERT 37 		//CERT query



//Function Prototypes
void ngethostbyname (unsigned char* , int);
void AssignToQname(unsigned char*,unsigned char*);
unsigned char* ReadName (unsigned char*,unsigned char*,int*);
void get_dns_servers();
void PrintAnswer(struct DNS_HEADER * ,struct Answer * ); 
void PrintAddit(struct DNS_HEADER * dns, struct Addit * addit);
void PrintAuth(struct DNS_HEADER * dns, struct Auth * auth);
void SetDns(struct DNS_HEADER *dns);
void SetQuery(struct QUESTION *qinfo);
struct Length
{
    unsigned short l;
};

//DNS header structure
/*
                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QDCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ANCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NSCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ARCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/

#pragma pack(push, 1)
struct DNS_HEADER
{
   
    unsigned short id; // identification number

    unsigned char qr :1; // query/response flag
    unsigned char opcode :4; // purpose of message
    unsigned char aa :1; // authoritive answer
    unsigned char tc :1; // truncated message
    unsigned char rd :1; // recursion desired

    unsigned char ra :1; // recursion available
    unsigned char z :1; // its z! reserved
    unsigned char ad :1; // authenticated data
    unsigned char cd :1; // checking disabled
    unsigned char rcode :4; // response code
 
    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count; // number of resource entries
};
 #pragma pack(pop)

//Constant sized fields of query structure
struct QUESTION
{
    unsigned short qtype;
    unsigned short qclass;
};
 
//Constant sized part of answer
#pragma pack(push, 1)
struct DATA
{  
    unsigned short name ;
    unsigned short type_cert;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
    unsigned short type_spki;
    unsigned short key_footprint;
    unsigned short algorithm :8;
};
#pragma pack(pop)

//Pointers to Answer
struct Answer
{
    struct DATA *resource;
    unsigned char *key;
};


//Constant sized part of authoritative record
#pragma pack(push, 1)
 struct Author
{
    unsigned short name;
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
    unsigned int name_server;
};
#pragma pack(pop)

// Pointers to authoritative records
struct Auth
{
    struct Author *author;
};

// Pointers to Additional Records
#pragma pack(push, 1)
 struct Addition
{
    unsigned short name;
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
    unsigned int addr :5;
};

#pragma pack(pop)
struct Addit
{
    struct Addition *addition;
};


using namespace std;
int main( int argc , char *argv[])
{
    unsigned char hostname[16] = "140.113.216.171";
          
    //Now get the ip of this hostname , A record
    unsigned char buf[65536],*qname,*reader;
  
 
    struct sockaddr_in a;
    //the replies from the DNS server
    struct Answer answers[20];
    struct Auth auth[20];
    struct Addit addit[20]; 
    struct sockaddr_in dest;
    struct Length *len = NULL;
    struct DNS_HEADER *dns = NULL;
    struct QUESTION *qinfo = NULL;
 
    printf("Resolving %s" , hostname);
 
    int s = socket(AF_INET , SOCK_STREAM , 0); //TCP packet for DNS queries
 
    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    dest.sin_addr.s_addr = inet_addr("140.113.216.171"); //dns servers
    if (connect(s , (struct sockaddr *)&dest , sizeof(dest)) < 0)
    {
        perror("connect failed. Error");
        
    }
    printf("\nconnecting to %s\n",inet_ntoa(dest.sin_addr) );
    

    //Set the DNS structure to standard queries
    dns = (struct DNS_HEADER *)&buf[sizeof(unsigned short)];
    
    SetDns(dns);
   //point to the query portion
    qname =(unsigned char*)&buf[sizeof(struct DNS_HEADER)+sizeof(struct Length)];
    
    
     unsigned char fqdn[] ="07b3244cea492e1c3af49ccff193ecfcc2871d74.network.cs.nctu.edu.tw";
     AssignToQname(qname , fqdn);

    qinfo =(struct QUESTION*)&buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) )+sizeof(struct Length)+1]; //fill it
     
    SetQuery(qinfo);

      len = (struct Length *)&buf;
    len->l = htons(sizeof(struct DNS_HEADER) + (strlen((const char*)qname)) + sizeof(struct QUESTION)+1);


    printf("\nSending Packet...");
    if( write(s,(char*)buf,sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+sizeof(struct Length)) + sizeof(struct QUESTION)+1) < 0)
    {
        perror("send failed");
    }
    printf("Done");
     
    //Receive the answer

    printf("\nReceiving answer...");
    
    if(read (s,(char*)buf , 65536 ) < 0)
    {
        perror("recv failed");
    }
    printf("Done\n");
    
   dns = (struct DNS_HEADER *)&buf[sizeof(unsigned short)];
 
    //move ahead of the dns header and the query field
    reader = &buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+sizeof(struct Length)+1) + sizeof(struct QUESTION)];
    
    printf("\nThe response contains : ");
    printf("\n %d Questions.",ntohs(dns->q_count));
    printf("\n %d Answers.",ntohs(dns->ans_count));
    printf("\n %d Authoritative Servers.",ntohs(dns->auth_count));
    printf("\n %d Additional records.\n\n",ntohs(dns->add_count));
 
    //Start reading answers

    for(int i=0;i<ntohs(dns->ans_count);i++)
    {
        answers[i].resource = (struct DATA*)(reader);
        reader = reader + sizeof(struct DATA);
 
  
        answers[i].key = (unsigned char*)malloc(1167*sizeof(unsigned char));
	for(int j=0 ; j <1166 ; j ++)
	{
	    answers[i].key[j] = *reader++;
	}
      
    }

   //read authorities
  
   for(int i=0;i<ntohs(dns->auth_count);i++)
    {
        auth[i].author = (struct Author*)(reader);
        reader = reader + sizeof(struct Author);
        
    }
	reader = reader +1;

   //read additional
    for(int i=0;i<ntohs(dns->add_count);i++)
    {
        addit[i].addition = (struct Addition*)(reader);
        reader = reader + sizeof(struct Addition);
        
    }
 
    //print answers
    PrintAnswer(dns, answers);

   
    //print authorities
    PrintAuth(dns,auth);    

    //print additional resource records
    PrintAddit(dns,addit);
 
    return 0;
}

// set up query type CERT for 37
void SetQuery(struct QUESTION *qinfo)
{
    qinfo->qtype = htons( CERT ); //type of the query : CERT
    qinfo->qclass = htons(1); //its internet (lol) 
}
// set dns format
void SetDns(struct DNS_HEADER *dns)
{
 
    dns->id = (unsigned short) htons(getpid());
    dns->qr =  htons(0); //This is a query
    dns->opcode =  htons(0); //This is a standard query
    dns->aa =  htons(0); //Not Authoritative
    dns->tc =  htons(0); //This message is not truncated
    dns->rd = htons(1); //Recursion Desired
    dns->ra =  htons(0); //Recursion not available! hey we dont have it (lol)
    dns->z =  htons(0);
    dns->ad =  htons(0);
    dns->cd =  htons(0);
    dns->rcode = htons(0);
    dns->q_count = htons(1); //we have only 1 question
    dns->ans_count =  htons(0);
    dns->auth_count =  htons(0);
    dns->add_count =  htons(0);
 
 
}
// print out additional RR

void PrintAddit(struct DNS_HEADER * dns, struct Addit * addit)
{
    printf("\nAdditional Records : %d \n" , ntohs(dns->add_count) );
    for(int i=0; i < ntohs(dns->add_count) ; i++)
    {
    printf("name: %d\n",  ntohs(addit[i].addition->name)             );
    printf("type: %d\n",  ntohs(addit[i].addition->type)             );
    printf("class:%d\n", ntohs(addit[i].addition->_class)            );
    printf("ttl: %d\n",  ntohs(addit[i].addition->ttl)               );
    printf("data_len: %d\n", ntohs(addit[i].addition->data_len)      );
    printf("addr: %d\n",  ntohs(addit[i].addition->addr)             );
    }
	
}
 //print authoritative RR

void PrintAuth(struct DNS_HEADER * dns, struct Auth * auth)
{
    printf("\nAuthoritive Records : %d \n" , ntohs(dns->auth_count) );
    for(int i=0 ; i < ntohs(dns->auth_count) ; i++)
    {
         
    printf("name: %d\n",  ntohs(auth[i].author->name)         	     );
    printf("type: %d\n",  ntohs(auth[i].author->type)         	     );
    printf("class: %d\n",  ntohs(auth[i].author->_class)       	     );
    printf("ttl: %d\n",  ntohs(auth[i].author->ttl)                  );
    printf("data_len: %d\n",  ntohs(auth[i].author->data_len)        );
    printf("name_server: %d\n",  ntohs(auth[i].author->name_server)  );

    }
    printf("\n");
	
}

//print answer section

void PrintAnswer(struct DNS_HEADER * dns, struct Answer * answers)
{
    printf("\nAnswer Records : %d \n" , ntohs(dns->ans_count) );
    for(int i=0 ; i < ntohs(dns->ans_count) ; i++)
    {
        
    printf( "Name:%d\n",     		 ntohs(answers[i].resource->name)          );
    printf( "Type:%d\n", 		 ntohs(answers[i].resource->type_cert)     );
    printf( "Class:%d\n", 		 ntohs(answers[i].resource->_class)        );
    printf( "Time to live:%d\n", 	 ntohs(answers[i].resource->ttl)           );
    printf( "Data length:%d\n",	 ntohs(answers[i].resource->data_len)      );
    printf( "Type:%d\n",		 ntohs(answers[i].resource->type_spki)     );
    printf( "Key footprint:%d\n",	 ntohs(answers[i].resource-> key_footprint));
    printf( "Algorithm:%d\n",		 ntohs(answers[i].resource->algorithm )    );
    printf("Public Key\n\n");
    for(int j=0 ; j <1166 ; j ++)
	{
	    printf("%02x " , (answers[i].key[j]));
	    if(j%8 == 1)
		printf("\n");
	}
     printf("\n");
    }
	
}

// change the hostname to dns format

void AssignToQname(unsigned char* dns,unsigned char* host) 
{   
   
    int lock = 0 , i;
    strcat((char*)host,".");
     
    for(i = 0 ; i < strlen((char*)host) ; i++) 
    {
        if(host[i]=='.') 
        {
            *dns++ = i-lock;
            for(;lock<i;lock++) 
            {
                *dns++=host[lock];
            }
            lock++; //or lock=i+1;
        }
    }
    *dns++='\0';
}



