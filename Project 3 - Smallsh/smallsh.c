#include "smallsh.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>


void showStatus(int status){
    if(WIFEXITED(status)){          //Taken from lecture
        printf("exit value %i\n", WEXITSTATUS(status));
    } 
    else{
        printf("terminated by signal %i\n", status);
    }
}

void catchSIGINT(int signo){                //Code to catch and display message for signals
	char* message = "\nCaught SIGINT.\n";
	write(STDOUT_FILENO, message, 38);
}

void catchSigStop(int signal){
	char* message = "\nCaught SIGTSTP.\n";
	write(STDOUT_FILENO, message, 25);
	exit(0);
}

int main(){             //All the variables
    int fileIn = -1;          
    int fileOut = -1;          
    int fgProcess;     
    int status = 0;
    int cpid;
    char userInput[400];
    char* argument[400];
    char* token;
    char* inputFile = NULL;  
    char* outputFile = NULL;    
    struct sigaction action = {0};
    struct sigaction action2 = {0};
    
    
    action.sa_handler = SIG_IGN;            //Set up actions and handlers for ctrl+z and ctrl+c functionality
    action.sa_flags = 0;                    //Taken from lecture
    sigfillset(&(action.sa_mask));
    sigaction(SIGINT, &action, NULL);
    
    action2.sa_handler = catchSigStop;
    action2.sa_flags = 0;
    sigfillset(&(action2.sa_mask));
    sigaction(SIGTSTP, &action2, NULL);
    
    


    
    while(1){
        fgProcess = 1;      

        printf(": ");       //Prompt and flushing to avoid errors
        fflush(stdout);     

        
        if(fgets(userInput, 400, stdin) == NULL){   //Reached the end
            return 0;
        }

        int helper = 0;                     
        token = strtok(userInput, " \n");           //Parse the user input for instructions
        while(token != NULL){
            if(strcmp(token, "<") == 0){           //This means input file     
                token = strtok(NULL, " \n");        //Parse and store
                inputFile = strdup(token);
                token = strtok(NULL, " \n");
            } 
            else if(strcmp(token, ">") == 0){       //This is an output file so perform same action and save to outputFile
                token = strtok(NULL, " \n");
                outputFile = strdup(token);
                token = strtok(NULL, " \n");
            } 
            else if(strcmp(token, "&") == 0){       //Background process
                fgProcess = 0;
                break;
            } 
            else{
                argument[helper] = strdup(token);   //Save pointer to instruction in argument array at the position of helper
                token = strtok(NULL, " \n");
                ++helper;                           //Increment our helper to get next place
            }
        }
        argument[helper] = NULL;        

        if(argument[0] == NULL || *(argument[0]) == '#'){            //If no input or pound sign indicating comment, do nothing and move on
            ;
        }
        else if(strcmp(argument[0], "cd") == 0){            //User wants to change directories. If there is no argument after cd command, go to home directory
            if(argument[1] == NULL){                        //Else, go to directory stated by user
                chdir(getenv("HOME"));
            } 
            else{
                chdir(argument[1]);
            }
        } 
        else if(strcmp(argument[0], "status") == 0){        //Print the status
            showStatus(status);
        }
        
        else if (strcmp(argument[0], "exit") == 0){     //User wants to quit
            exit(0);
        } 
        else{
            cpid = fork();
            switch(cpid){
            case 0:                     //Fork a new process and run switch on pid(idea taken from lecture)
                if(fgProcess){          //Foreground can be interrupted if need be
                    action.sa_handler = SIG_DFL;
                    sigaction(SIGINT, &action, NULL);
                }                
                if(inputFile != NULL){    //If unable to open and read file, inform user             
                    fileIn = open(inputFile, O_RDONLY);
                    if(fileIn == -1){
                        printf("smallsh: cannot open %s for input\n", inputFile);
                        fflush(stdout);
                        _Exit(1);
                    }
                    if(dup2(fileIn, 0) == -1){
                        perror("dup2");
                        _Exit(1);
                    }
                    close(fileIn);      //Need to close since it was opened
                } 
                else if(!fgProcess){
                    fileIn = open("/dev/null", O_RDONLY);       //Redirect to null device to ignore information
                    if(fileIn == -1){
                        perror("open");
                        _Exit(1);
                    }
                    if(dup2(fileIn, 0) == -1){
                        perror("dup2");
                        _Exit(1);
                    }
                }
                
                if(outputFile != NULL){                         //Inform user if output file cannot be opened
                    fileOut = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0744);
                    if(fileOut == -1) {
                        printf("smallsh: cannot open %s\n", outputFile);
                        fflush(stdout);
                        _Exit(1);
                    }
                    if(dup2(fileOut, 1) == -1){
                        perror("dup2");
                        _Exit(1);
                    }
                    close(fileOut);
                }
                
                if(execvp(argument[0], argument)){         //Command is not recognized so inform the user and change exit status
                    printf("smallsh: Sorry, command \"%s\" is not a valid command\n", argument[0]);
                    fflush(stdout);
                    _Exit(1);
                }
                break;
                
            case -1:    
                perror("fork");
                status = 1;
                break;
                
            default:    
                if (fgProcess){                         //Wait for foreground to finish (that's why we pass specific cpid)
                    waitpid(cpid, &status, 0);
                } 
                else{                    
                    printf("Background PID: %i\n", cpid);
                    break;
                }
            }

        }
        
        for(int i = 0; argument[i] != NULL; i++){
            free(argument[i]);
        }
           
        free(inputFile);            //Some cleanup
        inputFile = NULL;
        free(outputFile);
        outputFile = NULL;

        
        cpid = waitpid(-1, &status, WNOHANG);           //Check if any process has completed; Returns 0 if no terminated processes
        while(cpid > 0){
            printf("background process, %i, is done: ", cpid);
            showStatus(status);
            cpid = waitpid(-1, &status, WNOHANG);
        }
    }

    return 0;
}

