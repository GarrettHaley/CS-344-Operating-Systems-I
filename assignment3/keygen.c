/////////////////////////////////////////////////////////////////
//AUTHOR: GARRETT HALEY
//DATE: 06/01/2017
//OPERATING SYSTEMS I
//CREDITS FOR PROJECT: BENJAMIN BREWSTER, JOSHUA LIOY, MATTHEW BAKER, STACKOVERFLOW, AND PIAZZA
//OTP PROJECT
////////////////////////////////////////////////////////////////

////Included Headers/////////////////////////////////////////////
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
   int randomNumber;//initialize random number
   int randomCharacter;//initialize random number
   int x;//initialize counter
   srand(time(NULL));//randomize seed

   if (argc < 2) {// if there are less than two arguments
	printf("Please include a key size");//return error message 
        exit(1);//exit
   }
   int length = atoi(argv[1]);// set the key length to the second argument from the user (the atoi function converts this argument into an integer)
   for (x = 0; x < length; x++) {//loop through the key length
	randomNumber = rand() % 27;//randomNumber is set to remainder
	if(randomNumber < 26) {//if its less than 26, and 65 to it and then print the randomCharacter with position randomNumber
	   randomCharacter = 65 + randomNumber;
	   printf("%c", randomCharacter);
	}
	else {
	   printf(" ");//if its not less than 26, then just print a space, increment forward
	}
   }
   printf("\n");//print a newstring

   return 0;//return mn
}
