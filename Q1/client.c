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

int main(int argc, char const *argv[]){

    /*CREATE A TCP SOCKET*/
    int sock1 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock1 < 0) {
        printf ("Error in opening a socket"); 
        exit (0);
    } printf ("Client Socket1 Created\n");

    int sock0 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock0 < 0) {
        printf ("Error in opening a socket"); 
        exit (0);
    } printf ("Client Socket2 Created\n");


    struct timeval timeout ;
    timeout.tv_sec = TIMEOUT ;
    timeout.tv_usec = 0 ;

    if( setsockopt(sock1, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0 ) { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }
    if( setsockopt(sock0, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0 ) { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }


    /*CONSTRUCT LOCAL ADDRESS STRUCTURE*/
    struct sockaddr_in serverAddr;
    memset (&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORTNO);                
    serverAddr.sin_addr.s_addr = inet_addr(ipaddr);  
   // printf ("Address assigned\n");


    /*ESTB. CONNECTION WITH SERVER*/
    int c1 = connect (sock0, (struct sockaddr*) &serverAddr , sizeof (serverAddr));  
    if (c1 < 0){ 
        printf ("Error while establishing connection");
        exit (0);
    }//printf ("Connection Established of 1\n");

    int c2 = connect (sock1, (struct sockaddr*) &serverAddr , sizeof (serverAddr));   
    if (c2 < 0){ 
        printf ("Error while establishing connection");
        exit (0);
    }//printf ("Connection Established of 0\n");
 
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
    long long int totLength = fileSize ;
    int len= 0; 
    int nread =0 ; 
    int bytesSent ; 
    int bytesRec ; 
    int lastt ;
    unsigned char ClientSendBuf0[sizeof(packet)];
    unsigned char ClientSendBuf1[sizeof(packet)];
    int sendChan ; 

    packet p , q ;

    fd_set read_fds;
    int fdmax ;
    int temp2 ;

    time_t t1, t2, now;
    int waiting= 0 ;
    struct record* rec = (struct record* ) malloc (noOfPackets* sizeof(packet));

    // printf("encapsulated packet size is %d\n",(int) sizeof(packet));
    int i = 0  ;

    sendChan = 0 ;
    while(1){

        if( i< noOfPackets){
            
            if(totLength > PACKET_SIZE){
                len = PACKET_SIZE ;
                totLength -= len ;
                lastt = 0 ;    
            }
            
            else{
                len = totLength ;
                totLength -= len ; 
                lastt = 1 ;
                printf("sending last packet\n");
            }   

            switch(sendChan){
                case(0):{
                    makePacket(&p,(int)len, i, lastt, 0,0) ;
                    nread = fread(p.buff,1,len,fp);
                    memmove(ClientSendBuf0, &p, sizeof(p)); 

                    rec[i].acked=0;
                    rec[i].t = time(NULL);
                    rec[i].pkt = p ;
                    // time(&t1);

                    bytesSent = send (sock0, ClientSendBuf0, sizeof(ClientSendBuf0), 0);   
                    if (bytesSent !=  sizeof(ClientSendBuf0)){ 
                        printf("Error while sending the message");
                        exit(0);
                    }
                    else{
                        printf ("SENT PKT:\t"); 
                        printf("Seq No: %d of size: %d bytes from channel %d at %ld sec\n", p.seqNo, bytesSent,p.channelid,rec[i].t);
                    }  
                    sendChan =1 ;
                break;
                }
                case(1):{
                        makePacket(&p,(int)len, i, lastt, 0, 1) ;
                        nread = fread(p.buff,1,len,fp);
                        memmove(ClientSendBuf1, &p, sizeof(p)); 

                        rec[i].acked=0;
                        rec[i].t = time(NULL);
                        rec[i].pkt = p ;
                    
                        bytesSent = send (sock1, ClientSendBuf1, sizeof(ClientSendBuf1), 0);   
                        if (bytesSent !=  sizeof(ClientSendBuf1)){ 
                            printf("Error while sending the message");
                            exit(0);
                        }
                        else{
                            printf ("SENT PKT:\t"); 
                            printf("Seq No: %d of size: %d bytes from channel %d at %ld sec\n", p.seqNo,bytesSent,p.channelid,rec[i].t);
                        }  
                        if(i==1){
                            sleep(2);
                        }
                        sendChan = 0;
                break ;
                }
            }
            
            if (i==noOfPackets-1)
                sleep(TIMEOUT+1);
            i++ ;
        }//end of for Pcket loop

        FD_ZERO(&read_fds); 
        FD_SET(sock1, &read_fds); 
        FD_SET(sock0, &read_fds); 
        (sock0>sock1)? (fdmax= sock0) : (fdmax= sock1) ; 

        if(select(fdmax+1, &read_fds, NULL, NULL, &timeout) == -1){
            perror("Server not connected!");
            exit(1);
        }
        if( FD_ISSET(sock0, &read_fds) ){
            //printf("enetered sock0 fdisset\n");
            bytesRec = recv(sock0, &q, sizeof(q), 0) ;
            if ( bytesRec>0 && bytesRec <= sizeof(q) ){
                
                printf("RCVD ACK: Seq No: %d of %d bytes from channel %d\n", q.seqNo, bytesRec, q.channelid );

                rec[q.seqNo].acked = 1; //update acknowledgement in record
                if(q.seqNo == waiting){
                    waiting++ ;
                    while(rec[waiting].acked == 1)
                        waiting++ ;
                }
                if(waiting< q.seqNo){
                    time(&now);
                    //printf("time difference is %f\n",difftime(now, rec[waiting].t));
                    if(difftime(now, rec[waiting].t)>= (double)TIMEOUT || q.last == 1 ||  q.seqNo ==noOfPackets-2 ){
                        handletimeout(rec[waiting].pkt, sock0, sock1);   
                        rec[waiting].t = time(NULL);                       
                    }                   
                }

                if(waiting == noOfPackets){
                    printf("clinet closing slocket\n");
                    close(sock1) ;
                    close(sock0);
                    fclose(fp);
                    exit(0);
                    
                }
            }
            sendChan = 0;
        }
        else if( FD_ISSET(sock1, &read_fds) ){
            //printf("enetered sock1 fdisset\n");
            bytesRec = recv(sock1, &q, sizeof(q), 0) ;
            if(bytesRec>0 && bytesRec <= sizeof(q)){

                printf("RCVD ACK: Seq No: %d of %d bytes from channel %d\n", q.seqNo, bytesRec, q.channelid );
                rec[q.seqNo].acked = 1; //update acknowledgement in record
                if(q.seqNo == waiting){
                    waiting++ ;
                    while(rec[waiting].acked == 1)
                        waiting++ ;

                }
                if(waiting < q.seqNo){
                    time(&now);
                    //printf("time difference is %f\n",difftime(now, rec[waiting].t));
                    if(difftime(now, rec[waiting].t)>= (double)TIMEOUT || q.last == 1 ||  q.seqNo ==noOfPackets-2 ){
                        handletimeout(rec[waiting].pkt, sock0, sock1);   
                        rec[waiting].t = time(NULL);                       
                    }                   
                }

                if(waiting == noOfPackets){
                    printf("clinet closing slocket\n");
                    close(sock1) ;
                    close(sock0);
                     fclose(fp);
                    exit(0);
                }
                
            }
            sendChan = 1;
            
        } 

    }

}//end of main fxn 


void handletimeout(packet wait, int sock0, int sock1){
    
    //printf("handling timeout\n");
    int path = wait.channelid ;
    unsigned char SendBuf[sizeof(packet)];
    memmove(SendBuf, &wait, sizeof(packet));    

    if(path==0){
        int bytesSent = send(sock0, SendBuf, sizeof(SendBuf), 0);   
            if (bytesSent !=  sizeof(SendBuf)){ 
                printf("Error while sending the message");
                exit(0);
            }
            else{
                printf ("RESENT LOST PKT..."); 
                printf("Seq No: %d from channel %d\n", wait.seqNo,wait.channelid);
            }  
    }
    else{
            int bytesSent = send(sock1, SendBuf, sizeof(SendBuf), 0);   
            if (bytesSent !=  sizeof(SendBuf)){ 
                printf("Error while sending the message");
                exit(0);
            }
            else{
                printf ("RESENT LOST PKT..."); 
                printf("Seq No: %d from channel %d\n", wait.seqNo,wait.channelid);
            }  
    }
}
