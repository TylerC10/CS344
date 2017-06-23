/*Author: Tyler Cope
 *Date: 6/5/2017
 *Description: This file does all the encoding per assignment specs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#define SIZE 70000            //Easy to assign this to different char arrays

void encryptMessage(char message[], char key[], int length);    //Function prototypes to avoid errors
char intToCharacter(int check);
int characterToInt(char character);

void encryptMessage(char message[], char key[], int length){    
	int i;                          //Variables to help setup encryption
	int textNumber;
	int keyNumber;
	int encryptNumber;
	length = (strlen(message)-1);

	for (i = 0; i < length; i++){
		textNumber = characterToInt(message[i]);        //Loop through swapping out letters using key
		keyNumber = characterToInt(key[i]);

		encryptNumber = (textNumber + keyNumber) % 27;
		message[i] = intToCharacter(encryptNumber);	    //Actual swap	
	}
	message[i] = '\0';                                  //Null terminator
	return;
}

char intToCharacter(int check){    
	static const char *characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";      //These are all the acceptable characters

	if (check < 0 || 27 < check){	                //If number is invalid, just return lowercase char	
		return 'a';
	}
	return characters[check];                       //Else, return valid character
}

int characterToInt (char character){
    int i;
	static const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";       //Again, all acceptable characters	
	
	for(i = 0; i < 27; i++){            //Loop through and if we find the correct character, return that position
		if (character == chars[i]) {
			return i;
		}
	}
	return -1;                  //Operation failed
}

int main(int argc, char* argv[]){   //NOTE: I borrowed a good bit from server.c (including variable names and some structure)
    int value = 1;                  //This is setup for the function provided here: goo.gl/0gfNR2. Suggested usage on specs page. It has given me mixed results
    socklen_t sizeOfClientInfo;     //Holds the size of the client
    int socketFD;                   //Used to listen
    int portNumber;                 //The port number
    int newSocketFD;                //What we'll use for child process
    char buffer[SIZE];
    struct sockaddr_in clientAddress;   //Structs to hold addresses
    struct sockaddr_in serverAddress;
    pid_t pid;                      //Process ID for fork
    
    
    if(argc != 2){                  //Not equal to two arguments will return an error
        fprintf(stderr, "Incorrect number of arguments");
        exit(EXIT_FAILURE);
    }
    
    socketFD = socket(AF_INET, SOCK_STREAM, 0); //Create the socket
    if(socketFD < 0){                           //Error if not created correctly
        perror("Error opening socket.");
        exit(1);
    }
  
    
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));    //Suggested function from the earlier link
    
    bzero((char *) &serverAddress, sizeof(serverAddress));                  //Clear server addresse for use; I tried using memset but I kept getting weird errors
    portNumber = atoi(argv[1]);                             //Port number from the second argument
    serverAddress.sin_family = AF_INET;                     //Network capable socket
    serverAddress.sin_port = htons(portNumber);             //Save port number
    serverAddress.sin_addr.s_addr = INADDR_ANY;             //Any address is allowed to connect

    
    if(bind(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0){  //Enable listening
        perror("Error on binding");
        exit(1);
    }
    
    listen(socketFD, 5);        //Equivalent of turning it on
    
    while(1){
        sizeOfClientInfo = sizeof(clientAddress);
        newSocketFD = accept(socketFD, (struct sockaddr *) &clientAddress, &sizeOfClientInfo);  //Need to be able to accept sent calls
        if(newSocketFD < 0){            //Error if unable to accept
            perror("Error on accept");
            exit(1);
        }
        pid = fork();                   //Error if fork doesn't work properly
        if(pid < 0){
            perror("Error on fork");
            exit(1);
        }
        if(pid == 0){                       //This is how we'll handle all our connections
            bzero(buffer, sizeof(buffer));  //Variables to help with the process
            int iter;
            int next;
            char* key;
            int numBytes;
            int bytesRead;
            numBytes = sizeof(buffer);
            bytesRead = 0;
            next = 0;
            
            read(newSocketFD, buffer, sizeof(buffer) - 1);      //Authenticate and exit with error if unable to
            if(strcmp(buffer, "e") != 0){
                char output[] = "not valid";
                write(newSocketFD, output, sizeof(output));
                exit(2);
            }
            else{
                char output[] = "e";
                write(newSocketFD, output, sizeof(output));
            }
            bzero(buffer, sizeof(buffer));                  //Clear buffer and set a char pointer to help keep track 
            char* buffHelper = buffer;
            
            while(1){
                bytesRead = read(newSocketFD, buffHelper, numBytes);    //Get number of read bytes
                if(numBytes == 0){                      //No need to read anymore, break here if bytes is 0
                    break;
                }
                
                for(iter = 0; iter < bytesRead; iter++){        //Iterate to find new lines to know when to break
                    if(buffer[iter] == '\n'){
                        ++next;                                 //Increment lines
                        if(next == 1){
                            key = buffer + iter + 1;            //Have to remember to add 1 to get proper key (null terminator is at the end)
                        }
                    }
                }
                if(next == 2){                  //We know we're done when we get to this point 
                    break;
                }
                numBytes = numBytes - bytesRead;        //Subtract the number of bytes read from total number of bytes
                buffHelper = buffHelper + bytesRead;    //Keep track of where we are             
            }
            char message[SIZE];                         //Set up message to be sent
            bzero(message, sizeof(message));
            strncpy(message, buffer, key-buffer);       //Copy to message
            encryptMessage(message, key, strlen(message)); //Encrypt it using earlier function
            write(newSocketFD, message, sizeof(message));   //Write to socket
        }
        close(newSocketFD);                             //Clean up by closing sockets
    }
    close(socketFD);
    return 0;
}
















