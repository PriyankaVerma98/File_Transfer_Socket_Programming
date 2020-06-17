#include "packet.h"

int main(int argc, char const *argv[]){
	
    int len= PACKET_SIZE; 
    int nread =0 ; 
    int bytesSent ; 
    int bytesRec ; 
    int lastt= 0 ;
    unsigned char ClientSendBuf0[sizeof(packet)];
    int sendChan ; 

    packet p , q ;

    fd_set read_fds;
    int fdmax ;
    int temp2 ;
    int flag = 1 ;
    char t[20];
    int activity; 

    time_t now;
    int waiting= 0 ;
    struct record* rec = (struct record* ) malloc (sendWindow* sizeof(struct record ));

	/*CREATE A UDP SOCKET*/
    int sock0 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock0 < 0) {
        printf ("Error in opening a socket"); 
        exit (0);
    } printf ("Client Socket0 Created\n");

    int sock1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock1 < 0) {
        printf ("Error in opening a socket"); 
        exit (0);
    } printf ("Client Socket1 Created\n");

    struct timeval timeout22 ;
    timeout22.tv_usec = 0 ;
    timeout22.tv_sec = 0;

    // set sockets receive as non-blocking and to return after timeout22 seconds
    if( setsockopt(sock1, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout22, sizeof(timeout22)) < 0 ) { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }
    if( setsockopt(sock0, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout22, sizeof(timeout22)) < 0 ) { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }

    struct sockaddr_in serverAddr; 
    memset (&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORTNO);                
    serverAddr.sin_addr.s_addr = inet_addr(ipaddr);  
    unsigned int slen = sizeof(serverAddr) ;
    printf ("Address assigned\n");

    FILE *fp = fopen("input.txt","r");
    if(fp==NULL){
        printf("File open error");
        return 1;   
    }
    //find file size    
    struct stat st; 
    long long int fileSize ;   
    if(stat("input.txt",&st)==0)
        fileSize = st.st_size ;
    else
        fileSize = -1 ;

    long long int noOfPackets =  (long long int) ceil( (double)fileSize / (double) PACKET_SIZE );

    fseek(fp, 0, SEEK_SET);

    for(int j=0 ; j< noOfPackets ; j = j+sendWindow){
        
        flag = 1;
        for(int i = 0 ; (i< sendWindow) && (i+j < noOfPackets) ; i++){
    		if(i+j == noOfPackets-1)
    			lastt= 1;

    		if( (i+j)%2 == 0){
    			makePacket(&p , PACKET_SIZE, i+j, lastt,0, 0) ;
	    		nread = fread(p.buff,1,len,fp);
	    		memmove(ClientSendBuf0, &p, sizeof(p)); 

	    		rec[i].acked=0;
	    		char vela[20];
	            fetchTime( vela ) ;
	            strcpy(rec[i].time , vela);
	            rec[i].pkt = p ;

	            if (sendto(sock0, ClientSendBuf0, sizeof(ClientSendBuf0) , 0 , (struct sockaddr *) &serverAddr, slen) == sizeof(ClientSendBuf0)){
            	   fetchTime(t) ;
                   printf("CLIENT  S  %s  DATA  %d  CLIENT  RELAY0\n",t, p.seqNo);
                   fflush(stdout);
	        	}
    		}
    		else{
    			makePacket(&p , PACKET_SIZE, i+j, lastt,0, 1) ;
	    		nread = fread(p.buff,1,len,fp);
	    		memmove(ClientSendBuf0, &p, sizeof(p)); 

	    		rec[i].acked=0;
	    		char vela[20];
	            fetchTime( vela ) ;
	            strcpy(rec[i].time , vela);
	            rec[i].pkt = p ;

	            if (sendto(sock1, ClientSendBuf0, sizeof(ClientSendBuf0) , 0 , (struct sockaddr *) &serverAddr, slen) == sizeof(ClientSendBuf0)){
                   fetchTime(t) ;
                   printf("CLIENT  S  %s  DATA  %d  CLIENT  RELAY1\n",t, p.seqNo);
                   fflush(stdout);
                }
    		}
		
        }
	
        struct timeval timeout ;
	    while(flag == 1){
            
            timeout.tv_sec = TIMEOUT ;
            timeout.tv_usec = 0 ;
            FD_ZERO(&read_fds); 
            FD_SET(sock1, &read_fds); 
            FD_SET(sock0, &read_fds); 
            fdmax = (sock0>sock1)? sock0 : sock1; 

            activity = select(fdmax+1, &read_fds, NULL, NULL, &timeout) ; //monitors sock1, sock0 for receiving data 
            if( activity > 0){

                    if( FD_ISSET(sock0, &read_fds)) {                   
                        bytesRec = recvfrom(sock0, &q, sizeof(q), 0, (struct sockaddr *)&serverAddr, &slen ) ;
                        if ( bytesRec>0 && bytesRec == sizeof(q) ){   
                            fetchTime(t);
                            printf("CLIENT R  %s  ACK  %d  RELAY0  CLIENT\n", t,  q.seqNo);
                            rec[q.seqNo-j].acked = 1; //update acknowledgement in record
                        }
                    }

                    if(FD_ISSET(sock1, &read_fds)){
                        bytesRec = recvfrom(sock1, &q, sizeof(q), 0, (struct sockaddr *)&serverAddr, &slen ) ;
                        if ( bytesRec>0 && bytesRec == sizeof(q) ){               
                            fetchTime(t);
                            printf("CLIENT R  %s  ACK  %d  RELAY1  CLIENT\n", t,  q.seqNo);
                            rec[q.seqNo-j].acked = 1; //update acknowledgement in record
                        }
                    }

                    for(int k = 0 ; k<sendWindow ; k++){
                        if(rec[k].acked ==0)
                            break ;
                        if(k==sendWindow-1)
                            flag = 0 ;
                    }

            } 
            if(activity == 0){
                for(int k = 0 ; k< sendWindow ; ){
                    if(rec[k].acked == 0){
                        unsigned char SendBuf[sizeof(packet)];
                        memmove(SendBuf, &rec[k].pkt, sizeof(packet)); 
                        if( (j+k )%2 == 0){
                            if (sendto(sock0, SendBuf, sizeof(SendBuf) , 0 , (struct sockaddr *) &serverAddr, slen) == sizeof(SendBuf)){
                                fetchTime(t) ;
                                printf("CLIENT  RE  %s  DATA  %d  CLIENT  RELAY0\n",t, rec[k].pkt.seqNo);
                                fflush(stdout);
                            }
                        }
                        else{
                            if (sendto(sock1, SendBuf, sizeof(SendBuf) , 0 , (struct sockaddr *) &serverAddr, slen) == sizeof(SendBuf)){
                                fetchTime(t) ;
                                printf("CLIENT  RE  %s  DATA  %d  CLIENT  RELAY1\n",t, rec[k].pkt.seqNo);
                                fflush(stdout);
                            }
                        }
                    }
                    k++ ;
                } 
            }        
        }

    } // end of for loop

	return 0;
}

void  makePacket( packet *p , int size, int seq, int last, int ack, int id){
    p->size = size ;
    p->seqNo = seq ;
    p->ack = 0 ;
    p->last = last;
    p->ack = ack ;
    p->channelid = id ;
    memset(p->buff, '\0',PACKET_SIZE+1 ) ;
}

void fetchTime(char str[20]){
    int rc ;
    time_t now ;
    struct tm* ptr;
    struct timeval tval ;

    time(&now) ;
    ptr = localtime(&now) ;
    gettimeofday(&tval, NULL);

    rc = strftime(str,20,"%H:%M:%S", ptr);
    char msec[8];
    sprintf(msec, ":%06ld", (long int)tval.tv_usec);
    strcat(str,msec);
}