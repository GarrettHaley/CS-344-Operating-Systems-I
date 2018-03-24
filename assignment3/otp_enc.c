/////////////////////////////////////////////////////////////////
////AUTHOR: GARRETT HALEY
////DATE: 06/01/2017
////OPERATING SYSTEMS I
////CREDITS FOR PROJECT: BENJAMIN BREWSTER, JOSHUA LIOY, MATTHEW BAKER, STACKOVERFLOW, AND PIAZZA
////OTP PROJECT
//////////////////////////////////////////////////////////////////
//needed header files///////////////////////////////////////////
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<netdb.h>
////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
   int i;//set up increment i
   if (argc < 4) {// if arguments present are to small, let the user know the proper argument and exit
       fprintf(stderr, "Error: please include plaintext, key, and port number\n");
       exit(1);
   }

   int portNum = atoi(argv[3]);//sets portnumber to 4th argument from argc
   int fdPlain = open(argv[1], O_RDONLY);//plaintext open read only
   int fdKey = open(argv[2], O_RDONLY);// key open read only
   if (fdPlain == -1 || fdKey == -1) {//if these can't be opened->
       fprintf(stderr, "error opening files\n");//print an error
       exit(1);//exit
   }
   int pLen = lseek(fdPlain, 0, SEEK_END);//move in plaintext to EOF
   int kLen = lseek(fdKey, 0, SEEK_END);//move in key to EOF
   if (kLen < pLen) {//if keylength is smaller than plaintext return a key error and exit
       fprintf(stderr, "Error: key is too short\n");
       exit(1);
   }
   char *plainText = malloc(sizeof(char) * pLen); //malloc plaintext
   lseek(fdPlain, 0, SEEK_SET);//go to EOF
   if (read(fdPlain, plainText, pLen) == -1) {// return error and exit if plaintext can't be read
       fprintf(stderr, "Error: can't read plaintext\n");
       exit(1);
   }

   plainText[pLen - 1] = '\0';//set to null termination

   for (i = 0; i < pLen - 1; i++) {//loop through and check for correct characters
	if(isalpha(plainText[i]) || isspace(plainText[i])) {
	}
	else {
	   fprintf(stderr, "Error: input contains bad characters\n");//if plaintext characters aren't alphabetical or spaces, return an error and exit
	   exit(1);
	}
   }

   char *keyText = malloc(sizeof(char) * kLen); //malloc keytext to keylength size
   lseek(fdKey, 0, SEEK_SET);//set to EOF
   if (read(fdKey, keyText, kLen) == -1) {//if the key cannot be read, return an error and exit
       fprintf(stderr, "Error: bad key\n");
       exit(1);
   }
   keyText[kLen - 1] = '\0';//set to null termination
   for (i = 0; i < kLen - 1; i++) {//loop to check for valid characters
	if(isalpha(keyText[i]) || isspace(keyText[i])) {
	}
	else {//else there is an invalid error and a message should be returned follow by exiting
	   fprintf(stderr, "Error: invalid characters in key\n");
	   exit(1);
	}
   }
   int socketfd;//initialize fd socket
   if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {//if there is a socket issue, return a socket error and exit
       fprintf(stderr, "socket error\n");
       exit(1);
   }
   struct hostent * server_ip_address;//intiialize host address
   server_ip_address = gethostbyname("localhost");//set to localhost
   if(server_ip_address == NULL) {// if its null, then there is hostname problem, return an error and exit
       fprintf(stderr, "Error: unable to resolve host name\n");
       exit(1);
   }
   struct sockaddr_in server;//socket in server struct initialization
   memset((char *)&server, 0, sizeof(server));//memset server size
   server.sin_family = AF_INET;//Benjamin Brewster initialization code
   server.sin_port = htons(portNum);//Benjamin Brewster initialization code
   memcpy(&server.sin_addr, server_ip_address->h_addr, server_ip_address->h_length);//Benjamin Brewster initialization code
   if(connect(socketfd, (struct sockaddr*) &server, sizeof(server)) == -1) {//if we cann't connect return error message and connect
       fprintf(stderr, "connect\n");
       exit(2);
   }
   int r;//initialize
   int conNum;
   if((r = recv(socketfd, &conNum, sizeof(conNum), 0)) == -1) {//if we do not receive an encoding, return error message and exit
       fprintf(stderr, "Error: did not receive enconde\n");
       exit(1);
   } 
   else if(r == 0) {// if r is one, we have a bad encoding, reutnr an error and exit
       fprintf(stderr, "Error: bad encode\n");
       exit(1);
   }
   int confirm = ntohl(conNum);//initialize confirm
   if (confirm != 1) {//if confim is an error, return there was a connect -> specify port and exit
       fprintf(stderr, "Error: can't connect to otp_enc_d on port %d\n", portNum);
       exit(2);
   }
   int pLenSend = htonl(pLen);//initialize plaintextln
   if(send(socketfd, &pLenSend, sizeof(pLenSend), 0) == -1) {//if plaintext can't be send, return error and exit
       fprintf(stderr, "Error: can't send plaintext\n");
       exit(1);
   }
   int kLenSend = htonl(kLen);//set sender keylength like above..
   if(send(socketfd, &kLenSend, sizeof(kLenSend), 0) == -1) {//if key couldn't be sent, return a message and exit
       fprintf(stderr, "Error: can't send key\n");
       exit(1);
   }
   int len = 0;//initialze len
   while (len <= pLen) {//while its less than plaintext -> loop
        char plainSend[1024];//set plainsend size
        strncpy(plainSend, &plainText[len], 1023);//copy plaintext into plainsend at locaiton len
        plainSend[1024] = '\0';//set to null termination
        if(send(socketfd, &plainSend, 1024, 0) == -1){//if there was an error sending plaintext, return an error message and exit
        fprintf(stderr, "Error can't send plaintext\n");
        exit(1);
        }
        len += 1023;//increment len for counter
   }
   len = 0;//reset len
   while (len <= kLen) {//while len is less than keylength -> loop
        char keySend[1024];//set keySend size
        strncpy(keySend, &keyText[len], 1023);//copy keytext into keysend
        keySend[1024] = '\0';//set to null termination
        if(send(socketfd, &keySend, 1024, 0) == -1){// if we are unable to send key, return error and exit
            fprintf(stderr, "Error: can't send key\n");
            exit(1);
        }

        len += 1023;//increments by len
   }
   char *cipherText = malloc(sizeof(char) * pLen);//malloc ciphertext
   char buffer[1042];//set buffer
   memset(cipherText, '\0', pLen);//memset cipher
   len = 0;//reset len
   r = 0;//reset r
   while(len < pLen) {//while len is less than plainLength
        memset((char *)buffer, '\0', sizeof(buffer));//memset buffer
        r = recv(socketfd, buffer, 1024, 0);//check if cipher recieved
            if(r == -1) {//if cipher wasn't received, send error message and exit
                fprintf(stderr, "Error: did not receive cipertext\n");
                exit(1);
            }	   
        else if(r == 0) {// if r is zero, there was also an error receivng the ciphertexxt, return and error and exit
        if(len < pLen) {
            fprintf(stderr, "Error: did not receive cipertext\n");
            exit(1);
        }
        }
        else {
        strncat(cipherText,buffer,(r-1));//If the length of the C string in source is less than num, only the content up to the terminating null-character is copied.
        }	   
        len += (r-1);//increment on len for loop
   }
   cipherText[pLen - 1] = '\0';//set to null termination
   printf("%s\n", cipherText);//print ciphertext
   free(plainText);//free memory for plaintext
   free(keyText);//free memory for keytext
   free(cipherText);//free memory for ciphertext
   return 0;//return out of this huge function
}
