/*
MIT License

Copyright (c) 2020 ABHISHEK DHITAL

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line
#define MAX_COMMAND_SIZE 255    // The maximum command-line size
#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments
#define EXCLAMATION "!"         //used this for the !history functionality  


pid_t pid_history[15]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
char  command_history[15][100]={"0","0","0","0","0","0","0"
                                ,"0","0","0","0","0","0","0","0"};
//function used to change the current working directory
//we use the chdir function for this purpose
//this function takes in a char pointer to the destination
//and sets the current working directory as specified by the dir pointer
void ch_working_dir(char * dir)
 {
  chdir(dir);
 }


//a function which simply prints out our pid history by accessing the pid_history array
//it stops printing as soon as we reach a 0 or when we reach the end of the array
void showpids()
 {
  int i=0;
  for (i = 0; i <15; i++)
  {
    if(pid_history[i]!=0)
    printf("%d: %d\n",i,pid_history[i]);
  }
 } 


//this function takes the int p as a parameter and then updates the value
//in the empty index in our array
//in case the array is full, the oldest pid pid_history[0] is replaced 
//the new pid will be updated at the last index
//this function is called in the parent process after we fork
void updatepids(int p)
  {
    int i,c=0;
    for (i=0; i<15; i++)
    {
      if (pid_history[i]==0) 
        {
          pid_history[i]=p;
          c=1;
          break; //found an empty index in the array, updated the new pid into that index
        }
    }
      if (c==0)  //did not find empty index to write
      {
       int z=1;
       for (z=1;z<15;z++)
       {
        pid_history[z-1]=pid_history[z];
       if (z==14)
        pid_history[14]=p;
      }
    }
  }


//function to update our command_history array
//takes in a char pointer and updates the string in that address to our array
//this function is called after we execute the entered command in main
void update_History(char* hist)
  {
    int i,c=0;
    for (i=0; i<15; i++)
    {
      if (strcmp(command_history[i],"0")==0)
      { 
        strcpy(command_history[i],hist);
        c=1;
        break;
      }
    }
    if (c==0)
    {
      int z=1;
      for(z=1;z<15;z++)
      {
        strcpy(command_history[z-1],command_history[z]);
        if (z==14)
        {
          //debug
         strcpy(command_history[14],hist);
        //printf("put2 %s on index %d\n",command_history[z],z);
        }
      }
    }
  }


//function to print the content of our command_history array
//in case we have less than 15 commands in our history,
 //it stops printing after the last index with command string
 //called when the command "history" is entered in the msh
void print_History()  //!command not done yet
  {
   int i=0;
    for(i=0;i<15;i++)
     {
      if (strcmp(command_history[i],"0")==0) break;
      char *t=strtok(command_history[i],"\n");
      printf("%d: %s\n",i,t);
      }
  }


//function to print out Command not found in case the entered command 
 //was neither built in nor in the directories
 //called in case our execvp does not replace the ongoing process

void COMMAND_NOT_FOUND(char **tok)  //not implemented yet
{
  printf("%s: Command not found.\n", tok[0]);
} 



//function to start looking for the entered command in current working directory, 
// /usr/bin/, usr/local/bin/, /bin/
int search_working_directory(char **tok) 
  {
    pid_t child_pid=fork();
    if (child_pid==-1) exit(EXIT_FAILURE);

    else if (child_pid==0)
    {
     execvp(tok[0],tok);
     COMMAND_NOT_FOUND(tok);
     fflush(NULL);
     exit(0);
    }
    else
    {
     updatepids(child_pid);//function to update my pid array
     int status;
     (void)waitpid(child_pid,&status,0);
     fflush(NULL); 
   }
  return 1;  
  }




//function to check the entered command
//it starts by checking the command with the built in commands- history, showpids, cd
//if the entered command is not built in, it is forwarded to another function for execvp
int exclusive_check(char** tok, int tok_count)
{
 if(strcmp(tok[0],"cd")==0)
   {
     ch_working_dir(tok[1]);
   }
  //priority: Current working directory,  2./usr/local/bin  3./usr/bin  4./bin 
 else if (strcmp(tok[0],"history")==0 && tok_count<=2)
   {
     print_History();
   }

 else if (strcmp(tok[0],"showpids")==0 && tok_count<=2)
   {
     showpids(); 
   }
 else
   {
     search_working_directory(tok);
   }
   return 1;
}



int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  char** command_pointer=&cmd_str;
  while( 1 )
  {
   // Print out the msh prompt
   printf ("msh> ");

   // Read the command from the commandline.  The
   // maximum command that will be read is MAX_COMMAND_SIZE
   // This while command will wait here until the user
   // inputs something since fgets returns NULL when there
   // is no input
   while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );
   start: ;
   /* Parse input */
   char *token[MAX_NUM_ARGUMENTS];
   
   int   token_count = 0;                                 
                                                           
   // Pointer to point to the token
   // parsed by strsep
   char *arg_ptr;                                         
                                                           
   char *working_str  = strdup( cmd_str );                

   // we are going to move the working_str pointer so
   // keep track of its original value so we can deallocate
   // the correct amount at the end
   char *working_root = working_str;

   // Tokenize the input stringswith whitespace used as the delimiter
   while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
             (token_count<MAX_NUM_ARGUMENTS))
   {
     token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
     if( strlen( token[token_count] ) == 0 )
     {
        token[token_count] = NULL;
     }
        token_count++;
   }
    
   
  
  //in case a blank line is entered in the msh, 
   //the program will go back to the start of the first while loop
   int i,c=0;
   for(i=0;i<token_count;i++)
    if (token[i]!=WHITESPACE && token[i]!=NULL)   c++;
    if (c==0) continue;
    

  //since a blank line was not entered, 
    //we start by checking if the entered command was "exit"
   if(strcmp(token[0],"exit")==0 || strcmp(token[0],"quit")==0) exit(0);
 



  // this block checks whether the first character in the command is '!'
  //if yes, it will tokenize the part after '!' and convert it to integer
  //after doing that, the integer is used to execute that exact index of command in history
  
  char* cc=(char*) malloc (2*sizeof(char));
  char *dummy=strdup(token[0]);
  memcpy(cc,dummy,1);
  char *tn[sizeof(token)];
  int x=0;

//if the first character in the command was ! 
//and the number of tokens was exactly 2 
//(had to do this so that commands like "!123 123" would directly be invalid)
//we need the command to be ! followed by a number without any other number after space
  if(cc[0]=='!' && token_count==2) 
  {
   *tn=strtok(dummy,EXCLAMATION);
   x=atoi(*tn);
    
  //if the number after ! was between 0 and 14,
  //then we can try to execute the command at that particular index  
    if (x>=0 && x<15)
    {
     strcpy(*command_pointer ,command_history[x]);
     goto start;
    }
    else //if the number was not between 0 and 14
    {
     printf("Command not in history\n");
     continue;
    }
  
  }

 //if the first character in the entered command was not ! 
 //and the number of tokens was more than 2,
 //update the history with the given command
 //send the command to exclusive_check() for execution of either msh built in commands or linux commands

  else
  {
   update_History(cmd_str);
   exclusive_check(token, token_count);
  }
  
  
  
  free( working_root );
  }
  return 0;
}


