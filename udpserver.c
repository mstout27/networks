/* udp_server.c */
/* Programmed by Matt Stout & Bree McCausland */
/* Nov. 28, 2018 */

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */
#include <time.h>

#define STRING_SIZE 1024

/* SERV_UDP_PORT is the port number on which the server listens for
   incoming messages from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_UDP_PORT 65115


int simulateLoss(double packetLossRate){
	double random = (double)rand() / (double)RAND_MAX;
	if(random <= packetLossRate){
		return 0;
	}
	else{
		return 1;
	}
}

int simulateACKLoss(double ACKLossRate){
	double random = (double)rand() / (double)RAND_MAX;
	if(random <= ACKLossRate){
		return 0;
	}
	else{
		return 1;
	}
	
}


int main(void) {
  srand (time(NULL));
	char buffer[STRING_SIZE];
   
	struct Packet{
		short count;
		short seqNum;
		char data[80];
	};
	
   int sock_server;  /* Socket on which server listens to clients */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   unsigned short server_port;  /* Port number used by server (local port) */

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned int client_addr_len;  /* Length of client address structure */

   char sentence[STRING_SIZE];  /* receive message */
   char modifiedSentence[STRING_SIZE]; /* send message */
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
   unsigned int i;  /* temporary loop variable */

   /* open a socket */

   if ((sock_server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("Server: can't open datagram socket\n");
      exit(1);
   }

   /* initialize server address information */

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
                                        any host interface, if more than one
                                        are present */
   server_port = SERV_UDP_PORT; /* Server will listen on this port */
   server_addr.sin_port = htons(server_port);

   /* bind the socket to the local server port */

   if (bind(sock_server, (struct sockaddr *) &server_addr,
                                    sizeof (server_addr)) < 0) {
      perror("Server: can't bind to local address\n");
      close(sock_server);
      exit(1);
   }

   /* wait for incoming messages in an indefinite loop */

   printf("Waiting for incoming messages on port %hu\n\n", 
                           server_port);

   client_addr_len = sizeof (client_addr);
/*-------------------------------------------------------------------------*/
	
	
	/* user configuration interface */
	double packetLossRate;
	double ACKLossRate;
	
	printf("Enter value for Packet Loss Rate (between 0 and 1): ");
	scanf("%lf", &packetLossRate);
	
	printf("Enter value for ACK Loss Rate (between 0 and 1): ");
	scanf("%lf", &ACKLossRate);
	
    /* Stop and Wait protocol logic */
   
  struct Packet sentPacket;
	struct Packet recvPacket;
	 
  short curr_seqNum = 1;
  int exit = 0;
	
	int packetTotal = 0;
	int countSum = 0;
	int dupeSum = 0;
	int lossSum = 0;
	int goodACKTotal = 0;
	int lossACKTotal = 0;
 
  FILE *fp;
  fp = fopen("output.txt", "w");
 
    while(!exit) {
		
		bytes_recd = recvfrom(sock_server, (char *)&recvPacket, sizeof(recvPacket), 0, (struct sockaddr *)&client_addr, &client_addr_len);
		ntohs(recvPacket.count);
		ntohs(recvPacket.seqNum);
		
		
		/* End Of Transmission: exit loop */
		if(!recvPacket.count){
			printf("\nEnd of Transmission Packet with sequence number %d received with %d data bytes\n", recvPacket.seqNum, bytes_recd);
			exit = 1;
		}
		
		/* Normal packet: continue loop */
		else{
			/* If loss, restart loop */
			if(simulateLoss(packetLossRate)){
				
			/* No loss: continue checking packet */
      
				/* Check if packet is duplicate */
				if(curr_seqNum == recvPacket.seqNum){
					dupeSum++;
					printf("Duplicate packet %d received with %d data bytes\n", recvPacket.seqNum, bytes_recd);
					
					htons(recvPacket.seqNum);
				
					bytes_sent = sendto(sock_server, (char *)&recvPacket, recvPacket.count+4, 0,(struct sockaddr *) &client_addr, client_addr_len);
				}
				
				/* Not a duplicate: continue */
				else{
					packetTotal++;
					countSum += recvPacket.count;
                   
					/* add data to output & update sequence number */
  				//fprintf(fp, "%s", recvPacket.data);
          fputs(recvPacket.data,(FILE*)fp);
					curr_seqNum = recvPacket.seqNum;
					
					
					/* Possibly simulate ACK loss */
					if(!simulateACKLoss(ACKLossRate)){
						/* lost ACK */
						printf("ACK %d lost\n", recvPacket.seqNum);
						lossACKTotal++;
					}
					/* No ACK loss: continue */
					else{
            printf("Packet %d received with %d data bytes\n", recvPacket.seqNum, bytes_recd);
						goodACKTotal++;
						/* send ACK */
            curr_seqNum = recvPacket.seqNum;
						bytes_sent = sendto(sock_server, (char *)&recvPacket, recvPacket.count+4, 0,(struct sockaddr *) &client_addr, client_addr_len);
						printf("ACK %d transmitted\n", recvPacket.seqNum);
					}	
				}
			}
			else{
				printf("Packet %d lost\n", recvPacket.seqNum);
				lossSum++;
			}
		}
		
   }  
   
	/* Print receiver statistics */
		printf("\nNumber of data packets received successfully: %d \n", packetTotal);
		printf("Total number of data bytes received which are delivered to user: %d \n", countSum);
		printf("Total number of duplicate data packets received: %d \n", dupeSum);
		printf("Number of data packets received but dropped due to loss: %d \n", lossSum);
		printf("Total number of data packets received: %d \n", packetTotal+dupeSum+lossSum);
		printf("Number of ACKs transmitted without loss: %d \n", goodACKTotal);
		printf("Number of ACKs generated but dropped due to loss: %d \n", lossACKTotal);
		printf("Total number of ACKs generated: %d \n", goodACKTotal+lossACKTotal);
    fclose(fp);
}
