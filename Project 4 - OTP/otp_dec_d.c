/*Author: Tyler Cope
 *Date: 6/5/2017
 *Description: This file does all the encoding per assignment specs
 */
 
 //Note: I gave very detailed comments about the file in otp_enc_d.c. This file will have more cursory comments
 //since most things are similar. I will expand where needed

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#define SIZE 70000                //Easy to assign this to different char arrays

void decryptMessage(char message[], char key[], int length);    //Function prototypes to avoid errors
char intToCharacter(int check);
int characterToInt(char character);

void decryptMessage(char message[], char key[], int length){
    int i;                  //Variables to setup decryption
	int textNumber;
	int keyNumber;
	int decryptNumber;
	length = (strlen(message)-1);
    
    for (i = 0; i < length; i++){                   //Loop through and use key to decrypt
		textNumber = characterToInt(message[i]);
		keyNumber = characterToInt(key[i]);
		decryptNumber = (textNumber - keyNumber) % 27;

		if(decryptNumber < 0){
			decryptNumber += 27;
		}
		message[i] = intToCharacter(decryptNumber);

	}
	message[i] = '\0';
    return;
}

char intToCharacter(int check){                         //Function to convert integers to acceptable characters
    char *characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

	if (check < 0 || 27 < check){
		return 'a';
	}
	return characters[check];
}

int characterToInt(char character){                 //Convert characters to integers
    int i;
    char *characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

	for (i = 0; i < 27; i++){		
		if (characters[i] == character){
			return i;
		}
	}
    return -1;
}

int main(int argc, char* argv[]){
    int value = 1;                  //All needed variables
    socklen_t sizeOfClientInfo;
    int socketFD;
    int portNumber;
    int newSocketFD;
    char buffer[SIZE];
    struct sockaddr_in clientAddress;
    struct sockaddr_in serverAddress;
    pid_t pid; 
    
    if(argc != 2){              //Correct number of arguments needed
        fprintf(stderr, "Incorrect number of arguments\n");
        exit(EXIT_FAILURE);
    }
    
    socketFD = socket(AF_INET, SOCK_STREAM, 0); //Set up socket
    if(socketFD < 0){
        perror("Error opening socket.");
        exit(1);
    }
    
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));
    
    bzero((char *) &serverAddress, sizeof(serverAddress)); //Clear serverAddress for use and set needed values for socket
    portNumber = atoi(argv[1]);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    

    
    if(bind(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0){  //Enable socket to listen
        perror("Error on binding");
        exit(1);
    }
    
    listen(socketFD, 5);
    
    while(1){
        sizeOfClientInfo = sizeof(clientAddress);
        newSocketFD = accept(socketFD, (struct sockaddr *) &clientAddress, &sizeOfClientInfo);  //new socket to use
        if(newSocketFD < 0){
            perror("Error on accept.");
            exit(1);
        }
        pid = fork();                   //Fork process and exit with error on failure
        if(pid < 0){
            perror("Error on fork.");
            exit(1);
        }
        if(pid == 0){
            bzero(buffer, sizeof(buffer));
            int iter;
            int next;
            char* key;
            int numBytes;
            int bytesRead;
            numBytes = sizeof(buffer);
            bytesRead = 0;
            next = 0;
            
            read(newSocketFD, buffer, sizeof(buffer) - 1);  //Authenticate to make sure call is to correct file
            if(strcmp(buffer, "dec") != 0){
                char output[] = "not valid";
                write(newSocketFD, output, sizeof(output));
                exit(2);
            }
            else{
                char output[] = "dec_d";
                write(newSocketFD, output, sizeof(output));
            }
            bzero(buffer, sizeof(buffer));
            char* buffHelper = buffer;
            
            while(1){
                bytesRead = read(newSocketFD, buffHelper, numBytes);
                if(numBytes == 0){
                    break;
                }
                
                for(iter = 0; iter < bytesRead; iter++){    //Go until hit second new line
                    if(buffer[iter] == '\n'){
                        ++next;
                        if(next == 1){
                            key = buffer + iter + 1;
                        }
                    }
                }
                if(next == 2){
                    break;
                }
                numBytes = numBytes - bytesRead;
                buffHelper = buffHelper + bytesRead;
            }
            char message[SIZE];                 //Set message and get ready to send
            bzero(message, sizeof(message));
            strncpy(message, buffer, key-buffer);
            decryptMessage(message, key, strlen(message)); //Decrypt it, write it, and cleanup everything
            write(newSocketFD, message, sizeof(message));
        }
        close(newSocketFD);
    }
    close(socketFD);
    return 0;
}


























