#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <sys/stat.h> //for file size 
#include <math.h> // for ceil fxn 
#include <sys/time.h> 
#include <fcntl.h>
#include <time.h>

#define PACKET_SIZE 40  //refers to the data size encapsulated in the packet
#define PDR 0.30 //percentage of packet drop rate by relays  
#define PORT_SERVER 8882
#define ipaddr "127.0.0.1"
#define PORTNO 12345
#define sendWindow 6 // client sending window
#define TIMEOUT 7 // in seconds
#define sBuffCapacity 10 //no of out of order packets that can be buffered by Server

typedef struct h{    
    int size ;
    int seqNo;
    int last ; // last ke liye 1 
    int ack ; // data=0 , ack = 1
    int channelid ; // 0 or 1 
    char buff[PACKET_SIZE+1] ;
}packet;

struct record{ // for client buffer to resend when a packet is lost
    int acked;
    char time[20];
    packet pkt ;
} ;

typedef struct serverbuffer{
	int no  ; // pkt seq no-
	int last;
	char charbuff[PACKET_SIZE+1] ; //payload data 
	struct serverbuffer *next ;
}sBuff;   // server buffers out of order packets through this struct

struct head{
	int Count ;
	sBuff * first ;
	sBuff *last ;
};  // maintains a queue of server buffer

void fetchTime(char str[20]) ;
void  makePacket( packet *p , int size, int seq, int last, int ack, int id) ;
void deletee(struct head * node ) ;
void insert(struct head * node, sBuff *new) ;
