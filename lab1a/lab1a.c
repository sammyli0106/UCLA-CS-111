/*
 Name: Sum Yi Li
 Email: sammyli0106@gmail.com 
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
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>

/*Variables for the terminal function*/
pid_t PID;
struct termios oldAttribute;

/*Variables for signal checking*/
int shellSignal = 0;
int checkClose = 0;

/*Variables for control characters*/
char CR = '\r';
char LF = '\n';
char CRLF[2] = {'\r', '\n'};

/*Create two pipes, one for each direction of communication and checks for error*/
/*Resources : https://linux.die.net/man/2/pipe*/
int pipeShellTerminal[2];
int pipeTerminalShell[2];

/*Handler functions for the signal function when shell has shut down*/
void handler()
{
    /*Output a message and just exit with 0*/
    fprintf(stderr, "The handler function does not has major functions.");
    exit(0);
}

void inputRestore()
{
    /*Collect the shell's exit status or a related function to await the process' completion and capture the return status*/
    /*Ressource : http://www.tutorialspoint.com/unix_system_calls/waitpid.htm*/
    int checkSetAttr = 0;
    int checkWait = 0;
    int checkExited = 0;
    int state;
    
    checkSetAttr = tcsetattr(STDIN_FILENO, TCSANOW, &oldAttribute);
    if (checkSetAttr == -1)
    {
        fprintf(stderr, "tcsetattr() failed and reasons are %s\n", strerror(errno));
        exit(1);
    }
    
    if (shellSignal == 1)
    {
        checkWait = waitpid(PID, &state, 0);
        if (checkWait == -1)
        {
            fprintf(stderr, "waitpid() failed and reasons are %s\n", strerror(errno));
            exit(1);
        }
        /*WIFEXITED return a non-zero if exit normally*/
        checkExited = WIFEXITED(state);
        if (checkExited != 0)
        {
            /*Store the lower and higher bit's of the shell exit status*/
            int higherExit = WEXITSTATUS(state);
            int lowerExit = WTERMSIG(state);
            /*report to stderr about the process completion and shell's exit status*/
            fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", lowerExit, higherExit);
            exit(0);
        }
    }
}

void terminal()
{
    /*Manipulate the terminal attributes*/
    /*First, save the old attribute before modifing through termios*/
    /*oldAttribute would be declared as a global variable that help for restoring*/
    /*Resource : http://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html*/
    tcgetattr(STDIN_FILENO, &oldAttribute);
    atexit(inputRestore);
    
    /*check whether calling process is associated with a terminal*/
    int checkDevice = STDIN_FILENO;
    struct termios newAttribute;
    
    if (!isatty(checkDevice))
    {
        fprintf(stderr, "The standard input is not associated with a terminal.");
        exit(1);
    }
    
    /*Save the copy before modifying the new attributes*/
    tcgetattr(STDIN_FILENO, &newAttribute);
    
    /*make a copy of the terminal with the following changes*/
    /*only lower 7 bits*/
    newAttribute.c_iflag = ISTRIP;
    newAttribute.c_oflag = 0;
    newAttribute.c_lflag = 0;
    
    /*The mode would be non-canonical input mode with no echo*/
    /*Using the TCSANOW options to set the modes immediately on start-up*/
    /*Checking for errors also*/
    int noEchoModeError;
    noEchoModeError = tcsetattr(STDIN_FILENO, TCSANOW, &newAttribute);
    if (noEchoModeError < 0)
    {
        fprintf(stderr, "Error of transferring into non-canonical input mode with no echo.");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void writeError(int checkWrite);
void closeError(int checkClose);

void readFromKeyboard()
{
    /*Do a larger suggested read, then process the number of available bytes*/
    /*Based from resources : */
    char buffer[256];
    int readBytes = 0;
    readBytes = read(0, buffer, sizeof(char)*256);
    if (readBytes == -1)
    {
        fprintf(stderr, "read() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
    
    char currentChar;
    int checkClose = 0;
    int checkKill = 0;
    int checkWrite = 0;
    
    /*Loop through the bytes inside the buffer*/
    for (int i = 0; i < readBytes; ++i)
    {
        currentChar = buffer[i];
        
        if (currentChar == '\4')
        {
            /* ^D, it refers to '\004'*/
            /*close the pipe to the shell, but continue to process input from shell*/
            /*there may still be output in transit from the shell*/
            checkClose = close(pipeTerminalShell[1]);
            closeError(checkClose);
        }
        else if (currentChar == '\3')
        {
            /* ^C, it refers to '\003'*/
            /*Read a ^C from the keyboard, should kill to send a SIGINT to the shell process*/
            checkKill = kill(PID, SIGINT);
            if (checkKill == -1)
            {
                fprintf(stderr, "kill() failed");
                fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
                exit(1);
            }
        }
        else if (currentChar == LF || currentChar == CR)
        {
            /*<cr> or <lf> echo as <cr><lf>, but go to the shell as <lf>*/
            checkWrite = write(1, &CRLF, sizeof(char)*2);
            writeError(checkWrite);
            
            checkWrite = write(pipeTerminalShell[1], &LF, 1);
            writeError(checkWrite);
        }
        else
        {
            /*default mode : write the received characters back out to the display to be processed*/
            checkWrite = write(1, &currentChar, 1);
            writeError(checkWrite);
            checkWrite = write(pipeTerminalShell[1], &buffer, 1);
            writeError(checkWrite);
        }
    }
}

void readFromShell()
{
    char buffer[256];
    int readBytes = 0;
    /*Read input from the shell pipe and write it to stdout*/
    readBytes = read(pipeShellTerminal[0], buffer, sizeof(char)*256);
    if (readBytes == -1)
    {
        fprintf(stderr, "read() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
    
    char currentChar;
    int checkWrite;
    
    /*Use a loop to go through the readBytes again*/
    /*If input receive an <lf> from the shell, should print to the screen as <cr><lf>*/
    for (int i = 0; i < readBytes; ++i)
    {
        currentChar = buffer[i];
        if (currentChar == LF)
        {
            checkWrite = write(1, &CRLF, sizeof(char)*2);
            writeError(checkWrite);
        }
        else
        {
            /*default mode: write as normal*/
            checkWrite = write(1, &currentChar, 1);
            writeError(checkWrite);
        }
    }
}

void pollError(int pollCheck);

void parentProcess()
{
    /*Create an array of two poll fd structures, one poll describing the keyboard*/
    /*One poll describing pipe returning output from the shell*/
    /*Based from this resources: http://www.unixguide.net/unix/programming/2.1.2.shtml*/
    struct pollfd poll_fds[2];
    poll_fds[0].fd = 0;
    poll_fds[1].fd = pipeShellTerminal[0];
    
    /*Make both pipes wait for either POLLIN, POLLHUP, POLLERR*/
    poll_fds[0].events = POLLIN | POLLHUP | POLLERR;
    poll_fds[1].events = POLLIN | POLLHUP | POLLERR;

    int pollCheck = 0;
    
    /*loop that calls poll and only read from fd if it has pending input, report in revents*/
    while (1)
    {
        pollCheck = poll(poll_fds, 2, 0);
        pollError(pollCheck);
        
            if ((poll_fds[0].revents & POLLIN))
            {
                /*read from the keyboard, poll_fds[0].revents & POLLIN*/
                readFromKeyboard();
            }
            
            if ((poll_fds[1].revents & POLLIN))
            {
                /*read from shell pollfd, poll_fds[1].revents & POLLIN*/
                readFromShell();
            }
        
            if ((poll_fds[1].revents & POLLHUP) || (poll_fds[1].revents & POLLERR))
            {
                /*poll_fds[1].revents equals to POLLHUP or POLLERR*/
                exit(0);
            }
        
    }
}

void pollError(int pollCheck)
{
    if (pollCheck == -1)
    {
        fprintf(stderr, "poll() has failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void readError(int checkRead)
{
    if (checkRead == -1)
    {
        fprintf(stderr, "read() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void writeError(int checkWrite)
{
    if (checkWrite == -1)
    {
        fprintf(stderr, "write() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void pipeError(int pipeCheck)
{
    if (pipeCheck == -1)
    {
        fprintf(stderr, "Error creating the pipe.");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void forkError(int forkCheck)
{
    if (forkCheck < -1)
    {
        fprintf(stderr, "Error in duplicating the calling process to the new process");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void dupError(int dupCheck)
{
    if (dupCheck == -1)
    {
        fprintf(stderr, "Error in duplicating the input from the terminal");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void closeError(int checkClose)
{
    if (checkClose == -1)
    {
        fprintf(stderr, "Error in closing the pipe.");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void shellOff()
{
    char buffer;
    int checkRead = 0;
    int checkWrite = 0;
    
    checkRead = read(0, &buffer, 1);
    readError(checkRead);
    
    while (checkRead > 0)
    {
        if (buffer == LF || buffer == CR)
        {
            /*Write to <cr> and <lf> correpsondingly*/
            checkWrite = write(1, &CR, 1);
            writeError(checkWrite);
            checkWrite = write(1, &LF, 1);
            writeError(checkWrite);
            
        }
        else if (buffer == '\4')
        {
            /*End of Transmission*/
            /*Exit out of the program*/
            break;
        }
        else
        {
            checkWrite = write(1, &buffer, 1);
            writeError(checkWrite);
        }
        
        checkRead = read(0, &buffer, 1);
    }
}

int main(int argc, char **argv)
{
    /*Define the shell options first*/
    static struct option long_options[] = {
        {"shell", no_argument, NULL, 's'},
        {0, 0, 0, 0}
    };
    
    /*Create variable to store the option index*/
    int optionIndex = 0;
    
    /*Since there is no required parameter, so no need to use :*/
    while ((optionIndex = getopt_long(argc, argv, "", long_options, NULL)) != -1)
    {
        switch(optionIndex)
        {
            case 's':
                /*call the shell option related functions*/
                shellSignal = 1;
                break;
            default:
                /*Print error message and exit with code 1*/
                fprintf(stderr, "Unrecognized argument: %s\n. Proper usage : lab1a --shell\n", strerror(optionIndex));
                exit(1);
        }
    }
    
    
    /*Create two pipes, one for each direction of communication and checks for error*/
    /*Declare as global variables, but check errors here*/
    /*Resources : https://www.gnu.org/software/libc/manual/html_node/Creating-a-Pipe.html*/
    int pipeCheck;
    pipeCheck = pipe(pipeShellTerminal);
    pipeError(pipeCheck);
    
    pipe(pipeTerminalShell);
    pipeError(pipeCheck);
    
    //Manipulate the terminal
    terminal();
    
    /*Shutdown processing : Need to handle SIGPIPE from a write to the pipe to the shell*/
    /*signal returns SIG_ERR is there is error*/
    sig_t checkSignal;
    
    if (shellSignal == 1)
    {
        checkSignal = signal(SIGPIPE, handler);
        if (checkSignal == SIG_ERR)
        {
            fprintf(stderr, "signal() failed: %s\n", strerror(errno));
            exit(1);
        }
        
        /*Fork to create a new process and exec a shell*/
        /*Resource : https://www.gnu.org/software/libc/manual/html_node/Creating-a-Pipe.html*/
        
        PID = fork();
        
        if (PID == -1)
        {
            //error message
            forkError(PID);
        }
        else if (PID == 0)
        {
            /*Child Process is being called*/
            int checkClose = 0;
            /*Close the pipe from terminal to shell since there is no writing, just reading*/
            checkClose = close(pipeTerminalShell[1]);
            closeError(checkClose);
            
            /*Close the pipe from shell to terminal since there is no reading, just writing*/
            checkClose = close(pipeShellTerminal[0]);
            closeError(checkClose);
            
            /*Check for dup2*/
            int dup2Check = 0;
            /*Standard input is a pipe from the terminal process*/
            dup2Check = dup2(pipeTerminalShell[0], STDIN_FILENO);
            dupError(dup2Check);
            
            /*Standard output and standard error are a pipe to the terminal process*/
            dup2Check = dup2(pipeShellTerminal[1], STDOUT_FILENO);
            dupError(dup2Check);
            
            dup2Check = dup2(pipeShellTerminal[1], STDERR_FILENO);
            dupError(dup2Check);
            
            /*Close the pipe in the reverse direction this time*/
            /*Close reading, just writing this time*/
            checkClose = close(pipeTerminalShell[0]);
            closeError(checkClose);
            
            /*Close writing, just reading this time*/
            checkClose = close(pipeShellTerminal[1]);
            closeError(checkClose);
            
            /*exec a shell (/bin/bash) with no arguments other than the name*/
            /*Declare the argument and the path name variables*/
            /*From sample code*/
            
            char *execvp_argv[2];
            char execvp_filename[] = "/bin/bash";
            execvp_argv[0] = execvp_filename;
            execvp_argv[1] = NULL;
            if (execvp(execvp_filename, execvp_argv) == -1)
            {
                fprintf(stderr, "execvp() failed!\n");
                fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
                exit(1);
            }
        }
        else
        {
            /*Parent Process is being called*/
            /*First close the two pipes*/
            
            checkClose = close(pipeTerminalShell[0]);
            closeError(checkClose);
            
            checkClose = close(pipeShellTerminal[1]);
            closeError(checkClose);
            
            /*call a parent process function after closing the two pipes*/
            parentProcess();
        }
        
    }
    
    //No shell flag
    shellOff();
    
    exit(0);
    
}

