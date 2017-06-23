/*Author: Tyler Cope
 * Description: This is a text-driven adventure style game where the goal is to get to the end.
 * This particular file reads the created game files and allows playthrough of the game. It also has
 * a time function.
 * Date: 5/8/2017*/
 
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

char* roomList[10] = {"FIRE", "ICE", "WIND", "WATER", "EARTH", "SHADOW", "LIGHT", "SKY", "LAVA", "BURY"};   //Same room names and room types
char* roomType[3] = {"START_ROOM", "END_ROOM", "MID_ROOM"};
pthread_mutex_t mutex;

struct Room{            //Same Room struct as before
    char* name;
    char* type;
    int connections[6];
    int numConnections;    
};

void* getTime();            //I included all the function protoypes up top because I was getting errors. Did this to avoid worrying about order
void writeTime();
void threading();
const char* firstRoom();                  
char* getFolder();
int getRooms(struct Room* room, char roomName[100]);
void gamePlay(char start[50]);
int checkConnection(struct Room* room, char roomName[50]);

char* getFolder(){
  time_t t;                                     //We need the time of the most recent folder
  int i = 0;
  char* buffer = malloc(sizeof(char) * 64);
  DIR* dirToCheck = opendir(".");               //Get the current directory
  struct dirent* dp;                            //Need this pointer
  struct stat statbuffer;
  
  if(dirToCheck != NULL){                       //Referenced from lecture. S_ISDIR checks to make sure it's a directory and we search for what we know the directory will be named
    while(dp = readdir(dirToCheck)){
      if(stat(dp->d_name, &statbuffer) == 0 && S_ISDIR(statbuffer.st_mode) && strncmp(dp->d_name, "copety.rooms", 12)==0) {
        t = statbuffer.st_mtime;
        if(t > i){
          strcpy(buffer, dp->d_name);           //Copy the name into buffer to return the correct folder name
          i = t;
        }
      }
    }
    closedir(dirToCheck);                       //Good practice to close
  }
  
  return buffer;                                //Folder to return
}

const char* firstRoom(){                        //Function to get the starting room for the game
  char* directory = getFolder();
  char* start = malloc(sizeof(char) * 20);
  
  DIR* folder;                                  //Variables to help get the first room
  struct dirent* find;
  
  if((folder = opendir(directory)) != NULL){
    while((find = readdir(folder)) != NULL){
      if(!strcmp(find -> d_name, "..") || !strcmp(find -> d_name, ".")){        //We don't want to include these directories so skip them if they're here
        continue;
      }
      strcpy(start, find -> d_name);            //Get the name of the file into the variable we created and return it
      return start;
      printf("%s", find -> d_name);
      break;
    }
    closedir(folder);                           //Close and free variable
    free(folder);
  }
  else{
    perror("");                                 //If it doesn't exist then return an error. Cast to void* to avoid warning
    return (void *)EXIT_FAILURE;
  }
}

int getRooms(struct Room* room, char roomName[100]){        //Function to get all the room information created by copety.buildrooms
  char folder[100];
  sprintf(folder, getFolder());                             //Make use of the function we created to put the correct folder name into a variable
  char fileName[100];
  sprintf(fileName, "./%s/%s", folder, roomName);           //The whole filepath
  
  FILE* myFile;                         //Create file pointer to use fopen
  myFile = fopen(fileName, "r");        //We declare "r" because we're reading in from the file
  if(myFile == NULL){
    return -1;
  }
  
  room -> name = roomList[0];           //The first room will be named the first name on the list
  room -> numConnections = 0;           //The initial connections are set to 0
  
  char eachLine[100];                   //Variable to read each line
  
  fgets(eachLine, 100, myFile);
  
  char* name = strtok(eachLine, " ");   //We use strtok to break the information apart and store it
  name = strtok(NULL, " ");
  name = strtok(NULL, "\n");            //By putting the newline as the delimiter, we break apart by every new line
  
  int i;
  for(i = 0; i < 10; i++){              //Here we run through the list of names, if it's found in any of the files, set it equal to that place in the roomList array
    if(strcmp(name, roomList[i]) == 0){
      room -> name = roomList[i];
      break;
    }
  }
  
  while(fgets(eachLine, 100, myFile) != NULL){  //Now we're trying to find the connections from the files
    char* searcher = strtok(eachLine, " ");
    if(strcmp(searcher, "CONNECTION") == 0){    //If we hit the word CONNECTION, break apart by space, line, and store the results
      searcher = strtok(NULL, " ");
      searcher = strtok(NULL, "\n");
      for(i = 0; i < 10; i++){                  //If found, store the proper number of connections, determined by i
        if(strcmp(searcher, roomList[i]) == 0){
          room -> connections[room -> numConnections] = i;
          break;
        }
      }
      room -> numConnections++;                 //Increment the total number of connections
    }
    
    else{
      searcher = strtok(NULL, " ");             //If not found search by the room type and set that
      searcher = strtok(NULL, "\n");
      for(i = 0; i < 10; i++){
        if(strcmp(searcher, roomType[i]) == 0){
          room -> type = roomType[i];
          break;
        }
      }
    }
  }
  return 0;                                     //Success
}

int checkConnection(struct Room* room, char roomName[50]){
  int i;
  for(i = 0; i < room -> numConnections; i++){                      //Run through and check if connections between rooms exist, used to check user input
    if(strcmp(roomName, roomList[room -> connections[i]]) == 0){
      return 1;
    }
  }
  return 0;
}

void gamePlay(char start[50]){          //Function to play the game
  int steps = 0;                        //Need to count the number of steps user took
  int ending = 0;                       //A flag to change once the user reaches the end
  int iter;
  char roomName[100];                   //Variables to hold the path the user travels, rooms, and an iterator
  char* roomPath[500];
  struct Room* room = malloc(sizeof(struct Room));
  iter = getRooms(room, start);
  
  do{
    printf("\nCURRENT LOCATION: %s\n", room -> name);       //We want to print this per assignment specs
    printf("POSSIBLE CONNECTIONS:");
    printf(" %s,", roomList[room -> connections[0]]);
    for(iter = 1; iter < room -> numConnections; iter++){   //Run through each connection and print that place if it's in the list
      printf(" %s,", roomList[room -> connections[iter]]);
    }
    printf(". \nWHERE TO? >");          //Same printing per assignment specs
    scanf("%s", roomName);              //Get the user input for where they want to go
    
    if(strcmp(roomName, "time") == 0){  //Check if the user entered time. If so, call our time functions
      threading();
      writeTime();
    }
    else if(checkConnection(room, roomName) == 0){                  //If the function to check if a room is connected returned false (or 0 in this case),
      printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");  //tell user statement about not understanding  
    }
    else{                                   //Otherwise they entered a valid room name and the path is equal to the current step they're on
      roomPath[steps] = room -> name;
      steps++;                              //Only increment steps if they get to this point
      iter = getRooms(room, roomName);      //Use function to get the room info and place it into the variable we created
    }
    
    if(room -> type == "END_ROOM"){         //Check the type to see if they won the game. If so, set the flag to end the game and break out of the loop
      ending = 1;
    } 
  }while(ending != 1);                      //Ending of do-while to break the loop
  int i;
  printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");            //Tell user they won, how many steps they took, and the path they walked
  printf("YOU TOOK %i STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
  for(i = 0; i < steps; i++){          //Loop through and print each spot in the path
    printf("%s\n", roomPath[i]);
  }
  free(room);           //We mallocced this room so we need to free it to avoid leaks
}

void* getTime(){
  FILE* myFile;                             //I referenced a few stack overflow threads for this function: goo.gl/g69qLl and goo.gl/jU2QoT
  myFile = fopen("currentTime.txt", "w+");  //Create and write to the file
  char buffer[100];         
  struct tm *sTm;
  
  time_t now = time (0);
  sTm = gmtime (&now);
  
  strftime (buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", sTm);
  fputs(buffer, myFile);
  fclose(myFile);
     
}

void writeTime(){
  FILE* myFile;
  myFile = fopen("currentTime.txt", "r");   //Read from the file
  char buffer[100];
  
  if(myFile == NULL){               //currentTime.txt must exist
    perror("Not found\n");
  }
  else{
    fgets(buffer, 100, myFile);     //Else read it into buffer and print the time string
    printf("\n%s\n", buffer);
    fclose(myFile);
  }
}

 void threading() {                                 //I used a couple sources as a refernce for this function: https://computing.llnl.gov/tutorials/pthreads/#Mutexes
     pthread_t threaded;                            //http://www.yolinux.com/TUTORIALS/LinuxTutorialPosixThreads.html
     pthread_mutex_init(&mutex, NULL);
     pthread_mutex_lock(&mutex);
     
     int tid = pthread_create(&threaded, NULL, getTime, NULL);
     pthread_mutex_unlock(&mutex);
     pthread_mutex_destroy(&mutex);
     
     usleep(50);
}

int main(){
  char beginningPlace[50];              //Main function plays the game. It copies the first room into a variable and uses that variable to launch the game loop
  strcpy(beginningPlace, firstRoom());
  gamePlay(beginningPlace);    
  return 0;
}



