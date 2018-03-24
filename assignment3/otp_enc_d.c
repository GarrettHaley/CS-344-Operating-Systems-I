/////////////////////////////////////////////////////////////////
////AUTHOR: GARRETT HALEY
////DATE: 06/01/2017
////OPERATING SYSTEMS I
////CREDITS FOR PROJECT: BENJAMIN BREWSTER, JOSHUA LIOY, MATTHEW BAKER, STACKOVERFLOW, AND PIAZZA
////OTP PROJECT
//////////////////////////////////////////////////////////////////
//Needed header files
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
////////////////////////////////////
int main(int argc, char ** argv) {
   int i;// initialize counter
   int listeningPort;//initialize listening port #
   int socketfd;//initialze socket id
   int client_socket;//initialize client id
   int status;//initialize status

   if (argc < 2) {//if there are too few arguments, remind user to include a port number and exit
	   fprintf(stderr, "Please include a port number!\n");
	   exit(1);
   }
   else {
	   listeningPort = atoi(argv[1]);//if there are enough arguments, set the second argument to the listening port #
   }
   if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {// if unable to create the socket, return an error message and exit
	   fprintf(stderr, "Can't create socket\n");
	   exit(1);
   }
   //intiialize socket server///////
   struct sockaddr_in server;
   server.sin_family = AF_INET;
   server.sin_port = htons(listeningPort);
   server.sin_addr.s_addr = INADDR_ANY;
//////////////////////
   //Socket binding
   if(bind(socketfd, (struct sockaddr *) &server, sizeof(server)) == -1) {//if binding doesn't work, return an error msg and exit
	   fprintf(stderr, "Error: can't bind; socket already in use\n");
	   exit(1);
   }

   if(listen(socketfd, 5) == -1) {//if listening on socketfd fails return a msg and exit
	   fprintf(stderr, "Error: listen failed\n");
	   exit(1);
   }
   while(1) {// while true ->
	   client_socket = accept(socketfd, NULL, NULL);//check tp accept call
	   if (client_socket == -1) {	// if we cannot accept call->return error regarding call and exit
		   fprintf(stderr, "Error: can't  tp accept call\n");
		   exit(1);
	    }
	int pid = fork();//creat child process
	if (pid == -1) {//if it could not be created return a fork message 
	   fprintf(stderr, "Error: bad fork\n");
	}
	else if(pid == 0) {//if pid is zero initialize toSend
	   int toSend = htonl(1);
	   if(send(client_socket, &toSend, sizeof(toSend),0) == -1) {//if we could not send, return an error and exit
		   fprintf(stderr, "Error: client failed to send\n");
		   exit(1);
		}
	   int pNum;//intialize pNumber
	   if(recv(client_socket, &pNum, sizeof(pNum), 0) == -1) {//check if plaintextsize is correct, return error if its not
		   fprintf(stderr, "Error: plaintext size end_d -1\n");
	   }
	   else if(pNum == 0) {// if its of size zero, its still an error, return error
		   fprintf(stderr, "Error: plaintext size of 0\n");
	   }	
	   int pLen = ntohl(pNum);//set plaintextLen
	   int kNum;//set keyNumber
	   if(recv(client_socket, &kNum, sizeof(kNum), 0) == -1) {// if receive is errorous, return an error about key size
		   fprintf(stderr, "Error: key size end_d -1\n");
	   }
	   else if(kNum == 0) {//if key number is zero return an error about size
		   fprintf(stderr, "Error: key size of 0\n");
	   }
	   int kLen = ntohl(kNum);//initialize klen with knum
   	   char *plainText = malloc(sizeof(char) * pLen); //allocate memory for plaintext
   	   char buffer[1024];//set buffer
   	   memset(plainText, '\0', pLen);//memset plaintext
	   int len = 0;// set len
	   int r;//set r
	   while(len <= pLen) {//loop through the length of plen incrementing on len
	      memset((char *)buffer, '\0', sizeof(buffer));//memset buffer
	      r = recv(client_socket, &buffer, 1024, 0);//receive the txt
 	      if(r == -1) {//if it could not be received return an error regarding the plaintext file and break
			   fprintf(stderr, "Error: bad reception of plaintext file\n");
			   break;
	      }
	      else if (r == 0) {//if recv return 0 ->
			   if (len < pLen) {//and len is still less than plaintext len
			   break;//break the loop
		 	   }
	      }
	      else {
			  strncat(plainText,buffer,(r - 1));//If the length of the C string in source is less than num, only the content up to the terminating null-character is copied.
	      } 
	      len += (r-1);//increment len by r-1
	   }
	   plainText[pLen - 1] = '\0';//set null termination
   	   char *keyText = malloc(sizeof(char) * kLen); // allocate keytext
   	   memset((char *)buffer, '\0', sizeof(buffer));//memset buffer
	   memset(keyText, '\0', kLen);//memset keytext with keylength
	   len = 0;//reset len
	   while(len <= kLen) {//loop through klen incrementing by len
   	      memset((char *)buffer, '\0', sizeof(buffer)); //memset buffer
	      r = recv(client_socket, &buffer, 1024, 0);//Receive text
	   	if(r == -1) {//if there is an error receving key file, return an error regarding the key file and break
		   fprintf(stderr, "Error: bad reception of key file\n");
	  	   break;
	   	}
	   	else if (r == 0) {// if r is zero then break the loop
		       break;
		   }
		   else {
			   strncat(keyText,buffer,(r - 1));//If the length of the C string in source is less than num, only the content up to the terminating null-character is copied.
		   }   
	      len += (r - 1);//increment len with r-1
	   }
	   keyText[kLen - 1] = '\0';//set to null termination
	   int plainNum;//intialize plaintext num
	   int keyNum;// initialize keynum
	   int enNum;// initialize enNum
         //encryption//////////////////////////////////
	   for (i = 0; i < pLen - 1; i++) {
		   if(plainText[i] == ' ') {
			   plainNum = 26;
			}
		else {
		   plainNum = plainText[i] - 65;
		}

		if(keyText[i] == ' ') {
		   keyNum = 26;
		}
		else {
		   keyNum = keyText[i] - 65;
		}

		enNum = plainNum + keyNum;
		if (enNum >= 27) {
			enNum -= 27;
		}

		if(enNum == 26) {
		   plainText[i] = ' ';
		}
		else {
		   plainText[i] = 'A' + (char)enNum;
		}
	   }
         ///////////////////////////////////////////////
	   len = 0;//reset len
	   while (len <= pLen) {//lop through plen incrementing on len
		   char cipherSend[1024];//set ciphersend
		   strncpy(cipherSend, &plainText[len], 1023);//copy plaintext into ciphersend
		   cipherSend[1024] = '\0';//set to null termination
		   if(send(client_socket, &cipherSend, 1024, 0) == -1) {//if there is an issue sending the encryption, return encryption error 
			   fprintf(stderr, "Error: by send of encryption text\n");
			}
			len += 1023;//increment by 1023
	   }
	   free(plainText);//free plaintext memory
	   free(keyText);//key keyText memory
	}      
	else {
	   close(client_socket);//close the client socket
	   do {
		   waitpid(pid, &status, 0);//clear the pid's
	   } while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
   }

   close(socketfd);//close fd socket
   return 0;//return from this huge function
}
