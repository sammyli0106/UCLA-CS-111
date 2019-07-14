/*
 NAME: Sum Yi Li
 EMAIL: sammyli0106@gmail.com
 ID: 505146702
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

/*Based from resource of file descriptor:
http://web.cs.ucla.edu/~harryxu/courses/111/spring19/ProjectGuide/fd_juggling.html*/

/*Define the input redirection function*/
void inputRedirect(char *input_name)
{
  /*check if the input file arg is specify*/
  /*otherwise optarg is default to zero*/
  if (input_name != NULL)
    {
      int infd = open(input_name, O_RDONLY);
      if (infd >= 0)
	{
	  close(0);
	  dup2(infd, 0);
	  close(infd);
	}
      else
	{
	  /*unable to open input file, report failure, exit with return code of 2*/
	  /*report the file name*/
	  fprintf(stderr, "Error opening file: %s\n", input_name);
	  /*reason that it cannot be opened*/
	  fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
	  exit(2);
	}
    }
}

/*Based from resource of file descriptor:                                                       
http://web.cs.ucla.edu/~harryxu/courses/111/spring19/ProjectGuide/fd_juggling.html*/

/*Define the output redirection function*/
void outputRedirect(char *output_name)
{
  /*check if the output file arg is specify*/
  if (output_name != NULL)
    {
      int outfd = creat(output_name, 0666);
      if (outfd >= 0)
	{
	  close(1);
	  dup2(outfd, 1);
	  close(outfd);
	}
      else
	{
	  /*unable to create output file, report failure, exit with return code of 3*/
	  fprintf(stderr, "Error creating file: %s\n", output_name);
	  /*Failure reasons*/
	  fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
	  exit(3);
	}
    }
}

/*Based from resources of pointers:
http://www.cs.fsu.edu/~myers/cgs4406/notes/pointers.html*/
void segfault ()
{
  /*set a pointer to null and stores through the null pointer */
  char *segPtr = NULL;
  /*pick a random letter*/
  *segPtr = 'e';
}

void handler ()
 {
  /*logs an error message */
  write(2, "Catching segmentation fault error. Program exits.\n", 44);
  exit(4);
 }

/*Based from resources of SIGSEGV handler: 
http://www.alexonlinux.com/how-to-handle-sigsegv-but-also-generate-core-dump*/
void catch ()
{
  /*call the SIGSEGV handler*/
  signal(SIGSEGV, handler);
}


/*Based from resources of long_options:
https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html*/

int main(int argc, char **argv)
{
  /*Process all arguments and store the results in variables*/
  /*Define all the options*/
  static struct option long_options[] = {
					 {"input", required_argument, 0, 'i'},
					 {"output", required_argument, 0, 'o'},
					 {"segfault", no_argument, 0, 'f'},
					 {"catch", no_argument, 0, 'c'},
					 {0, 0, 0, 0}
  };

  /*create a variable to store the option index*/
  int optionIndex = 0;
  char *input_name = NULL;
  char *output_name = NULL;
  int segflag = 0;
  int catchflag = 0;

  //Options follow by colon has a required paramter, not specify, cause program to fail
  while ((optionIndex = getopt_long(argc, argv, "i:o:fc", long_options, NULL)) != -1)
    {
      switch(optionIndex)
	{
	case 'i':
	  /*call the input redirection function*/
	  input_name = optarg;
	  inputRedirect(input_name);
	  break;
	case 'o':
	  output_name = optarg;
	  outputRedirect(output_name);
	  break;
	case 'f':
	  /*create a flag for the segfault option*/
	  segflag = 1;
	  break;
	case 'c':
	  /*create a flag for the catch option*/
	  catchflag = 1;
	  break;
	default:
	  /*print error messages and exit with 1*/
	  fprintf(stderr, "Unrecognized argument: %s\n. Proper usage : lab0 --input=filename --output=filename\n",strerror(optionIndex));
	  exit(1);
	}
    }

  if (catchflag == 1)
    {
      /*call the SIGSEGV handler here*/
      catch();
    }
  
  if (segflag == 1)
    {
      /*call a subroutine here*/
      segfault(); 
    }

  /*No segfault, then copy the stdin to stdout, after all the checking*/
  /*File descriptor for read() function*/
  int fdRead = 0;
  /*File descriptor for write() function*/
  int fdWrite = 1;
  /*checking variables for read and write functions*/
  int check;
  int check2;
  /*Create a buffer for reading and writing a character*/
  char *buffer = (char*) malloc(sizeof(char));
  if (buffer == NULL)
    {
      fprintf(stderr, "Memory allocation fail");
      free(buffer);
      exit(-1);
    }

  /*read the first character and check for errors*/
  check = read(fdRead, buffer, 1);
  if (check == -1)
    {
      fprintf(stderr, "read() failed: %s\n", strerror(errno));
      exit(-1);
    }

  while (check > 0)
    {
      /*write the char*/
      check2 = write(fdWrite, buffer, 1);
      if (check2 == -1)
	{
	  fprintf(stderr, "write() failed: %s\n", strerror(errno));
	  exit(-1);
	}
      /*read the next char*/
      check = read(fdRead, buffer, 1);
      if (check == -1)
      {
	fprintf(stderr, "read() failed: %s\n", strerror(errno));
         exit(-1);
      }
    }

  free(buffer);
  
  /*Successful at the end here*/
  exit(0);
}

