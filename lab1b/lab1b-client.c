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


//Declear terminal variable
struct termios oldAttribute;

//Create variables to store the pass in argument for the two options
int portNum = 0;
char* filename;

//Declare variable to store the socket file descriptor
int socketFd;

//Read, write, log descriptor
int readFd = 0;
int writeFd = 1;
int logFd = 0;

//three options flags
int compressSig = 0;
int portSig = 0;
int logSig = 0;

//Child process ID
pid_t PID;

//initalize the streams for compression process
z_stream clientServer;
z_stream serverClient;

/*Variables for control characters*/
char CR = '\r';
char LF = '\n';
char CRLF[2] = {'\r', '\n'};

//constant variables for the log format output
char sent_first_word[15] = "SENT ";
char sent_second_word[15] = " bytes: ";
char received_first_word[15] = "RECEIVED ";
char received_second_word[15] = " bytes: ";

void serverError(struct hostent *server);

void serverAddress(struct sockaddr_in serv_addr, struct hostent *server);

//Function that set up socket
//Based from resources: http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/client.c
void createSocket()
{
    //Create variable for the socket address and the server address
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0)
    {
        fprintf(stderr, "socket() failed");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
    
    //set up the server
    server = gethostbyname("localhost");
    serverError(server);
    
    //set up the address for the provided server
    serverAddress(serv_addr, server);
}

void connectClient(struct sockaddr_in serv_addr);

//Function that set up the server address
//Based from resources: http://www.mathcs.richmond.edu/~barnett/cs332/lectures/Client_Server_Example_ho.txt
void serverAddress(struct sockaddr_in serv_addr, struct hostent *server)
{
    //construct the server address structure
    //zero out the structure
    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    //adress family
    serv_addr.sin_family = AF_INET;
    
    //make a copy of the server address
    memcpy((char*) &serv_addr.sin_addr.s_addr, (char*) server->h_addr, server->h_length);
    serv_addr.sin_port = htons(portNum);
    
    //connect the client to the server through the address
    connectClient(serv_addr);
}

void connectClient(struct sockaddr_in serv_addr)
{
    if (connect(socketFd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stderr, "connect() failed");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void serverError(struct hostent *server)
{
    if (server == NULL)
    {
        fprintf(stderr, "gethostbyname() failed");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void closeFd()
{
    close(socketFd);
    close(logFd);
}

void inputRestore()
{
    /*Collect the shell's exit status or a related function to await the process' completion and capture the return status*/
    /*Ressource : http://www.tutorialspoint.com/unix_system_calls/waitpid.htm*/
    int checkSetAttr = 0;
    
    checkSetAttr = tcsetattr(STDIN_FILENO, TCSANOW, &oldAttribute);
    if (checkSetAttr == -1)
    {
        fprintf(stderr, "tcsetattr() failed and reasons are %s\n", strerror(errno));
        exit(1);
    }
    
    
    closeFd();
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

void pollError(int pollCheck)
{
    if (pollCheck == -1)
    {
        fprintf(stderr, "poll() has failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void readBytesError(int readBytes)
{
    if (readBytes == -1)
    {
        fprintf(stderr, "read() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
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

void writeError(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "write() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
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

void compressLog(char compressedBuffer [])
{
    int check = 0;
    //write the SENT word
    check = write(logFd, sent_first_word, strlen(sent_first_word));
    writeError(check);
    
    //declare variable to print the number of bytes in formatted output
    char bytesNum[15];
    int length = 256 - clientServer.avail_out;
    sprintf(bytesNum, "%d", length);
    
    //write the number of bytes that are being read
    check = write(logFd, bytesNum, strlen(bytesNum));
    writeError(check);
    
    //write the bytes word
    check = write(logFd, sent_second_word, strlen(sent_second_word));
    writeError(check);
    
    //write the actual stuffs that are being sent
    check = write(logFd, compressedBuffer, length);
    writeError(check);
    
    //write the last character of the string which is a newline
    check = write(logFd, "\n", 1);
    writeError(check);

}

void regularLog(char buffer [], int readBytes)
{
    //repeat the same log steps, except use the normal buffer instead the compressed
    int check = 0;
    
    //write the SENT word
    check = write(logFd, sent_first_word, strlen(sent_first_word));
    writeError(check);
    
    //declare variable to print the number of bytes sent in formatted output
    char bytesNum[15];
    sprintf(bytesNum, "%d", readBytes);
    
    //write the number of bytes that are being sent
    check = write(logFd, bytesNum, strlen(bytesNum));
    writeError(check);
    
    //write the bytes word
    check = write(logFd, sent_second_word, strlen(sent_second_word));
    writeError(check);
    
    //write the actual stuffs that are being sent
    check = write(logFd, buffer, readBytes);
    writeError(check);
    
    //write the last character of the string which is a newline
    check = write(logFd, "\n", 1);
    writeError(check);
}

void ByteByByte(char buffer [], int readBytes)
{
    char currentChar;
    int check = 0;
    
    for (int i = 0; i < readBytes; i++)
    {
        currentChar = buffer[i];
        
        if (currentChar == '\r' || currentChar == '\n')
        {
            check = write(writeFd, "\r\n", 2);
            writeError(check);
        }
        else
        {
            check = write(writeFd, &currentChar, 1);
            writeError(check);
        }
    }
}

void readFromKeyboard()
{
    /*Do a larger suggested read, then process the number of available bytes*/
    char buffer[256];
    int readBytes = 0;
    readBytes = read(readFd, buffer, sizeof(char)*256);
    readBytesError(readBytes);
    
    ByteByByte(buffer, readBytes);
    
    if (compressSig == 1)
    {
        
        //compress the traffic from client to traffic
        //declare a compression buffer with size 256
        int compressBufSize = 256;
        char compressedBuffer[compressBufSize];
        
        //Based from resources : https://www.zlib.net/zlib_how.html
        clientServer.avail_in = readBytes;
        clientServer.next_in = (unsigned char *)buffer;
        clientServer.avail_out = compressBufSize;
        clientServer.next_out = (unsigned char *)compressedBuffer;
        
        int check = 0;
        
        do
        {
            check = deflate(&clientServer, Z_SYNC_FLUSH);
            deflateInitError(check);
        }while (clientServer.avail_in > 0);
        
        //pass the output to the socket
        write(socketFd, compressedBuffer, compressBufSize - clientServer.avail_out);
        
        if (logSig == 1)
        {
            compressLog(compressedBuffer);
        }
    }
    else
    {
        //no need to compress the traffic
        //output per byte to stdout and to socket
        //compress signal is off here
        
        write(socketFd, buffer, readBytes);
        
        //could do the stuffs required when the log signal is on here
        if (logSig == 1)
        {
            regularLog(buffer, readBytes);
        }
    }
}

void socketLog(char buffer [], int readBytes)
{
    //similar the log version from the no compressed version
    int check = 0;
    
    //write the RECEIVED word
    check = write(logFd, received_first_word, strlen(received_first_word));
    writeError(check);
    
    //declare the variable to print the number of bytes received in formatted output
    char bytesNum[15];
    sprintf(bytesNum, "%d", readBytes);
    
    //write the number of bytes that are being received
    check = write(logFd, bytesNum, strlen(bytesNum));
    writeError(check);
    
    //write the bytes word
    check = write(logFd, received_second_word, strlen(received_second_word));
    writeError(check);
    
    //write the actual stuffs that are being received
    check = write(logFd, buffer, readBytes);
    writeError(check);
    
    //write the last character of the string which is a newline
    check = write(logFd, "\n", 1);
    writeError(check);
}

void No_Byte_Exit(int readBytes)
{
    if (readBytes == 0)
    {
        exit(0);
    }
}

void ByteByByteSock(char buffer [], int readBytes)
{
    char currentChar;
    int check = 0;
    
    for (int i = 0; i < readBytes; i++)
    {
        currentChar = buffer[i];
        
        if (currentChar == CR || currentChar == LF)
        {
            check = write(writeFd, &CRLF, 2 * sizeof(char));
            writeError(check);
        }
        else
        {
            check = write(writeFd, &currentChar, 1);
            writeError(check);
        }
    }
}

void readFromSocket()
{
    //similar to the format from reading from the keyboard
    /*Do a larger suggested read, then process the number of available bytes*/
    char buffer[256];
    int readBytes = 0;
    //read from the socket
    readBytes = read(socketFd, buffer, sizeof(char)*256);
    readBytesError(readBytes);
    
    No_Byte_Exit(readBytes);
    
    ByteByByteSock(buffer, readBytes);
    
    if (compressSig == 1)
    {
        //this time, make sure use a large enough buffer for the decompressed data
        //decompress the traffic from client over the network
        //declare a decompression buffer with large size: 2000
        int decompressBufSize = 2000;
        char decompressedBuffer[decompressBufSize];
        
        //Based from resources : https://www.zlib.net/zlib_how.html
        serverClient.avail_in = readBytes;
        serverClient.next_in = (unsigned char *) buffer;
        serverClient.avail_out = decompressBufSize;
        serverClient.next_out = (unsigned char *) decompressedBuffer;
        
        int check = 0;
        
        do
        {
            check = inflate(&serverClient, Z_SYNC_FLUSH);
            deflateInitError(check);
        }while (serverClient.avail_in > 0);
        
        
        //pass the output to the stdout for the shell
        write(writeFd, decompressedBuffer, decompressBufSize - serverClient.avail_out);
    }
    
    //No difference between the two log file for with and without the compressed option
    if (logSig == 1)
    {
        socketLog(buffer, readBytes);
    }
}

//Based from resources : https://www.zlib.net/zlib_how.html
void preCompress()
{
    //From client to the server
    //use deflateInit here
    clientServer.zalloc = Z_NULL;
    clientServer.zfree = Z_NULL;
    clientServer.opaque = Z_NULL;
    
    int check = 0;
    check = deflateInit(&clientServer, Z_DEFAULT_COMPRESSION);
    if (check != Z_OK)
    {
        fprintf(stderr, "deflateInit() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

//Based from resources : https://www.zlib.net/zlib_how.html
void preDecompress()
{
    //From server to the client
    //use inflateInit here
    serverClient.zalloc = Z_NULL;
    serverClient.zfree = Z_NULL;
    serverClient.opaque = Z_NULL;
    
    int check = 0;
    check = inflateInit(&serverClient);
    if (check != Z_OK)
    {
        fprintf(stderr, "inflateInit() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void logCheck(char* filename)
{
    logFd = creat(filename, 0666);
    if (logFd == -1)
    {
        fprintf(stderr, "create() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void cleanUp()
{
    
    //should free up the allocated state information referenced by stream
    //all the pending output is discarded, and unprocessed input is ignored
    if (inflateEnd(&serverClient) == Z_STREAM_ERROR)
    {
        fprintf(stderr, "inflateEnd() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }
    
    if (deflateEnd(&clientServer))
    {
        fprintf(stderr, "deflateEnd() failed");
        fprintf(stderr, "Reasons for the failure: %s\n", strerror(errno));
        exit(1);
    }

}
void portError()
{
    //output an error message about the mandatory port number
    fprintf(stderr, "--port option are needed and failed reasons: %s\n", strerror(errno));
    fprintf(stderr, "Proper usage : lab1b-client --port=port_number");
    exit(1);
}

/*Based from resources of long_options:
 https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html*/
int main(int argc, char **argv)
{
    //set up three options
    static struct option long_options[] = {
        {"port", required_argument, NULL, 'p'},
        {"log", required_argument, NULL, 'l'},
        {"compress", no_argument, NULL, 'c'},
        {0, 0, 0, 0}
    };
    
    //Create variable to store the number returned from options
    int optionIndex = 0;
    
    while ((optionIndex = getopt_long(argc, argv, "p:l:c", long_options, NULL)) != -1)
    {
        switch(optionIndex)
        {
            case 'p':
                portSig = 1;
                //store the port number in order to switch to
                portNum = atoi(optarg);
                break;
            case 'l':
                logSig = 1;
                //store the filename
                filename = optarg;
                
                //call the create function to test the log file
                logCheck(filename);
                
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
                fprintf(stderr, "Unrecognized argument: %s\n. Proper usage : lab1b-client --port=command_line_parameter --log=filename --compress\n", strerror(optionIndex));
                exit(1);
        }
    }
    
    //Check if the port number is being provided, it is mandatory
    //If it is not 1, some other numbers, then the port number is not being provided
    if (portSig != 1)
    {
        portError();
    }
    
    //set up non-canonical terminal like the one previous project
    terminal();
    
    //set up the sockets for passing input between the client program and the server
    createSocket();
    
    //set up the poll for the keyboard and the socket
    /*Based from this resources: http://www.unixguide.net/unix/programming/2.1.2.shtml*/
    struct pollfd poll_fds[2];
    //reading from the keyboard
    poll_fds[0].fd = readFd;
    //reading from the socket
    poll_fds[1].fd = socketFd;
    
    /*Make both pipes wait for either POLLIN, POLLHUP, POLLERR*/
    //same as previously
    //check if need to add POLLRDHUP
    poll_fds[0].events = POLLIN | POLLHUP | POLLERR;
    poll_fds[1].events = POLLIN | POLLHUP | POLLERR;
    
    //send the input from the keyboard to the socket
    //send the input from the socket to the display
    int pollCheck = 0;
    
    while (1)
    {
        pollCheck = poll(poll_fds, 2, -1);
        
        if (pollCheck > 0)
        {
            //no error in poll
            
            //read from keyboard and check whether the standard input is available
            if (poll_fds[0].revents & POLLIN)
            {
                readFromKeyboard();
            }
            
            //read from the socket
            if (poll_fds[1].revents & POLLIN)
            {
                readFromSocket();
            }
            
            //possible error for poll to equal POLLUP or POLLERR
            if ((poll_fds[1].revents & POLLHUP) || (poll_fds[1].revents & POLLERR))
            {
                exit(0);
            }
            
        }
        else
        {
            //there is error in poll and should output a error message
            pollError(pollCheck);
        }
    }
    
    //could do the deflateEnd and the inflateEnd here before exit
    //inflateEnd clean up all
    if (compressSig == 1)
    {
        cleanUp();
    }
    
    exit(0);
}
