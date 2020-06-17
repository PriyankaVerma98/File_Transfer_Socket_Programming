#include "packet.h"


int main(int argc, char const *argv[]){


	int sockfd;
    char t[20] ; int rno ; 
    packet p , q ; int rvd =0 , rvd2= 0;
    unsigned char SendBuf0[sizeof(packet)];
    int waiting=0;
    char temp[PACKET_SIZE+1] ;
    char bla[PACKET_SIZE];

    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    struct timeval timeout ;
    timeout.tv_sec = 1 ;
    timeout.tv_usec = 0 ;
            
    if( setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) == 0 ) {  
    }

    struct sockaddr_in rhs0 ;
    memset(&rhs0, 0, sizeof(rhs0));
    rhs0.sin_family = AF_INET;
    rhs0.sin_port = htons(PORT_SERVER);                
    rhs0.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr(ipaddrServer);  
    unsigned int rlen0 = sizeof(rhs0) ;

    struct sockaddr_in cl0, cl1 ;
    memset(&cl0, 0, sizeof(cl0));
    memset(&cl0, 0, sizeof(cl1));
    unsigned int lencl0 = sizeof(cl0) ;
    unsigned int lencl1 = sizeof(cl1) ;

    if ( bind(sockfd, (const struct sockaddr *)&rhs0,  sizeof(rhs0)) < 0 ) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    FILE *fp;     
    fp = fopen("output.txt", "wb"); 
    if(NULL == fp){
        printf("Error opening output file");
        return 1;
    }
    sBuff *ptr ;
    struct head h ;
    h.Count = 0;
    h.first = NULL ;
    h.last = NULL ;
    int counter =0 ;

    while(1) {
        
        memset(&p, '\0', sizeof(p) ) ;
    	if ( recvfrom(sockfd, &p, sizeof(p) , 0, (struct sockaddr *) &cl0, &lencl0) == sizeof(p) ){
    		fetchTime(t);
    		printf("SERVER  R  %s  DATA  %d RELAY0  SERVER: \n",t, p.seqNo);
            if(waiting == p.seqNo){ //correct packet recvd is writtrn to file              
                fprintf(fp, "%s",  p.buff);
                fflush(fp);

                makePacket( &q , p.size, p.seqNo, p.last, 1, 0) ;
                memmove(SendBuf0, &q, sizeof(q));               
                if(sendto(sockfd, SendBuf0, sizeof(SendBuf0) , 0 , (struct sockaddr *) &cl0, lencl0) >0 ){
                    fetchTime(t);
                    printf("SERVER  S  %s  ACK  %d  SERVER  RELAY0\n",t, q.seqNo);
                } 

                waiting++;                                                       
                ptr = h.first ;
                while( h.Count !=0 && ptr->no == waiting){
                    strncpy(bla, ptr->charbuff, PACKET_SIZE);
                    fprintf(fp, "%s",bla);
                    fflush(fp);
                    ptr = ptr->next ;
                    deletee(&h); 
                    counter-- ; 
                    waiting++;                    
                }      
            }
            else{ //buffer the data 
                if(counter < sBuffCapacity) {
                    // printf("q\n");
                    sBuff *new = (sBuff *)malloc(sizeof(sBuff)) ;
                    new->no = p.seqNo;
                    strcpy(new->charbuff, p.buff ) ;
                    new->next = NULL ;
                    new->last= p.last;
                    insert(&h, new);
                    makePacket( &q , p.size, p.seqNo, p.last, 1, 0) ;
                    memmove(SendBuf0, &q, sizeof(q));               
                    if(sendto(sockfd, SendBuf0, sizeof(SendBuf0) , 0 , (struct sockaddr *) &cl0, lencl0) >0 ){
                        fetchTime(t);
                        printf("SERVER  S  %s  ACK  %d  SERVER  RELAY0\n",t, q.seqNo);
                    } 
                    counter++ ; 
                    if(counter== sBuffCapacity)
                        printf("server buffer full! more packets will be dropped\n");
                }
            }
    		// if(counter < sBuffCapacity) { // silly mistake
          //           makePacket( &q , p.size, p.seqNo, p.last, 1, 0) ;
          //           memmove(SendBuf0, &q, sizeof(q));               
          //           if(sendto(sockfd, SendBuf0, sizeof(SendBuf0) , 0 , (struct sockaddr *) &cl0, lencl0) >0 ){
          //               fetchTime(t);
          //               printf("SERVER  S  %s  ACK  %d  SERVER  RELAY0\n",t, q.seqNo);
          //           }
          //       }       
    	}
        memset(&p, '\0', sizeof(p) ) ;
    	if( recvfrom(sockfd, &p, sizeof(p) , 0, (struct sockaddr *) &cl1, &lencl1) == sizeof(p) ) {
    		fetchTime(t);
    		printf("SERVER  R  %s  DATA  %d  RELAY1  SERVER\n",t, p.seqNo);
            //write to file
            if(waiting == p.seqNo){ //correct packet recvd is writtrn to file
        
                fprintf(fp, "%s",  p.buff);
                fflush(fp) ;
                makePacket( &q , p.size, p.seqNo, p.last, 1, 1) ;
                memmove(SendBuf0, &q, sizeof(q));              
                if(sendto(sockfd, SendBuf0, sizeof(SendBuf0) , 0 , (struct sockaddr *) &cl1, lencl1) >0 ){
                    fetchTime(t);
                    printf("SERVER  S  %s  ACK  %d  SERVER  RELAY1\n",t, q.seqNo);
                }

                waiting++;                                                           
                ptr = h.first ;
                while( h.Count !=0 && ptr->no == waiting){
                    
                    strncpy(bla, ptr->charbuff, PACKET_SIZE);
                    fprintf(fp, "%s",bla);
                    fflush(fp);
                    ptr = ptr->next ;
                    deletee(&h); 
                    counter-- ; 
                    waiting++;                    
                }               
            }

            else{ //buffer the data                
                if(counter < sBuffCapacity) {
                    // printf("1\n");
                    sBuff *new = (sBuff *)malloc(sizeof(sBuff)) ;
                    new->no = p.seqNo;
                    // printf("2\n");
                    strcpy(new->charbuff, p.buff ) ;
                    new->next = NULL ;
                    new->last = p.last ;
                    // printf("3\n");
                    insert(&h, new);
                    // printf("4\n");

                    makePacket( &q , p.size, p.seqNo, p.last, 1, 1) ;
                    memmove(SendBuf0, &q, sizeof(q));              
                    if(sendto(sockfd, SendBuf0, sizeof(SendBuf0) , 0 , (struct sockaddr *) &cl1, lencl1) >0 ){
                        fetchTime(t);
                        printf("SERVER  S  %s  ACK  %d  SERVER  RELAY1\n",t, q.seqNo);
                    } 
                    counter++ ; 
                    if(counter== sBuffCapacity)
                        printf("server buffer full! packets will be dropped now\n");
                }
            }    
            // makePacket( &q , p.size, p.seqNo, p.last, 1, 1) ;
            // memmove(SendBuf0, &q, sizeof(q));              
            // if(sendto(sockfd, SendBuf0, sizeof(SendBuf0) , 0 , (struct sockaddr *) &cl1, lencl1) >0 ){
            //     fetchTime(t);
            //     printf("SERVER  S  %s  ACK  %d  SERVER  RELAY1\n",t, q.seqNo);
            // }       
           
    	}
        else{
            continue ;
        }

    }// end of while
	
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
    sprintf(msec, ":%06ld", (long int)tval.tv_usec);
    strcat(str,msec);
}

void  makePacket( packet *p , int size, int seq, int last, int ack, int id){
    p->size = size ;
    p->seqNo = seq ;
    p->last = last;
    p->ack = ack ;
    p->channelid = id ;
    memset(p->buff, '\0',PACKET_SIZE+1 ) ;
}

void deletee(struct head * node ){
    sBuff * temp = node->first; 
    node->first = temp->next  ;
    if(node->Count == 1){
        node->last = NULL ;
    }
    free(temp) ;
    node->Count-- ;
}

void insert(struct head * node, sBuff *new){
    
    
    // printf("here insert /n");

    node->Count++ ;
    if(node->Count== 1){    
        node->first = new ;
        node->last = new;
        return ; 
    }
    sBuff *st = node->first ;
    sBuff *temp = node->first->next ;
    while(st != NULL){
        if(temp == NULL){
            st->next = new ;
            node->last = new ;
            return ;
        }
        else if ((st->no < new->no) && (temp->no > new->no) ){
            st->next = new ;
            new ->next = temp ;
            return  ;
        }
        else {
            st = st->next ;
            temp = temp->next ;
        }
    }
    
}

