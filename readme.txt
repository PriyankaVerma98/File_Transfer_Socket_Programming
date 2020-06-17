Name : Priyanka Verma
ID : 2016B3A70492P

COMPUTER NETWORKS ASSIGNMENT 1

## Problem 1 :File transfer using multi-channel stop-and-wait protocol

Make two separate terminals and follow procedure described as follows:-
1) Run server file by compiling server.c as "gcc -o s server.c" 
2) Run client file by compiling client.c as "gcc -o c client.c" and then run "./c".
3) Ensure Input file name should be "input.txt". 
4) First execute server using "./s" and then run client using "./c" in the 2 different terminals.



### Methodology

1.. Packet Structure
Input file is read PACKET_SIZE bytes at a time and the data is encapsulated in the form of the struct ‘packet’. The packets are encoded in the form of string for sending to the server. Before sending, the client buffers the :-
   - timestamp when a packet is sent to server
   - the packet 
   - a flag of whether the corresponding acknowledgement packet is received or not.

2.. Transmission of packets by client :-
The 0th packet is sent via channel #0 and 1st packet is send via channel #1. All other packets are transmitted randomly via the 2 client channels controlled in switch case construct. A channel (say channel 0) sends the data when the ‘sendChan’ flag corresponds to the same value (‘sendChan’ = 0) and after sending changes ‘sendChan’ flag (‘sendChan’= 1) for the other channel to send data. Similarly, channel 1 sends data when ‘sendChan’= 1 and after sending changes the flag to ‘sendChan’= 0. However, when the acknowledgement comes from a particular channel (say Channel 1) ‘sendChan’ flag is set for the same channel (sendChan =1) to send the next packet. Hence, the two channels compete for sending the next packet. Therefore, the packets are send randomly via either of the two channels.

3.. Handling of timeout by client :-
A global variable ‘waiting’ is maintained to keep track of the expected sequence number of acknowledgement packet from server. When the expected sequence number of ack packet is greater than ‘waiting’, the difference in time ‘now’ and sending timestamp is measured. If it exceeds the ‘TIMEOUT’ value then the data packet is re-sent. 

4.. Server Side :-
The server randomly drops 10% (set with macro PDR) of the incoming packets. The client does not receive acknowledgements for the dropped packets and resends those packets to server after timeout happens. 
When the server receives out of order packets, it buffers the packet data and sequence no in a queue of structs named ‘sBuff’. The ‘struct head’ keeps information about queue with pointers to first and last elements.
When the maximum capacity of the server buffer is reached (defined as macro sBuffCapacity) all the incoming packets are dropped. The methods ‘deletee’ and ‘insert’ are used to manipulate the buffer queue in FIFO order.
The server writes the data either immediately to an output file when correct seq. no packet is received or later from the buffer to the file.

5.. Assumptions :-
 - PACKET_SIZE refers to the payload data encapsulated within a packet.
 - The timeout is handled for individual packets through the difference of timestamps at sending and time till ack is not received

6.. Cases Handled :
 - File size is less than packet size
 - File size is not a multiple of packet size
 - Dropping packets when buffer is full

7.. Note
 - Refer to macros defined in 'packet.h' file


## Problem 2 : File transfer using Selective Repeat protocol over UDP

### How to run the program

1. Open 3 separate terminals (for client, server and relay) and compile the 3 files as :
	- gcc -o c client.c
        - gcc -o s server.c
        - gcc -o r relay.c
2. Check input file is named as "input.txt"
3. First run the server using "./s"
4. Run the relay as "./r". (Note only single instance is made for relay)
5. Run the client using "./c"

Note :  Refer to "packet.h" to changing macros and testing 

### Methodology


- Input file is read PACKET_SIZE bytes at a time and is encapsulated in the form of the struct ‘packet’. The packets are encoded in the form of string for sending to relay0 or relay1, which then forward them to the server. 
- The client makes and sends ‘sendWindow’ no of packets at a time and waits to receive all the acks. Packets within the sender window for which acks are not received within ‘TIMEOUT’ seconds are re-sent. 
- The window is moved forward by ‘sendWindow’ size to implement a common timer for all the packets within the window.

1.. Transmission of packets 
Even no. packets are sent via channel #0 and odd no. packets via channel #1. However, the acknowledgement packets are received from any channel.
The relay0 and relay1 each randomly drop 10% (set with macro PDR) of the incoming packets. Each of them also adds a delay of a few milliseconds.

2.. Handling of timeout by client
A global variable ‘waiting’ is maintained to keep track of the expected sequence number of acknowledgement packet. The ‘select’ method, using ‘fd_set’, keeps a check of time since when no acks is received. If it exceeds the ‘TIMEOUT’ value then the unacknowledged packets are resend from the client buffer.

3.. Server Side
When the server receives out of order packets, it buffers the packet data and sequence no in a queue of structs named ‘sBuff’. When the maximum capacity of the server buffer is reached (defined as macro sBuffCapacity) all the future incoming packets would be dropped. The methods ‘deletee’ and ‘insert’ are used to manipulate the buffer queue in FIFO order.
The server writes the data either immediately to an output file when correct seq. no packet is received or later from the buffer to the file.

Note 1 : When Server buffer is full, all other incoming packets will be dropped till the required packet no. is received. However, even in such a case the out of order packets received by server are displayed on terminal but their ack won't be sent.

Note 2 : the relay program is supposed to be ended by User using control+ c.
Cases Handled :
1. Packets are dropped when buffer capacity is full.
2. File size is not a multiple of packet size 

