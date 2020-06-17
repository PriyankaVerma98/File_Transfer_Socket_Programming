#include "header.h"

void  makePacket( packet *p , int size, int seq, int last, int ack, int id){
    p->size = size ;
    p->seqNo = seq ;
    p->ack = 0 ;
    p->last = last;
    p->ack = ack ;
    p->channelid = id ;
    memset(p->buff, '\0',PACKET_SIZE+1 ) ;
}
int main(){

	int sockets[3]={0} ;
	fd_set read_fds;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	unsigned int addrlen = sizeof(clientaddr) ;
	int fdmax;
	int newfd;
	int opt =1 ; int sd ; int count=1 ;

	FILE *fp;     
    fp = fopen("output.txt", "w"); 
    if(NULL == fp){
      	printf("Error opening output file");
        return 1;
    }
    

	FD_ZERO(&read_fds);

	sockets[2] = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP); 
	int status = fcntl(sockets[2], F_SETFL, fcntl(sockets[2], F_GETFL, 0) | O_NONBLOCK);
    if (status == -1){
      perror("calling fcntl");

    }

	sockets[0] = 0 ; 
	sockets[1]= 0 ;
	if (sockets[2] < 0) { 
		printf ("Error while server sockets creation"); 
		exit (0); 
	}
	else{
		printf ("Server Socket Created\n");
	}

	struct timeval timeout ;
	timeout.tv_sec = TIMEOUT ;
	timeout.tv_usec = 0 ;

	if( setsockopt(sockets[2], SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0 ) { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }

    serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(ipaddr);
	serveraddr.sin_port = htons(PORTNO);
	memset(&(serveraddr.sin_zero), '\0', 8);


	if(bind(sockets[2], (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1){
	    perror("Server-bind() error lol!");
	    exit(1);
	}
	printf("Server-bind() is OK...\n");
 
	/* listen */
	if(listen(sockets[2], 10) == -1){
	     perror("Server-listen() error lol!");
	     exit(1);
	}
	printf("Server-listen()	is OK...\n");

	FD_SET(sockets[2], &read_fds);
	fdmax = sockets[2];

	int counter=0; int flag= 0;
	int rno; int waiting=0 ;
	
	struct head h ;
	h.Count = 0;
	h.first = NULL ;
	h.last = NULL ;

	int activity ;
	sBuff *ptr ;

	while(1){

		struct timeval timeout ;
		timeout.tv_sec = TIMEOUT +2 ;
		timeout.tv_usec = 0 ;
		FD_ZERO(&read_fds); 
		FD_SET(sockets[2], &read_fds); 

		fdmax = sockets[2]; 
		for (int i = 0 ; i < 2 ; i++) { 
			sd = sockets[i]; 	 
			if(sd > 0) 
				FD_SET( sd , &read_fds); 	
			if(sd > fdmax) 
				fdmax = sd; 
			//printf("socket fd value %d\n",sd );
		} 
		activity = select(fdmax+1, &read_fds, NULL, NULL, &timeout) ;
		if(activity == -1){
	        perror("Server not connected!");
	        exit(1);
	    }

	    if(activity == 0){
	        perror("Server timeout!");
	        exit(1);
	    }
	    //printf("Server-select() is OK...\n");

	    if(FD_ISSET(sockets[2], &read_fds)){
	    	if((newfd = accept(sockets[2], (struct sockaddr *)&clientaddr, &addrlen)) == -1){
	                    perror("Server-accept() error lol!");
	         }
	         else{
	         	FD_SET(newfd, &read_fds);
	         	if(newfd > fdmax){ /* keep track of the maximum */
	                        fdmax = newfd;
                }    
                // printf("New connection from %s on sockets fd= %d\n", inet_ntoa(clientaddr.sin_addr), newfd);
				if(sockets[0]==0)
					sockets[0] = newfd; 
				else
					sockets[1] = newfd; 
				// printf("Adding to list of socketss as %d\n" , newfd); 		
				 

	        }
	    }
	    rno = (rand() % 100 ); 
	    for (int i = 0; i < 2; i++) { 
			sd = sockets[i]; 
	    	if(FD_ISSET(sd, &read_fds)){

	    				int temp2 ; char temp[PACKET_SIZE+1] ;
						int l=0 ; 
						packet r, q;
						//printf("entered into 1st else part fd= %d\n", sd);

            			if( (temp2 = recv(sd, &r, sizeof(r), 0) ) ==  sizeof(r)  ) {
            					//printf("randoom no = %d\n", rno );
            					if( (rno < PDR*100) || h.Count > sBuffCapacity){
									printf("packet blocked\n");
								}
								else /* ( r.seqNo != 1 && counter <25) */ {
				
									printf ("RCVD PKT:  "); 
									printf("Seq No: %d of size: %d bytes from channel %d\n", r.seqNo, temp2, r.channelid);
									l = r.last ;
									//printf("recieved packet size is\n: %d\n", (int)  sizeof(r) );									
									//fflush(fp);
									makePacket( &q ,  r.size, r.seqNo, l, 1, r.channelid);
										//printf("made packet of size \n: %d\n",  (int) sizeof(q) );

									int bytesSent = send (sd,&q, sizeof(q),0); 
									if (bytesSent != sizeof(q) ){ 
										printf ("Error while sending message to client"); 
										exit(0);
								    }
							  		printf("SENT ACK for PKT with Seq No: %d from channel %d of %d bytes\n", q.seqNo, q.channelid, bytesSent);

							  		if(waiting == r.seqNo){ //correct packet recvd is writtrn to file
							  			memset(temp, '\0', sizeof(temp));
										strcpy(temp, r.buff);
										fprintf(fp, "%s", temp);
										// if(r.last == 1){
										// 	fclose(fp);
									 //    	close(sockets[1]);
									 //    	close(sockets[2]);
									 //    	close(sockets[0]);
									 //    	sockets[0]= 0 ;
									 //    	sockets[1]= 0;
									 //   		sockets[2]=0;
									 //   		exit(0);
										// }
										waiting++; 										
										
										//printf("copied string to output.txt is:- %s\n", temp);						
										ptr = h.first ;
										while( h.Count !=0 && ptr->no == waiting){
											
											fprintf(fp, "%s",ptr->charbuff);
											fflush(fp);
											if(ptr->last == 1){
												fclose(fp);
										    	close(sockets[1]);
										    	close(sockets[2]);
										    	close(sockets[0]);
										    	sockets[0]= 0 ;
										    	sockets[1]= 0;
										   		sockets[2]=0;
										   		exit(0);
											
											}
											ptr = ptr->next ;
											deletee(&h); 
											waiting++;
											
										}
										
							  		}

							  		else{ //buffer it 
							  			
							  			// memset(temp, '\0', sizeof(temp));
							  			// strcpy(temp, r.buff);
							  			sBuff *new = (sBuff *)malloc(sizeof(sBuff)) ;
							  			new->no = r.seqNo;
							  			strcpy(new->charbuff, r.buff ) ;
							  			new->next = NULL ;
							  			new->last= l;
							  			insert(&h, new);
							  			//printf("buffer value is:\n----%s----\n",charbuff[counter] );
							  			
							  			if(h.Count== sBuffCapacity)
							  				printf("server buffer full! packets will be dropped now\n");
							  		}

								}
						}
	    	}
	    }   
	}
}//end od main

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
	sBuff *temp = node->last ;
	
	if(node->Count== 0){	
		node->first = new ;
		node->last = new;
	}
	else{
		temp->next = new ;
		node->last = new;
	}
	node->Count++ ;
}




