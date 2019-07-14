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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <termios.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <poll.h>
#include <zlib.h>

//Child process ID
pid_t PID;

//Variables for signal checking
int checkClose = 0;
int checkKill = 0;

/*Variables for control characters*/
char CR = '\r';
char LF = '\n';

//Create variable to store the pass in argument for the one option
int portNum = 0;

//Variables for the sockets
int sockfd;
int newsockfd;

//Create variables to store the flag for the two options
int portSig = 0;
int compressSig = 0;

//initialize the streams for compression process
z_stream clientServer;
z_stream serverClient;

/*Create two pipes, one for each direction of communication and checks for error*/
/*Resources : https://linux.die.net/man/2/pipe*/
int pipeShellTerminal[2];
int pipeTerminalShell[2];

void preCompress()
{
    //From server to the client
    //use deflateInit here
    //Resources : https://www.zlib.net/zlib_how.html
    serverClient.zalloc = Z_NULL;
    serverClient.zfree = Z_NULL;
    serverClient.opaque = Z_NULL;
    
    int check = 0;
    check = deflateInit(&serverClient, Z_DEFAULT_COMPRESSION);
    if (check != Z_OK)
    {
        fprintf(stderr, "deflateInit() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void preDecompress()
{
    //From server to the client
    //use inflateInit here
    //Resources : https://www.zlib.net/zlib_how.html
    clientServer.zalloc = Z_NULL;
    clientServer.zfree = Z_NULL;
    clientServer.opaque = Z_NULL;
    
    int check = 0;
    check = inflateInit(&clientServer);
    if (check != Z_OK)
    {
        fprintf(stderr, "inflateInit() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void socketError(int check)
{
    if(check < 0)
    {
        fprintf(stderr, "socket() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void acceptError(int check)
{
    if(check < 0)
    {
        fprintf(stderr, "accept() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void signalError(sig_t check)
{
    if (check == SIG_ERR)
    {
        fprintf(stderr, "signal() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void forkError(int forkCheck)
{
    if (forkCheck == -1)
    {
        fprintf(stderr, "Error in duplicating the calling process to the new process");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
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

void pipeError(int pipeCheck)
{
    if (pipeCheck == -1)
    {
        fprintf(stderr, "Error creating the pipe.");
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

void writeError(int checkWrite)
{
    if (checkWrite == -1)
    {
        fprintf(stderr, "write() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void readBytesError(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "read() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void inflateInitError(int check)
{
    if (check != Z_OK)
    {
        fprintf(stderr, "inflateInit() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void deflateInitError(int check)
{
    if (check != Z_OK)
    {
        fprintf(stderr, "deflateInit() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void portError()
{
    //error message about the mandatory port number
    fprintf(stderr, "--port option are needed and failed reasons: %s\n", strerror(errno));
    fprintf(stderr, "Proper usage : lab1b-client --port=port_number");
    exit(1);
}

//From project 1A code
void SIGPIPE_Handler()
{
     fprintf(stderr, "The handler function does not has major functions.");
    exit(0);
}

//From project 1A code
void SIGINT_Handler()
{
    int check = 0;
    check = kill(PID, SIGINT);
    if (check == -1)
    {
        fprintf(stderr, "kill() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

int decompression(char buffer [], int readBytes)
{
    //compress the traffic from client to traffic
    //declare a compression buffer with size 256
    int decompressBufSize = 2000;
    char decompressedBuffer[decompressBufSize];
    
    memcpy(decompressedBuffer, buffer, readBytes);
    
    //Based from resources : https://www.zlib.net/zlib_how.html
    clientServer.avail_in = readBytes;
    clientServer.next_in = (unsigned char *) decompressedBuffer;
    clientServer.avail_out = decompressBufSize;
    clientServer.next_out = (unsigned char *) buffer;
    
    int check = 0;
    
    do
    {
        check = inflate(&clientServer, Z_SYNC_FLUSH);
        inflateInitError(check);
    }while (clientServer.avail_in > 0);
    
    int number = decompressBufSize - clientServer.avail_out;
    
    return number;
}

int compression(char buffer [], int readBytes)
{
    //compress the traffic from client to traffic
    //declare a compression buffer with size 256
    int compressBufSize = 2000;
    char compressedBuffer[compressBufSize];
    
    memcpy(compressedBuffer, buffer, readBytes);
    
    //Based from resources : https://www.zlib.net/zlib_how.html
    serverClient.avail_in = readBytes;
    serverClient.next_in = (unsigned char *)compressedBuffer;
    serverClient.avail_out = compressBufSize;
    serverClient.next_out = (unsigned char *)buffer;
    
    int check = 0;
    
    do
    {
        check = deflate(&serverClient, Z_SYNC_FLUSH);
        deflateInitError(check);
    }while (clientServer.avail_in > 0);
    
    int number = compressBufSize - serverClient.avail_out;
    
    return number;
}

//From project 1A code
void killChild()
{
    checkKill = kill(PID, SIGINT);
    if (checkKill == -1)
    {
        fprintf(stderr, "kill() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void readFromKeyboard()
{
    char buffer[2000];
    int readBytes = 0;
    readBytes = read(newsockfd, buffer, sizeof(char)*2000);
    readBytesError(readBytes);
    
    
    if (compressSig == 1)
    {
        readBytes = decompression(buffer, readBytes);
    }
    
    char currentChar;
    int check = 0;
    
    //code model based from project A
    for (int i = 0; i < readBytes; i++)
    {
        currentChar = buffer[i];
        
        if (currentChar == '\4')
        {
            checkClose = close(pipeTerminalShell[1]);
            closeError(checkClose);
        }
        else if (currentChar == '\3')
        {
            killChild();
        }
        else if (currentChar == LF || currentChar == CR)
        {
            check = write(pipeTerminalShell[1], &LF, 1);
            writeError(check);
        }
        else
        {
            check = write(pipeTerminalShell[1], &buffer, 1);
            writeError(check);
        }
    }
}

void readFromShell()
{
    char buffer[2000];
    int readBytes = 0;
    readBytes = read(pipeShellTerminal[0], buffer, sizeof(char)*2000);
    readBytesError(readBytes);
    
    if (compressSig == 1)
    {
        readBytes = compression(buffer, readBytes);
        
    }
    
    //write to socket
    write(newsockfd, buffer, readBytes);
}

void atexitError(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "atexit() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

//From project 1A code
void inputRestore()
{
    /*Collect the shell's exit status or a related function to await the process' completion and capture the return status*/
    /*Ressource : http://www.tutorialspoint.com/unix_system_calls/waitpid.htm*/
    //int checkSetAttr = 0;
    int checkWait = 0;
    int checkExited = 0;
    int state;
    
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

void shellExitStatus()
{
    int exitNumber = 0;
    exitNumber = atexit(inputRestore);
    atexitError(exitNumber);
}

void parentProcess()
{
    /*Create an array of two poll fd structures, one poll describing the keyboard*/
    /*One poll describing pipe returning output from the shell*/
    /*Based from this resources: http://www.unixguide.net/unix/programming/2.1.2.shtml*/
    struct pollfd poll_fds[2];
    poll_fds[0].fd = newsockfd;
    poll_fds[1].fd = pipeShellTerminal[0];
    
    /*Make both pipes wait for either POLLIN, POLLHUP, POLLERR*/
    poll_fds[0].events = POLLIN | POLLHUP | POLLERR;
    poll_fds[1].events = POLLIN | POLLHUP | POLLERR;
    
    //Exit and restore
    shellExitStatus();
    
    //checking for poll
    int pollCheck = 0;
    
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

//srever handle SIGPIPE from the shell pipes(the shell exits)
void signalHandler()
{
    sig_t checkSignal;
    checkSignal = signal(SIGPIPE, SIGPIPE_Handler);
    signalError(checkSignal);
    
    checkSignal = signal(SIGINT, SIGINT_Handler);
    signalError(checkSignal);
}

/*Based from resources of long_options:
 https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html*/
int main(int argc, char **argv)
{
    //set up two options
    static struct option long_options[] = {
        {"port", required_argument, NULL, 'p'},
        {"compress", no_argument, NULL, 'c'},
        {0, 0, 0, 0}
    };
    
    //Create variable to store the number returned from options
    int optionIndex = 0;
    
    while ((optionIndex = getopt_long(argc, argv, "p:c", long_options, NULL)) != -1)
    {
        switch(optionIndex)
        {
            case 'p':
                portSig = 1;
                //store the port number for the switching process
                portNum = atoi(optarg);
                
                break;
            case 'c':
                compressSig = 1;
                //prepare for the compression for the keyboard input
                preCompress();
                //prepare for the decompression for the socket
                preDecompress();
                break;
            default:
                //print the unrecognized error message and exit with code 1
                fprintf(stderr, "Unrecognized argument: %s\n. Proper usage : lab1b-server --port=command_line_parameter --compress\n", strerror(optionIndex));
                exit(1);
        }
    }
    
    if (portSig != 1)
    {
        portError();
    }
    
    //Code for setting up the socket for server
    //Based from resources : https://www.tutorialspoint.com/unix_sockets/socket_server_example.htm
    socklen_t clientLength = 0;
    
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    
    //call to the socket function to set up the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    socketError(sockfd);
    
    //Initialize socket structure
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portNum);
    
    //Bind the host address using the bind() call.
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stderr, "bind() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
    
    listen(sockfd, 5);
    clientLength = sizeof(cli_addr);
    
    //accept the actual connection from the client
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clientLength);
    acceptError(newsockfd);
    
    //fork process (from project 1A)
    //set up the two pipes that are needed
    int pipeCheck;
    pipeCheck = pipe(pipeShellTerminal);
    pipeError(pipeCheck);
    
    pipe(pipeTerminalShell);
    pipeError(pipeCheck);
    
    //handler for the SIGPIPE and SIGINT
    signalHandler();
    
    //Fork to create a new process and exec a shell (Project 1A)
    //Resource: https://www.gnu.org/software/libc/manual/html_node/Creating-a-Pipe.html
    PID = fork();
    
    if (PID == -1)
    {
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
        /*Based from sample code*/
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
    
    exit(0);
}
