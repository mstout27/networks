/* tcp_ client.c */ 
/* Programmed by Adarsh Sethi */
/* Sept. 13, 2018 */     

#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, connect, send, and recv */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024


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

   int sock_client;  /* Socket used by client */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
   char server_hostname[STRING_SIZE]; /* Server's hostname */
   unsigned short server_port;  /* Port number used by server (remote port) */

   
   
   struct Command serverMessage; /* received message */
   
 
   struct Command cmd;  /* message to be sent */
   
   unsigned int msg_len;  /* length of message */                      
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
  
   /* open a socket */

   if ((sock_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      perror("Client: can't open stream socket");
      exit(1);
   }

   /* Note: there is no need to initialize local client address information 
            unless you want to specify a specific local port
            (in which case, do it the same way as in udpclient.c).
            The local address initialization and binding is done automatically
            when the connect function is called later, if the socket has not
            already been bound. */

   /* initialize server address information */
   
   
   
   printf("Enter hostname of server: ");
   scanf("%s", server_hostname);
   if ((server_hp = gethostbyname(server_hostname)) == NULL) {
      perror("Client: invalid server hostname");
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

    /* connect to the server */
   
   
   
   if (connect(sock_client, (struct sockaddr *) &server_addr, 
                                    sizeof (server_addr)) < 0) {
      perror("Client: can't connect to server");
      close(sock_client);
      exit(1);
   }
  
   /* user interface */


   
   
   /*parse through client command to identify action, account type, and value, and assign the inputs to a struct */
  
   int exit = 0;  /* bool determining if user wishes to continue making transactions */
   while(!exit){
   
	   /* determine action to perform */
	   
	   printf("Please enter your desired action. (check/transfer/deposit/withdraw/dc) \n");
	   scanf("%s", cmd.arg1);
	   while(strcmp(cmd.arg1,"check") && strcmp(cmd.arg1,"transfer") && strcmp(cmd.arg1,"deposit") && strcmp(cmd.arg1,"withdraw") && strcmp(cmd.arg1,"dc")){
		 printf("Invalid action. Please try again. (check/transfer/deposit/withdraw/dc)\n");
		 scanf("%s", cmd.arg1); 
	   }
	   
	   /* determine account type to be referenced */
	   
	   if(!(strcmp(cmd.arg1,"dc"))){ /* if user wishes to disconnect */
		   exit = 1; 
		   printf("Thank you for using our services. We hope to see you again!\n");
	   }
	   else{
	   
		   if(!(strcmp(cmd.arg1,"transfer"))){ 
			 /* additional clarification for transfer */
			 printf("Please enter the account type to transfer funds into. (checking/savings) \n");
			 scanf("%s", cmd.arg2);
		   }
		   else{
			 printf("Please enter the account type. (checking/savings) \n");
			 scanf("%s", cmd.arg2);
		   }
		   while(strcmp(cmd.arg2,"checking") && strcmp(cmd.arg2,"savings")){
			 printf("Invalid account type. Please try again. (checking/savings)\n");
			 scanf("%s", cmd.arg2); 
		   }
		   
		   /* determine amount to be changed, unless checking current balance */
		   if(strcmp(cmd.arg1,"check")){
			 printf("Please enter the amount to be changed. \n");
			 scanf("%d", &cmd.arg3);
		   }

		   /* send message */
		   bytes_sent = send(sock_client, (char *) &cmd, sizeof(cmd), 0);
		   printf("Sent message of %d bytes\n", bytes_sent);

		   
		   /* get response from server */
		   bytes_recd = recv(sock_client, (char *)&serverMessage, sizeof(serverMessage), 0); 
		   printf("Received server message of %d bytes\n", bytes_recd);
		   
		   printf("The response from server is:\n\n");

		   printf("Account type: %s\n",serverMessage.arg2);
		   printf("Previous balance: %d\n",serverMessage.arg3);
		   printf("Current balance: %d\n",serverMessage.acctBal);
		   printf("Error message: %s\n",serverMessage.errorCode);   
		   
	   } 
   }
   /* close the socket */
   close (sock_client);
}
