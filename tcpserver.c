/* tcpserver.c */
/* Programmed by Adarsh Sethi */
/* Sept. 13, 2018 */    

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, bind, listen, accept */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024   

/* SERV_TCP_PORT is the port number on which the server listens for
   incoming requests from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_TCP_PORT 61111

struct Command{
  char arg1[STRING_SIZE];    //specified action
  char arg2[STRING_SIZE];    //account type
  
  /* Client-side = amount to change
     Server-side = pre-change balance */
  int arg3;     
  int acctBal;   //Balance in relavent account, returned by server
  char errorCode[STRING_SIZE]; //Possible error code returned by server
  
};

int main(void) {
  
   int sock_server;  /* Socket on which server listens to clients */
   int sock_connection;  /* Socket on which server exchanges data with client */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   unsigned int server_addr_len;  /* Length of server address structure */
   unsigned short server_port;  /* Port number used by server (local port) */

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned int client_addr_len;  /* Length of client address structure */

   
   struct Command cmd;
   
   /* message to send back to client */
   struct Command serverMessage;

   int amt; /* temp variable used in logic conditions */   
   unsigned int client_msg_len;  /* length of client's message */ 
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
   unsigned int i;  /* temporary loop variable */

   /* open a socket */

   if ((sock_server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      perror("Server: can't open stream socket");
      exit(1);                                                
   }

   /* initialize server address information */
    
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
                                        any host interface, if more than one
                                        are present */ 
   server_port = SERV_TCP_PORT; /* Server will listen on this port */
   server_addr.sin_port = htons(server_port);

   /* bind the socket to the local server port */

   if (bind(sock_server, (struct sockaddr *) &server_addr,
                                    sizeof (server_addr)) < 0) {
      perror("Server: can't bind to local address");
      close(sock_server);
      exit(1);
   }                     

   /* listen for incoming requests from clients */

   if (listen(sock_server, 50) < 0) {    /* 50 is the max number of pending */
      perror("Server: error on listen"); /* requests that will be queued */
      close(sock_server);
      exit(1);
   }
   printf("I am here to listen ... on port %d\n\n", server_port);
   client_addr_len = sizeof (client_addr);
    
    int checkSum = 0;
    int saveSum = 0;
    
   
   /* wait for incoming connection requests in an indefinite loop */
   for (;;) {
      sock_connection = accept(sock_server, (struct sockaddr *) &client_addr, 
                                         &client_addr_len);
                     /* The accept function blocks the server until a
                        connection request comes from a client */
      if (sock_connection < 0) {
         perror("Server: accept() error\n"); 
         close(sock_server);
         exit(1);
      }
 
     
      
	  /* loop that allows multiple client transactions */
	  int exit = 0;
  	  while(!exit){ 
	  
	      /* receive the message */
		  bytes_recd = recv(sock_connection, (char *)&serverMessage, sizeof(cmd), 0); 
		  
		  if(!(strcmp(serverMessage.arg1,"dc"))){
			  exit = 1;
		  }
		  else{
		  
			  if (bytes_recd > 0){
				 printf("Message received with length %d\n\n", bytes_recd);
				 printf("Received message is: %s %s %d\n",serverMessage.arg1,serverMessage.arg2,serverMessage.arg3);
				 printf("Requested action: %s\n", serverMessage.arg1);
				 printf("Account type: %s\n",serverMessage.arg2);
				 
				 

				
				/* prepare the message to send */
				 msg_len = bytes_recd;
				 
				 
				 /*conditions imposed on client requests*/
				 /* check */
				 if(!(strcmp(serverMessage.arg1,"check"))){     
				   if(!(strcmp(serverMessage.arg2,"checking"))){
					 serverMessage.acctBal = checkSum;
				   }
				   if(!(strcmp(serverMessage.arg2,"savings"))){
					 serverMessage.acctBal = saveSum;
				   }
				   serverMessage.arg3 = -1;

				   
				 }
				 
				 
				 /* withdraw */
				 if(!(strcmp(serverMessage.arg1,"withdraw"))){
				 
				   if(!(strcmp(serverMessage.arg2,"savings"))){
					 serverMessage.arg3 = -1;
					 serverMessage.acctBal = -1;
					 strcpy(serverMessage.errorCode,"A withdrawal is only permissible from the checking account.");
				   }
				   else{
					 if(serverMessage.arg3 % 20 != 0){
					   serverMessage.arg3 = -1;
					   serverMessage.acctBal = -1;
					   strcpy(serverMessage.errorCode,"Please withdraw a value which is a multiple of 20. Transaction not executed.");
					 }
					 else if(serverMessage.arg3 > checkSum){
					  serverMessage.arg3 = -1;
					  serverMessage.acctBal = -1;
					  strcpy(serverMessage.errorCode,"Insufficient balance. Transaction not executed.");
					 }
					 else{
					   amt = serverMessage.arg3;
					   serverMessage.arg3 = checkSum;
					   checkSum -= amt;
					   serverMessage.acctBal = checkSum;   

					 }
				   }
				 }
				 
				 /* deposit */
				 if(!(strcmp(serverMessage.arg1,"deposit"))){
				   if(!(strcmp(serverMessage.arg2,"savings"))){
					 amt = serverMessage.arg3;
					 serverMessage.arg3 = saveSum;
					 saveSum += amt;
					 serverMessage.acctBal = saveSum; 

				   }
				   else{
					 amt = serverMessage.arg3;
					 serverMessage.arg3 = checkSum;
					 checkSum += amt;
					 serverMessage.acctBal = checkSum;  

				   }
				 }
				 
				 /* transfer */
				 if(!(strcmp(serverMessage.arg1,"transfer"))){
				 
				   /* transfer savings -> checking */
				   if(!(strcmp(serverMessage.arg2,"checking"))){
					 if(saveSum < serverMessage.arg3){
					   serverMessage.arg3 = -1;
					   serverMessage.acctBal = -1;
					   strcpy(serverMessage.errorCode,"Insuffient funds in savings account. Transaction not executed.");
					 }
					 else{
					   amt = serverMessage.arg3;
					   serverMessage.arg3 = checkSum;
					   saveSum -= amt;
					   checkSum += amt;
					   serverMessage.acctBal = checkSum;  

					 }
				   }
				   
				   /* transfer checking -> savings */
				   else{
					 if(checkSum < serverMessage.arg3){
					   serverMessage.arg3 = -1;
					   serverMessage.acctBal = -1;
					   strcpy(serverMessage.errorCode, "Insuffient funds in checking account. Transaction not executed.");
					 }
					 else{
					   amt = serverMessage.arg3;
					   serverMessage.arg3 = saveSum;
					   checkSum -= amt;
					   saveSum += amt;
					   serverMessage.acctBal = saveSum;  

					 }  
				   }
				 
				 
				 }
				 
				 
				 /* send message */
				 printf("%s %s %d %d %s\n",serverMessage.arg1, serverMessage.arg2, serverMessage.arg3, serverMessage.acctBal, serverMessage.errorCode);
				 bytes_sent = send(sock_connection, (char *)&serverMessage, sizeof(serverMessage), 0);
				 
				 
			  }
		  }
	  }
      /* close the socket */
      close(sock_connection);
   } 
}
