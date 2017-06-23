/*Author: Tyler Cope
 * Description: This is a text-driven adventure style game where the goal is to get to the end.
 * This particular file generates rooms for the game.
 * Date: 5/8/2017*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

char foldName[100];   //Will hold the name of the directory that the files write to
char* roomList[10] = {"FIRE", "ICE", "WIND", "WATER", "EARTH", "SHADOW", "LIGHT", "SKY", "LAVA", "BURY"};   //List of room names
char* roomType[3] = {"START_ROOM", "END_ROOM", "MID_ROOM"};   //Use this to assign a room type per assignment specs

struct Room{            //Struct to hold all the information about a room
    char* name;
    char* type;
    int connections[6];
    int numConnections;    
};

char *makeFolder();
void makeRooms(struct Room rooms[7]);
void makeConnection(int room, struct Room rooms[7]);
void mix();
void putFiles();


char* makeFolder(){    //This is the directory that the rooms will be written to. Specs say include process id so that's the getpid
  
  sprintf(foldName, "copety.rooms.%d", getpid());
  mkdir(foldName, 0700);  //Need to set permissions for folder. I forgot to do this at first and kept getting seg fault errors
  return foldName; 
}

void makeRooms(struct Room roomHold[7]){    //Need seven rooms
  int i;

  for(i = 0; i < 7; i++) {
    roomHold[i].numConnections = 0;     //Initialize all connections to be 0
  }


  int foo;                      //Some garbage variable to iterate. I used singular letters at first but it was much easier to assign a word I easily recognized
  for(foo = 0; foo < 7; foo++){ //because I had several single letter iterators
    int checker, bar, k;
    checker = 1;
    while(checker){
        bar = rand() % 10;      //Assign names for rooms
        checker = 0;
        for (k = 0; k < 7; k++){
            if(roomHold[k].name == roomList[bar]){
                checker = 1;
            }
        }
    }
  roomHold[foo].name = roomList[bar];

    
  if(foo == 0){                          //Assign the type of room per assignment specs
    roomHold[foo].type = roomType[0];
  }
  else if(foo == 1){
    roomHold[foo].type = roomType[1];
  }
  else{
    roomHold[foo].type = roomType[2];
  }

  int totalConnections = rand() % 4 + 3; //Correct number range of random connections
  while(roomHold[foo].numConnections < totalConnections){
    makeConnection(foo, roomHold);
  }
 }
}

void makeConnection(int room, struct Room rooms[7]){
  int randConnect, checker;
  checker = 1;

  while(checker){               //Loop through until we don't get a room
    checker = 0;
    randConnect = rand() % 7;
    if(randConnect == room){
      checker = 1;
    }
  int i;
  for(i = 0; i <rooms[room].numConnections; i++){
    if(rooms[room].connections[i] == randConnect){
      checker = 1;
    }
  }
}

  rooms[room].connections[rooms[room].numConnections] = randConnect;          //Set the number of connections
  rooms[room].numConnections++;       //Increase total number of connections
  rooms[randConnect].connections[rooms[randConnect].numConnections] = room;  
  rooms[randConnect].numConnections++;
}


void mix(int holder[]){             //This is to ensure the path isn't the same each time
  int i, ending;
  ending = 10;
  for (i = 0; i < ending; i++){     //Mix up spots in the array
  int j;
  j = i + (rand() % 9);
  int place = holder[j];
  holder[j] = holder[i];
  holder[i] = place;
  }
}

void putFiles(struct Room rooms[7], char* folder){
  chdir(folder);                //Need to go into the proper folder that we got earlier
  int i;
  for(i = 0; i < 7; i++){       //Make a file with the proper name for each room
  FILE* myFile = fopen(rooms[i].name, "a"); 
  fprintf(myFile, "ROOM NAME: %s\n", rooms[i].name);

  int j;
  for(j = 0; j < rooms[i].numConnections; j++){     //Print the connections for each file. We use j+ 1 because we want to print connections starting at 1, not 0
    fprintf(myFile, "CONNECTION %d: %s\n", j + 1, rooms[rooms[i].connections[j]].name);
  }

  fprintf(myFile, "ROOM TYPE: %s\n", rooms[i].type);    //Get the type and print it to the room

  fclose(myFile);       //Opened a file so good practice to close the file
 }
}

int main(){
  srand(time(NULL));                //Seed the randomization
  char* roomsFolder = makeFolder(); //Call function to create the folder
  struct Room rooms[7];
  makeRooms(rooms);             //Create the rooms
  
  putFiles(rooms, roomsFolder); //Write all the files using the room information we created
  
  return 0;  
}



