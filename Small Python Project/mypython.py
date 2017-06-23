#Author: Tyler Cope
#Date: 6/2/2017
#Description: A small Python program that creates and writes to 3 files and gets random integers and prints their product

import random
import string

file1 = open("firstFile", 'w')          #Open up three files for writing, hence the 'w' designation
file2 = open("secondFile", 'w')
file3 = open("thirdFile", 'w')

files = [file1, file2, file3]           #Easy to put files into a list to perform actions on all of them with a python for loop

for file in files:
    randomLetters = ''.join(random.choice(string.ascii_lowercase) for n in range(10))   #Assign random letters to variable to write to file   
    file.write(randomLetters + "\n")                                                    #Write to file, making sure to write the newline character as well
    file.close()                                                                         #Close files because we're done writing
    

file1 = open("firstFile", 'r')      #Open all files again for reading and assign them to a list
file2 = open("secondFile", 'r')     
file3 = open("thirdFile", 'r')

files = [file1, file2, file3]

for file in files:          
    newLine = file.read(10)         #Each file has 11 characters, so you could read 11 here and the output would print some extra spacing; I chose 10 since the assignment specs don't say exactly what to do
    print newLine                   #Print out each line
    
rand1 = random.randint(1,42)        #Get random integers between 1 and 42 and print them
print rand1

rand2 = random.randint(1, 42)
print rand2

print rand1 * rand2                 #Print their product