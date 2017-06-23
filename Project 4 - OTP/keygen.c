/*Author: Tyler Cope
 *Date: 6/5/2017
 *Description: This file generates the key that will be used for encryption/decryption
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[]){
    srand(time(NULL));              //Seeding for random
    
    int random;
    int keylength;
    int i;    
    
    if (argc < 2){              //Need two arguments. Tell user
        fprintf(stderr, "Error, key is too short.");
        exit(0);
    }
    
    keylength = atoi(argv[1]);  //Set the length of the key equal to the second argument provided
    
    for(i = 0; i < keylength; i++){                         //Pick random capital letters. Put in space character where needed
        random = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[rand()%27];
        if(random == 'A' + 26){
            random = ' ';
        }
        fprintf(stdout, "%c", random);          //Write to stdout
    }
    
    fprintf(stdout, "\n");      //Need to include this
    
   return 0;                    //Success
}