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

#define PACKET_SIZE 20  //refers to the file size chunk encapsulated in the packet
#define dataMax 999
#define PDR 0.10 //percentage
#define TIMEOUT 1 // in sec 
#define PORTNO 12345
#define ipaddr "127.0.0.1"
#define sBuffCapacity 20

typedef struct h{    
    int size ;
    int seqNo;
    int last ; // last ke liye 1 
    int ack ; // data=0 , ack = 1
    int channelid ; // 0 or 1 
    char buff[PACKET_SIZE+1] ;
}packet;

struct record{
    int acked;
    time_t t;
    packet pkt ;
} ;

typedef struct serverbuffer{
	int no  ; // pkt seq no-
	int last;
	char charbuff[PACKET_SIZE+1] ; //payload data 
	struct serverbuffer *next ;
}sBuff;

struct head{
	int Count ;
	sBuff * first ;
	sBuff *last ;
};


void  makePacket( packet *p , int size, int seq, int last, int ack, int id) ;
void handletimeout(packet wait, int sock0, int sock1) ;
void deletee(struct head * node ) ;
void insert(struct head * node, sBuff *new) ;

