/////////////////////////////////////////////////////////////////
////AUTHOR: GARRETT HALEY
////DATE: 06/01/2017
////OPERATING SYSTEMS I
////CREDITS FOR PROJECT: BENJAMIN BREWSTER, JOSHUA LIOY, MATTHEW BAKER, STACKOVERFLOW, AND PIAZZA
////OTP PROJECT
//////////////////////////////////////////////////////////////////

//HEADER FILES///////////////////////////////////////////////////
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<netinet/in.h>
/////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
   int i;//initialize counter	
   if (argc < 4) {//if argument count is less than 4->
       fprintf(stderr, "Error: please include cipher text, key, and port number\n");//return an error message that the usage is incorrect
       exit(1);//exit
   }
   int key_fd = open(argv[2], O_RDONLY);//open key read only
   int cipher_fd = open(argv[1], O_RDONLY);//open Cipher, read only
   int portNumber = atoi(argv[3]);//get the port number fourth argument from argv
   if (key_fd == -1 || cipher_fd == -1) {//if either couldn't open->
	fprintf(stderr, "Error: can't open files\n");//return error message
	exit(1);//exit
   }
   int cLen = lseek(cipher_fd, 0, SEEK_END);// file offset shall be set to the size of the cfile plus offset
   int kLen = lseek(key_fd, 0, SEEK_END);//file offset shall be set to the size of the kfile plus offset
   if (kLen < cLen) {//if the key length is smaller than the cipher length ->
	fprintf(stderr, "Error: key is too short\n");//return an error message
	exit(1);//exit
   }
   char *cipherText = malloc(sizeof(char) * cLen); // allocation initialization
   lseek(cipher_fd, 0, SEEK_SET);//cipher offset shall be set to offset bytes
   if (read(cipher_fd, cipherText, cLen) == -1) {//if we cannot open fl ->
	fprintf(stderr, "Error: can't read cipher text\n");//return error to user
	exit(1);//exit
   }
   cipherText[cLen - 1] = '\0';//set to null termination
   for (i = 0; i < cLen - 1; i++) {//loop through length clength
        if(isalpha(cipherText[i]) || isspace(cipherText[i])) {//if ciphertext is an alphabetical character or space->
		continue; //continue onwards
        }
        else { 
            fprintf(stderr, "Error: cipher text contains invalid characters\n");//return error message to user about invalid chars
            exit(1);//exit
        }
   }
   char *keyText = malloc(sizeof(char) * kLen); //allocation initialization
   lseek(key_fd, 0, SEEK_SET);// key offset shall be set to offset bytes
   if (read(key_fd, keyText, kLen) == -1) {// if we cannot open fl ->
       fprintf(stderr, "Error: can't read key\n");//return error regarding the key
       exit(1);//exit
   }
   keyText[kLen - 1] = '\0';//set key to null termination
   for (i = 0; i < kLen - 1; i++) {//loop through klength times ->
        if(isalpha(keyText[i]) || isspace(keyText[i])) {//Continue if character is alphabetic or a space
		continue;//continue onwards
        }
        else {
            fprintf(stderr, "Error: key contains invalid characters\n");//else return an error
            exit(1);//exit
        }
   }
   int socketfd;//initialize
   if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {//if there is a socket err-> 
       fprintf(stderr, "socket error\n");//return the error to user
       exit(1);//exit
   }
   //Setting up address
   struct hostent * server_ip_address;//initialize host struct 
   server_ip_address = gethostbyname("localhost");//set up local host
   if(server_ip_address == NULL) {//if ip is null ->
       fprintf(stderr, "Error: cannot resolve host name issue\n");//return error regarding host nae
       exit(1);//exit
   }
   struct sockaddr_in server;//initialize server struct
   memset((char *)&server, 0, sizeof(server));//allocate
   server.sin_family = AF_INET;//Networking code from Professor Brewster;
   server.sin_port = htons(portNumber);//Networking code from Professor Brewster;
   memcpy(&server.sin_addr, server_ip_address->h_addr, server_ip_address->h_length);//Networking code from Professor Brewster;
   if(connect(socketfd, (struct sockaddr*) &server, sizeof(server)) == -1) {//if server connection cannot be made ->
       fprintf(stderr, "Error: cannot connect\n");//return connection error
       exit(2);//exit
   }
   int r;//initialize
   int conNum;//initialize
   if((r = recv(socketfd, &conNum, sizeof(conNum), 0)) == -1) {//if recv returns -1 ->
       fprintf(stderr, "Error: bad reception of encoded\n");//return a reception error
       exit(1);//exit
   } 
   else if(r == 0) {//if recv returns 0
       fprintf(stderr, "Error: bad reception of enconded\n");//return a reception error as above.
       exit(1);//exit
   }
   int confirm = ntohl(conNum);//initialize confirmation
   if (confirm != 0) {//it confirmation is not equal to zero ->
       fprintf(stderr, "Error: cannot connect to otp_dec_d on port %d\n", portNumber);//return error regarding otp_dec_d on port
       exit(2);//exit
   }
   int cLenSend = htonl(cLen);//initialize cLenSend to Clength
   if(send(socketfd, &cLenSend, sizeof(cLenSend), 0) == -1) {//if send returns -1: we cannot send the cipher ->
       fprintf(stderr, "Error: cannot send cipher text\n");//return message about cipher
       exit(1);//exit
   }
   int kLenSend = htonl(kLen);//key length send initialized to key length
   if(send(socketfd, &kLenSend, sizeof(kLenSend), 0) == -1) {// if send returns -1, we cannot send the key ->
       fprintf(stderr, "Error: cannot send key\n");//return message about key
       exit(1);//exit
   }
   int len = 0;//initialize length
   while (len <= cLen) {//loop through the cipherlength ->
        char cipherSend[1024];// initialize ciphersend with 1024
        strncpy(cipherSend, &cipherText[len], 1023);//copy cipher/cipherText
        cipherSend[1024] = '\0';//set null termination
        if(send(socketfd, cipherSend, 1024, 0) == -1){//if cipher send returns -1
        printf("Error: cannot send cipher ciphertext\n");//send error regarding sending cipher text
        exit(1);//exit
        }
        len += 1023;//add this length continually to total length
   }
   len = 0;//reset to 0
   while (len <= kLen) {//loop through keylength
	char keySend[1024];//initialize keysend with 1024
	strncpy(keySend, &keyText[len], 1023);//copy keysend into address keyText at location len
	keySend[1024] = '\0';//set null termination
   	if(send(socketfd, &keySend, 1024, 0) == -1){// if we cannot send the key
	   fprintf(stderr, "Error: cannot send keyd\n");//inform the user
	   exit(1);//exit
	}
	len += 1023;//add 1023 each loop through keylength
   }
   char *plainText = malloc(sizeof(char) * cLen);//allocate plaintext space
   char buffer[1024];//allocate char buffer
   memset(plainText, '\0', cLen);//allocation
   len = 0;//reset len
   r = 0;//reset r
   while(len < cLen) {//loop through cipher length
        memset((char *)buffer, '\0', sizeof(buffer));//allocate
        r = recv(socketfd, &buffer, 1024, 0);//receive plaintext
        if(r == -1) {//if plaintext was not returned
            fprintf(stderr, "Error: plaintext was not received\n");//return a plaintext error
            exit(1);//exit
        }	   
        else if(r == 0) {//also if r is zero and length is less than cipherlen, then plaintext was not recieved =>
                if (len < cLen) {
                    fprintf(stderr, "Error: did not receive plaintext\n");//return a plaintext error exactly like above
                    exit(1);//exit
                }
        }
        else {
            strncat(plainText,buffer,(r-1));//If the length of the string in source is less than num, only the content up to the terminating null-character is copied.
        }	   
        
        len += (r-1);// add r-1 to the length
   }
   plainText[cLen - 1] = '\0';//set null termination
   printf("%s\n", plainText);//print the plaintext
   free(plainText);//free allocated memory for plaintext
   free(keyText);//free allocated memory for keyText
   free(cipherText);//free allocated memeory for cipherText
   return 0;
}
