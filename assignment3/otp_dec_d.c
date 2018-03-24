/////////////////////////////////////////////////////////////////
////AUTHOR: GARRETT HALEY
////DATE: 06/01/2017
////OPERATING SYSTEMS I
////CREDITS FOR PROJECT: BENJAMIN BREWSTER, JOSHUA LIOY, MATTHEW BAKER, STACKOVERFLOW, AND PIAZZA
////OTP PROJECT
//////////////////////////////////////////////////////////////////
//INCLUDED HEADER FILE///////////////////////////////////////////
#include<netinet/in.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
/////////////////////////////////////////////////////////////////
int main(int argc, char ** argv) {
   int i;//initialize counter
   int listeningPort;//initialize listining
   int socketfd;//initializ socketH
   int client_socket;//initialize socketC
   int status;//initialize status

   if (argc < 2) {//if there are too few arguments ->
	   fprintf(stderr, "please include a port number\n");//return error message asking user to include the port number
	   exit(1);//exit
   }
   else {
	   listeningPort = atoi(argv[1]);//set the listeningport value to the second argument in argv
   }
   if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {//if there is an error in socket configuration->
	   fprintf(stderr, "Error: bad socket creation\n");//return error regarding socket creation
	   exit(1);//return error
   }
   struct sockaddr_in server;// code from Brewsters slides
   server.sin_family = AF_INET;//code from Brewsters slides
   server.sin_port = htons(listeningPort);//code from Brewsters
   server.sin_addr.s_addr = INADDR_ANY;//code from Brewsters
   if(bind(socketfd, (struct sockaddr *) &server, sizeof(server)) == -1) {//if binding returns -1, then ->
	   fprintf(stderr, "Error: socket already in use\n");//send message to user that socket exits
	   exit(1);//exit
   }
   if(listen(socketfd, 5) == -1) {// if there is a bad listenr issue ->
	   fprintf(stderr, "Error: bad listen\n");//return message regarding bad listener
	   exit(1);//exit
   }
   while(1) { //while true 
	client_socket = accept(socketfd, NULL, NULL);//clent socket accepts socketfd
	if (client_socket == -1) {//if clients socket is -1, then
	    fprintf(stderr, "Error: bad accept\n");//return error about bad client-socket acceptence
	    exit(1);//exit
	}
	int pid = fork();// fork child process
	//Create child process
	if (pid == -1) {//if fork returned -1, then->
	   fprintf(stderr, "Error: bad fork\n");//return user error regarding fork
	}
	else if(pid == 0) {//if child pid is 0 ->
	   int toSend = htonl(0);//network to host long
	   if(send(client_socket, &toSend, sizeof(toSend), 0) == -1){//if this is not sent ->
		   fprintf(stderr, "Error: client did not send\n");//return a not sent error
	   }
	   int cNum;//set ciphernumber
	   if(recv(client_socket, &cNum, sizeof(cNum), 0) == -1) {//if cipher is not received->
		   fprintf(stderr, "Error: unable to receive cipher text\n");//error message about receive cipher
	   }
	   else if(cNum == 0) {//if cipherNum is still 0 ->
		   fprintf(stderr, "Error: unable to receive cipher text\n");//error message about receive cipher
	   }
	   int cLen = ntohl(cNum);//cipher long
	   int kNum;//keyNumber initialization
	   if(recv(client_socket, &kNum, sizeof(kNum), 0) == -1) {//if the key is not received
		   fprintf(stderr, "Error: can't revceive key\n");//return key error
	   }
	   else if(kNum == 0) {//if kNum is still zero->
		   fprintf(stderr, "Error: can't receive key\n");//return key error
	   }
	   int kLen = ntohl(kNum);//setup klen long
   	   char *cipherText = malloc(sizeof(char) * cLen); //allocate ciphertext with cipherLength
   	   char buffer[1024];//set buffer
   	   memset(cipherText, '\0', cLen);//memset cipher
	   int len = 0;//initialize len
	   int r; // initialize r
	   while(len <= cLen) {//when length<cipherlength
			memset((char *)buffer, '\0', sizeof(buffer));//memset buffer
			r = recv(client_socket, &buffer, 1024, 0);//receive the buffer
			if(r == -1) {//if it couldn't be received ->
				fprintf(stderr, "recv cipher text file -1\n");//return cipher text file error
				break;//break the loop
			}
			else if (r == 0) {//r is zero ->
			if (len < cLen) {//length<cipherlen->
			break;//break the loop
			}
			}
			else {
				strncat(cipherText,buffer,(r-1));//appends the first num characters of source to destination, plus a terminating null-character
			}
			len += (r-1);//add to length to increment by r-1
	   }
	   cipherText[cLen - 1] = '\0';//set to null termination
   	   char *keyText = malloc(sizeof(char) * kLen); //allocate keytext with keylength
   	   memset((char *)&buffer, '\0', sizeof(buffer));//memset buffer
	   memset(keyText, '\0', kLen);//memset keyText
	   len = 0;//set length to zero again
	   while(len <= kLen) {//while length is less than keylength -> loop
	      memset((char *) buffer, '\0', sizeof(buffer));//memset buffer
	      r = recv(client_socket, &buffer, 1024, 0);//receive client socet (the buffer)
		   if(r == -1) {//if it couldn't be received
		       fprintf(stderr, "Error: unable to receive key\n");//return unable to receive key error
			   break;//break the loop
		   }
		   else if (r == 0) {//or if r is zero ->
			   break;//break
		   }
		   else {
			   strncat(keyText,buffer,(r-1));//appends the first num chars of source to destination plu a terminating null-character
		   }
		len += (r-1);//add to length to increment by r-1
	   }
	   keyText[kLen - 1] = '\0';//set null termination
	   int cipherNum;//set cipherNum
	   int keyNum;//set keyNu,
	   int decNum;//set decNum

	   //DECRYPT THE ENCODED TEXT FILE BELOW:
	   for (i = 0; i < cLen - 1; i++) {
			if(cipherText[i] == ' ') {
			cipherNum = 26;
			}
			else {
			cipherNum = cipherText[i] - 65;
			}

			if(keyText[i] == ' ') {
			keyNum = 26;
			}
			else {
			keyNum = keyText[i] - 65;
			}

			decNum = cipherNum - keyNum;
			if (decNum < 0) {
			decNum += 27;
			}

			if(decNum == 26) {
			cipherText[i] = ' ';
			}
			else {
			cipherText[i] = 'A' + (char)decNum;
			}
	   }
	   len  = 0;//rest leng
	   while (len <= cLen) {//if len is less than cipher length
			char plainSend[1024];//set plaintext
			strncpy(plainSend, &cipherText[len], 1023);//copy it over
			plainSend[1024] = '\0';//set to null termination
			if(send(client_socket, &plainSend, 1024, 0) == -1) {// if there is a sending error
				fprintf(stderr, "Error: can't send decode\n");//return sending error
			}
			len += 1023; //increment length
	   }
	   free(cipherText);//free memory for cipherText
	}      
	else {
	   close(client_socket);//close the socket
	   do {
		waitpid(pid, &status, 0);//set pid status
	   } while(!WIFEXITED(status) && !WIFSIGNALED(status));//while this is true
	}
   }
   close(socketfd);//close the socket
   return 0;//return from this huge function
}
