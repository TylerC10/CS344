/*Author: Tyler Cope
 *Date: 6/5/2017
 *Description: This file connects to otp_enc_d and asks it to perform encryption
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include<sys/stat.h>
#define SIZE 70000

 //Note: I gave very detailed comments about the file in otp_enc.c. This file will have more cursory comments
 //since most things are similar. I will expand where needed

void fileSend(char *filename, int socketFD, int length){
    FILE *f = fopen(filename, "r");
	char buffer[SIZE];	
	bzero(buffer, SIZE);
	int byteNum;

	
	while((length = fread(buffer, sizeof(char), SIZE, f)) > 0){     //read and send the file
		if((byteNum = send(socketFD, buffer, length, 0)) < 0){
			break;
		}
		bzero(buffer, SIZE);
	}
	if(byteNum == SIZE){                                        //We know if it matches our number then that's the end
		send(socketFD, "0", 1, 0);                                //Send this to terminate
	}
	fclose(f);
	return;
}

int main(int argc, char* argv[]){
    int socketFD;                       //All needed variables
    int portNumber;
    int value;
    int status;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    char buffer[SIZE];
    char authenticate[] = "dec";
    value = 1;
    bzero(buffer, sizeof(buffer));
    
    if(argc != 4){                  //must have correct number of arguments
        printf("Incorrect number of arguments.");
        exit(0);
    }
    
    portNumber = atoi(argv[3]);                 //Set port and socket. Send error if socket doesn't open
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    
    if(socketFD < 0){
        error("Error opening socket.", 1);
    }
    
    serverHostInfo = gethostbyname("localhost");    //Convert machine name to address
    
    if(serverHostInfo == NULL){
        fprintf(stderr, "Error, no such host.");
        exit(0);
    }
    
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));        //Trying to use that function
    
    bzero((char *) &serverAddress, sizeof(serverAddress));      //Set up socket and all values
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    bcopy((char *)serverHostInfo -> h_addr, (char *)&serverAddress.sin_addr.s_addr, serverHostInfo -> h_length);    //Copy in the address (action from client.c)
    
    if(connect(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0){   //Error in connection
        perror("Error connecting.");
        exit(1);
    }
    
    write(socketFD, authenticate, sizeof(authenticate));        //Proper authenticattion to prevent connection from forbidden files
    read(socketFD, buffer, sizeof(buffer));
    
    if(strcmp(buffer, "dec_d") != 0){
        fprintf(stderr, "Error, can't use");
        exit(2);
    }
    int key = open(argv[2], O_RDONLY);                  //Get text, key, and the sizes of both
    int keySize = lseek(key, 0, SEEK_END);
    
    int text = open(argv[1], O_RDONLY);
    int textSize = lseek(text, 0 , SEEK_END);
    
    if(textSize > keySize){                         //Make sure key is proper
        fprintf(stderr, "Error, need longer key.");
        exit(1);
    }
    
    bzero(buffer, sizeof(buffer));              //Send files
    fileSend(argv[1], socketFD, textSize);
    fileSend(argv[2], socketFD, keySize);
    
    status = read(socketFD, buffer, sizeof(buffer) - 1); //Make sure it went okay
    
    if(status < 0){
        perror("Error reading from socket.");
        exit(1);
    }    
    printf("%s\n", buffer);             //Clean up and close
    close(socketFD);
    
    return 0;
}































