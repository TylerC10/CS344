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


void fileSend(char* filename, int socketFD, int length){
    FILE* f = fopen(filename, "r");         //Set up a file to read
    char buffer[SIZE];
    bzero(buffer, SIZE);
    int numBytes;
    
    while((length = fread(buffer, sizeof(char), SIZE, f)) > 0){ //While file is being read
        if((numBytes = send(socketFD, buffer, length, 0)) < 0){ //Send the file
            break;
        }
        bzero(buffer, SIZE);  //Clear buffer
    }
    
    if(numBytes == SIZE){           //Once we hit the end, send null to end request
        send(socketFD, "0", 1, 0);
    }
    fclose(f);              //Clean up
    return;
}

int main(int argc, char* argv[]){ //NOTE: For this function, I borrowed from client.c (both variable names and some structure)
    int socketFD;           //Variables to enable the process
    int portNumber;
    int value;
    int status;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    char buffer[SIZE];
    char authenticate[] = "e";
    value = 1;
    bzero(buffer, sizeof(buffer));
    
    if(argc != 4){      //Need 4 arguments; error if not equal to that number
        printf("Incorrect number of arguments.");
        exit(0);
    }
    
    portNumber = atoi(argv[3]); //Last argument is the user entered port number
    socketFD = socket(AF_INET, SOCK_STREAM, 0); //Set up the socket
    
    if(socketFD < 0){                       //Error if unable to
        error("Error opening socket.");
    }
    
    serverHostInfo = gethostbyname("localhost");    //Convert machine name into address
    
    if(serverHostInfo == NULL){                     //Need the server. Output error if unable to reach
        fprintf(stderr, "Error, no such host.");
        exit(0);
    }
    
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));    //Trying to use that function again
    
    bzero((char *) &serverAddress, sizeof(serverAddress));      //Clear server address
    serverAddress.sin_family = AF_INET;     //Create network capable socket
    bcopy((char *)serverHostInfo -> h_addr, (char *)&serverAddress.sin_addr.s_addr, serverHostInfo -> h_length); //Copy in the address
    serverAddress.sin_port = htons(portNumber);     //Store port number
    
    if(connect(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0){  //Error if unable to connect
        perror("Error connecting.");
        exit(1);
    }
    
    write(socketFD, authenticate, sizeof(authenticate));    //Authentication
    read(socketFD, buffer, sizeof(buffer));
    
    if(strcmp(buffer, "e") != 0){
        fprintf(stderr, "Error, can't use.");
        exit(2);
    }
    int key = open(argv[2], O_RDONLY);          //Get entered key and its size
    int keySize = lseek(key, 0, SEEK_END);
    
    int text = open(argv[1], O_RDONLY);         //Get the text and the size of the text
    int textSize = lseek(text, 0 , SEEK_END);
    
    if(textSize > keySize){                     //If text is larger than the key, tell user they need a bigger key
        fprintf(stderr, "Error, need longer key.");
        exit(1);
    }
    
    int checker = open(argv[1], 'r');           //Check second argument for invalid characters by looping throuh each one; error if a character isn't valid
    while(read(checker, buffer, 1) != 0){
        if(isspace(buffer[0]) || isalpha(buffer[0])){
            
        }
        else{
            fprintf(stderr, "Invalid characters in %s\n", argv[1]);
            exit(1);
        }
    }
    
    bzero(buffer, sizeof(buffer));          //Clear the buffer
    
    fileSend(argv[1], socketFD, textSize);  //Send text and key
    fileSend(argv[2], socketFD, keySize);
    
    status = read(socketFD, buffer, sizeof(buffer) - 1);    
    
    if(status < 0){                             //Make sure it was successful
        perror("Error from reading socket.");
        exit(1);
    }
    printf("%s\n", buffer);                     //Print and cleanup
    close(socketFD);
    
    return 0;
}























