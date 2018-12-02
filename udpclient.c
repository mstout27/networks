/* udp_client.c */ 
/* Programmed by Matt Stout & Bree McCausland */
/* Nov. 28, 2018 */

#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024


int main(void) {
		
    int sock_client;  /* Socket used by client */ 

    struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
    unsigned short client_port;  /* Port number used by client (local port) */

    struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
    struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
    char server_hostname[STRING_SIZE]; /* Server's hostname */
    unsigned short server_port;  /* Port number used by server (remote port) */

    char sentence[STRING_SIZE];  /* send message */
    char modifiedSentence[STRING_SIZE]; /* receive message */
    unsigned int msg_len;  /* length of message */
    int bytes_sent, bytes_recd; /* number of bytes sent or received */
  
    /* open a socket */

    if ((sock_client = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("Client: can't open datagram socket\n");
		exit(1);
    }

    /* Note: there is no need to initialize local client address information
            unless you want to specify a specific local port.
            The local address initialization and binding is done automatically
            when the sendto function is called later, if the socket has not
            already been bound. 
            The code below illustrates how to initialize and bind to a
            specific local port, if that is desired. */

    /* initialize client address information */

    client_port = 0;   /* This allows choice of any available local port */

    /* Uncomment the lines below if you want to specify a particular 
             local port: */
    /*
    printf("Enter port number for client: ");
    scanf("%hu", &client_port);
    */

    /* clear client address structure and initialize with client address */
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* This allows choice of
                                        any host interface, if more than one 
                                        are present */
    client_addr.sin_port = htons(client_port);

    /* bind the socket to the local client port */

    if (bind(sock_client, (struct sockaddr *) &client_addr,
                                    sizeof (client_addr)) < 0) {
      perror("Client: can't bind to local address\n");
      close(sock_client);
      exit(1);
    }

    /* end of local address initialization and binding */	

	/* initialize server address information */

	printf("Enter hostname of server: ");
    scanf("%s", server_hostname);
    if ((server_hp = gethostbyname(server_hostname)) == NULL) {
      perror("Client: invalid server hostname\n");
      close(sock_client);
      exit(1);
    }
    printf("Enter port number for server: ");
    scanf("%hu", &server_port);

	/* Clear server address structure and initialize with server address */
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
                                    server_hp->h_length);
	server_addr.sin_port = htons(server_port);
	
/* -------------------------------------------------------------------- */
   char input[10000]; 
    
   printf("Enter filename to use as input: ");
 	 scanf("%s", input);
	
  struct Packet{
		short count;
		short seqNum;
		char data[88];
	};

	FILE* fp = fopen(input, "r");
	size_t len = 88;
	char line[STRING_SIZE]; 
	ssize_t read;
	
	struct Packet sendPacket;
	struct Packet recvPacket;
	
	if(fp == NULL){
		exit(EXIT_FAILURE);
	}
	
	double timeout;
	/* user interface */
	printf("Enter timeout value between 1 and 10: ");
    scanf("%lf", &timeout);
	/* Define timeout */
 
  int microseconds = (int)timeout;
	int seconds = (int)((timeout - (double)microseconds)*1000000);
	
	/* Stop and Wait protocol logic */
	int recvACK = 1;
	int exit = 0;
	
	int packetTotal = 0;
	int countSum = 0;
	int retransmitTotal = 0;
	int ACKTotal = 0;
	int timeoutTotal = 0;
 
	int retransmit = 0;
 
   sendPacket.seqNum = 0;
	/* Wait for call from above */
	while(1){
    
		/* Make packet to transmit */
    memset(sendPacket.data,'\0', len);
    memset(recvPacket.data,'\0',len);
    
    if(!fgets(line,len,(FILE*)fp)){
      break;
    }
    
		sendPacket.count = strlen(line);
		memcpy(sendPacket.data,line,sendPacket.count-1);
		
		/* Wait for ACK, or timeout */
    
		while(1){
       /*send packet */
		  htons(sendPacket.count);
		  htons(sendPacket.seqNum);
     
			bytes_sent = sendto(sock_client, (char*)&sendPacket, sendPacket.count+4, 0,(struct           sockaddr *) &server_addr, sizeof (server_addr));
      packetTotal++;
   
      if(retransmit){
        printf("Packet %d retransmitted with %d data bytes\n", sendPacket.seqNum, bytes_sent);
        retransmitTotal++;
        retransmit = 0;
      } 
      else{
		    printf("Packet %d transmitted with %d data bytes\n", sendPacket.seqNum, bytes_sent);  
		    countSum += sendPacket.count;
		  }
   
      
      /* Create timeout */
			struct timeval timeout;
			timeout.tv_sec = seconds;
			timeout.tv_usec = microseconds;
			setsockopt(sock_client, SOL_SOCKET, SO_RCVTIMEO, (const void *)&timeout, sizeof(timeout));
			
			/* get ACK from server */

			bytes_recd = recvfrom(sock_client, (struct Packet *)&recvPacket, sizeof(recvPacket), 0, (struct sockaddr *) 0, (int *) 0);
				
			ntohs(recvPacket.seqNum);
			
			// this should return the packet w/the ack receieved
			
			/* Check for timeout, resend packet */
			if(bytes_recd <= 0){
				timeoutTotal++;
				printf("Timeout expired for packet numbered %d\n", sendPacket.seqNum);			
        retransmit = 1;
				
			}
			
			/* no timeout, ACK received */
			else{ 
				ACKTotal++;
        
				/* Expected ACK */
				if(recvPacket.seqNum == sendPacket.seqNum){
					printf("ACK %d received\n", recvPacket.seqNum);
          /* Change sequence number */
					sendPacket.seqNum = !sendPacket.seqNum;
					break;
				}
			}		
		}	
	}
	
	/* Transmit EOT packet */
	sendPacket.count = 0;
	htons(sendPacket.count);
	htons(sendPacket.seqNum);
	bytes_sent = sendto(sock_client, (struct Packet*)&sendPacket, sendPacket.count+4, 0,(struct sockaddr *) &server_addr, sizeof (server_addr));
	printf("\nEnd of Transmission Packet with sequence number %d transmitted with %d data bytes\n\n", sendPacket.seqNum, 0);
	
	/* Print sender statistics */
	printf("Number of data packets transmitted (initial transmit only): %d \n",packetTotal - retransmitTotal);
	printf("Total number of data bytes transmitted: %d \n",countSum);
	printf("Total number of retransmissions: %d \n", retransmitTotal);
	printf("Total number of data packets received: %d \n", packetTotal);
	printf("Number of ACKs received: %d \n", ACKTotal);
	printf("Count of how many timeouts occurred: %d \n", timeoutTotal);
	
	/* close the socket */
	close (sock_client);
}
