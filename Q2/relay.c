#include "packet.h"

int main(int argc, char const *argv[]){
	int sockfd;
    char t[20] ; int rno ; 

    packet p , q ;
    srand(time(NULL)); 

    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    struct timeval timeout1 ;
    timeout1.tv_sec = 1 ;
    timeout1.tv_usec = 0 ;
		    
    if( setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout1, sizeof(timeout1)) == 0 ) { 
       
    }
    
    
    struct sockaddr_in servaddr, cliaddr1, cliaddr0; 
    struct sockaddr_in rhs0 ; // server side to forward packets
    // relay's address   
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr0, 0, sizeof(cliaddr0)); 
    memset(&cliaddr1, 0, sizeof(cliaddr1));
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr =  inet_addr(ipaddr); 
    servaddr.sin_port = htons(PORTNO);
    unsigned int clen1 = sizeof(cliaddr1) ;
    unsigned int clen0 = sizeof(cliaddr0) ;

    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  sizeof(servaddr)) < 0 ) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    //rhs0 server's address 
    memset(&rhs0, 0, sizeof(rhs0)); 
    rhs0.sin_family = AF_INET;
    rhs0.sin_port = htons(PORT_SERVER);                
    rhs0.sin_addr.s_addr = htonl(INADDR_ANY); 
    unsigned int rlen0 = sizeof(rhs0) ;

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


    struct timeval timeout2 ;
    timeout2.tv_sec = 1 ;
    timeout2.tv_usec = 0 ;
            
    if( setsockopt(sock0, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout2, sizeof(timeout2)) == 0 ) { 
    
    }
    timeout2.tv_sec = 1;
    timeout2.tv_usec = 0 ;
     
    if( setsockopt(sock1, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout2, sizeof(timeout2)) == 0 ) { 
    }
    

    fd_set read_fds;
    int fdmax ;
    int activity ;
    struct timeval timeout3 ;

    while(1) {
        
        // DATA 
        memset(&p, '\0', sizeof(p) ) ;      
        if (recvfrom(sockfd, &p, sizeof(p) , 0, (struct sockaddr *) &cliaddr0, &clen0) == sizeof(p) ) {
           memset(t, 0, sizeof(t));
           fetchTime(t) ;
           printf("RELAY0  R  %s  DATA  %d  CLIENT  RELAY0\n",t, p.seqNo);
           fflush(stdout);
           rno = (rand()% 2001 ) ;
           usleep(rno) ;
           rno = (rand() % 100 ); 
            if(rno >= PDR*100 ){
            // if( p.seqNo !=  3 && p.seqNo !=  7 && p.seqNo !=  10 ){
               if (sendto(sock0, &p, sizeof(p) , 0 , (struct sockaddr *) &rhs0, rlen0)==sizeof(p) ){
                    memset(t, 0, sizeof(t));
                    fetchTime(t) ;
                	printf("RELAY0  S  %s  DATA  %d  RELAY0  SERVER\n",t, p.seqNo);
            	}
            }
        }

        if (recvfrom(sockfd, &p, sizeof(p) , 0, (struct sockaddr *) &cliaddr1, &clen1) == sizeof(p) ) {
           memset(t, 0, sizeof(t));
           fetchTime(t) ;
           printf("RELAY1  R  %s  DATA  %d  CLIENT  RELAY1\n",t, p.seqNo);
           fflush(stdout);
           rno = (rand()% 2001 ) ;
           usleep(rno) ;

           rno = (rand() % 100 ); 
           if(rno >= PDR*100 || 1){
                if(sendto(sock1, &p, sizeof(p) , 0 , (struct sockaddr *) &rhs0, rlen0)==sizeof(p) ){
                    memset(t, 0, sizeof(t));
                    fetchTime(t) ;
                    printf("RELAY1  S  %s  DATA  %d  RELAY1  SERVER\n",t, p.seqNo);
                }
            }

        }

        //waiting for ack

        FD_ZERO(&read_fds); 
        FD_SET(sock1, &read_fds); 
        FD_SET(sock0, &read_fds);
        fdmax = (sock0>sock1)? sock0 : sock1;       
        timeout3.tv_sec = 1 ;
        timeout3.tv_usec = 0 ; 

        if(select(fdmax+1, &read_fds, NULL, NULL, &timeout3) >0 ){

            FD_ZERO(&read_fds); 
            FD_SET(sock1, &read_fds); 
            FD_SET(sock0, &read_fds);
            fdmax = (sock0>sock1)? sock0 : sock1;       
            timeout3.tv_sec = 1 ;
            timeout3.tv_usec = 0 ; 

            memset(&q, '\0', sizeof(q) ) ;
            
            if( FD_ISSET(sock0, &read_fds) ){
                if(recvfrom(sock0, &q, sizeof(q), 0, (struct sockaddr *)&rhs0, &rlen0 ) == sizeof(q) ){
                   // printf("received data from serv: %d :-- %s--\n", q.seqNo , q.buff);
                    memset(t, 0, sizeof(t));
                    fetchTime(t) ;
                    printf("RELAY0  R  %s  ACK  %d  SERVER  RELAY0\n",t, q.seqNo);

                    if (sendto(sockfd, &q, sizeof(q), 0, (struct sockaddr *)  &cliaddr0, clen0)== sizeof(q)){
                        //perror("sendto() from 1st");
                        memset(t, 0, sizeof(t));
                        fetchTime(t) ;
                        printf("RELAY0  S  %s  ACK  %d  RELAY0  CLIENT\n",t, q.seqNo);
                    }
                }
            }
           if(FD_ISSET(sock1, &read_fds)){
                if(recvfrom(sock1, &q, sizeof(q),0, (struct sockaddr *)&rhs0, &rlen0 ) == sizeof(q)){
                    //printf("received data from serv: %d :-- %s--\n", q.seqNo , q.buff);
                    memset(t, 0, sizeof(t));
                    fetchTime(t) ;
                    printf("RELAY1  R  %s  ACK  %d  SERVER  RELAY1\n",t, q.seqNo);
                    if (sendto(sockfd, &q, sizeof(q), 0, (struct sockaddr *)  &cliaddr1, clen1)== sizeof(q)){
                        memset(t, 0, sizeof(t));
                        fetchTime(t) ;
                        printf("RELAY1  S  %s  ACK  %d  RELAY1  CLIENT\n",t, q.seqNo);
                    }
                }
            }
        }
        else{
            // printf("waiting for acks from server\n");
        }
   
    } 
	return 0;
     
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
    sprintf(msec, ":%06ld",(long int) tval.tv_usec);
    strcat(str,msec);
}
