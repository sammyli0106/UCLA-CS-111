/*
 Name: Sum Yi Li
 Email: sammyli0106@gmail.com
 ID: 505146702
 */

#include <ctype.h>
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
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <mraa.h>
#include <math.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <termios.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <poll.h>

//define constant values
#define temp_sensor_init 1
#define button_init 60
#define buffer_size 60
#define null_byte '\0'
#define tab '\t'
#define blank ' '

//variable that store the passed argument
//default to one second
int period_arg = 1;
//default to store one char with letter F
char scale_arg = 'F';
//default to zero
FILE* log_file_name = 0;
FILE* read_file = 0;
struct timeval clock_time;
struct tm* current_time;
time_t follow_up_time = 0;

//shut down version of the time
struct timeval shut_down_clock_time;
struct tm* shut_down_current_time;

//flag for the options
int scale_flag = 0;
int period_flag = 0;
int log_flag = 0;

//Declare the required variable for the temperature sensor
mraa_aio_context temperature_sensor;
mraa_gpio_context button;
mraa_result_t check;

//integer check variable for error message
int check_error = 0;

//variable to indicate output both time and temperature
//this is equal to the stop flag
int report_flag = 1;
float current_temperature = 0;

//set up a buffer and store the information
char buffer[buffer_size];

//set up shut down buffer
char shutdown_buffer[buffer_size];

//command buffer
char *command_buffer;
char *copy_buffer;
int position_index = 0;

//check for occurence of period and log
char *first_period;
char *first_log;

char *constant1;
char *constant2;

int sock_fd;
int logfile_fd;
struct sockaddr_in serv_addr;
struct hostent* server;
int port_number = 0;
char* host_name = NULL;
int id_flag = 0;
char* id_arg;
int host_flag = 0;
char id_buffer[60];
char read_buffer[60];

void check_connect_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "connect() failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void fopen_error(FILE* check)
{
    if (check == NULL)
    {
        fprintf(stderr, "fopen() failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void mraa_aio_init_error(mraa_aio_context temperature_sensor)
{
    if (temperature_sensor == NULL)
    {
        fprintf(stderr, "mraa_aio_init() failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void mraa_gpio_init_error(mraa_gpio_context button)
{
    if (button == NULL)
    {
        fprintf(stderr, "mraa_gpio_init() failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void mraa_gpio_dir_error(mraa_result_t check)
{
    if (check != MRAA_SUCCESS)
    {
        fprintf(stderr, "mraa_gpio_dir() failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void gettimeofday_error(int check_error)
{
    if (check_error == -1)
    {
        fprintf(stderr, "gettimeofday() failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void localtime_error(struct tm* current_time)
{
    if (current_time == NULL)
    {
        fprintf(stderr, "localtime() failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void poll_error(int check_error)
{
    if (check_error == -1)
    {
        fprintf(stderr, "poll() failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void malloc_error(char *command_buffer)
{
    if (command_buffer == NULL)
    {
        fprintf(stderr, "malloc() failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

//Based from resources : http://wiki.seeedstudio.com/Grove-Temperature_Sensor_V1.2/
double record_temperature_func()
{
    //double check the mraa read function here
    int temp_read = mraa_aio_read(temperature_sensor);
    int thermistor_read = 4275;
    float scalar = 100000.0;
    //calculate the temperature
    float temperature = 1023.0 / ((float) temp_read) - 1.0;
    temperature *= scalar;
    float celsius = 1.0 / (log(temperature/scalar)/thermistor_read + 1/298.15) - 273.15;
    
    //check whether it is C or F to report the correct values
    if (scale_arg == 'F')
    {
        float fahrenheit = (celsius * (9 / 5)) + 32;
        
        return fahrenheit;
    }
    else{
        return celsius;
    }
}

void send_to_log()
{
    if (log_flag == 1)
    {
        //have to separate more
        printf(buffer, log_file_name);
    }
}

void command_buffer_error(char* command_buffer)
{
    if (command_buffer == NULL)
    {
        fprintf(stderr, "Command cannot be handle due to empty input\n");
        exit(1);
    }
}

void calculated_period_error(int calculated_period)
{
    if (calculated_period == 0)
    {
        fprintf(stderr, "Period command failed due to invalid command\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void print_func(char *input);

void print_shutdown_buffer()
{
    sprintf(shutdown_buffer, "%02d:%02d:%02d SHUTDOWN", shut_down_current_time->tm_hour, shut_down_current_time->tm_min, shut_down_current_time->tm_sec);
}

void print_shutdown_socket()
{
    dprintf(sock_fd, "%02d:%02d:%02d SHUTDOWN", shut_down_current_time->tm_hour, shut_down_current_time->tm_min, shut_down_current_time->tm_sec);
}

void shutdown_buffer_log()
{
    fputs(shutdown_buffer, log_file_name);
}

void print_shutdown_log()
{
    if (log_flag == 1 && log_file_name != NULL)
    {
        shutdown_buffer_log();
    }
}

void print_shutdown_info()
{
    print_shutdown_buffer();
    
    print_shutdown_socket();
    
    print_shutdown_log();
}

void shut_down_func()
{
    //repeat the same process as the set up time process
    check_error = gettimeofday(&shut_down_clock_time, NULL);
    gettimeofday_error(check_error);
    
    shut_down_current_time = localtime(&shut_down_clock_time.tv_sec);
    
    print_shutdown_info();
    
    exit(0);
}

void clean_logfile()
{
    //remember to clean the log file
    fflush(log_file_name);
}

void print_to_logfile(char *input)
{
    if (log_flag == 1)
    {
        fprintf(log_file_name, "%s\n", input);
        
        clean_logfile();
    }
}

void print_func(char *input)
{
    print_to_logfile(input);
}

void insert_null_byte(char* command_buffer, int command_length)
{
    int last_index = command_length - 1;
    command_buffer[last_index] = null_byte;
}

void skip_over(char* command_buffer)
{
    char command_buffer_value;
    
    while (command_buffer_value == tab || command_buffer_value == blank)
    {
        command_buffer_value = *command_buffer;
        
        command_buffer++;
    }
}

char* find_first_period(char* command_buffer)
{
    char period_string[] = "PERIOD=";
    
    return strstr(command_buffer, period_string);
}

char* find_first_log(char* command_buffer)
{
    char log_string[] = "LOG";
    
    return strstr(command_buffer, log_string);
}

void call_to_print(char* command_buffer)
{
    print_func(command_buffer);
}

void increment_7(char* command_buffer)
{
    constant1 = command_buffer;
    constant1 = constant1 + 7;
}

void check_digit()
{
    char arg = *constant1;
    
    if (!isdigit(arg))
    {
        return;
    }
}

void loop_over()
{
    while(*constant1 != 0)
    {
        check_digit();
        
        constant1++;
    }
}

void handle_period_func(char* command_buffer)
{
    increment_7(command_buffer);
    
    if (*constant1 != 0)
    {
        int constant2 = atoi(constant1);
        
        loop_over();
        
        period_arg = constant2;
    }
}

void handle_command(char* command_buffer)
{
    //check if it is empty first
    command_buffer_error(command_buffer);
    
    //find the length of the command
    int command_length = strlen(command_buffer);
    
    //manually input the null byte character at the end of the char array
    insert_null_byte(command_buffer, command_length);
    
    //skip over the tab and the blank spaces
    skip_over(command_buffer);
    
    //find the first occurence of the period and log
    first_period = find_first_period(command_buffer);
    first_log = find_first_log(command_buffer);
    
    if (strcmp(command_buffer, "SCALE=F") == 0)
    {
        scale_arg = 'F';
        
        call_to_print(command_buffer);
    }
    else if (strcmp(command_buffer, "SCALE=C") == 0)
    {
        scale_arg = 'C';
        
        call_to_print(command_buffer);
    }
    else if (strcmp(command_buffer, "STOP") == 0)
    {
        report_flag = 0;
        
        call_to_print(command_buffer);
    }
    else if (strcmp(command_buffer, "START") == 0)
    {
        report_flag = 1;
        
        call_to_print(command_buffer);
    }
    else if (first_log == command_buffer)
    {
        //log file case
        call_to_print(command_buffer);
    }
    else if (strcmp(command_buffer, "OFF") == 0)
    {
        call_to_print(command_buffer);
        
        //call the shut down function and print out the related info
        shut_down_func();
    }
    else if (first_period == command_buffer)
    {
        //period case
        handle_period_func(command_buffer);
        
        call_to_print(command_buffer);
    }
    else{
        //unrecognized command print to the log file and exit with 1
        call_to_print(command_buffer);
        exit(1);
    }
}

void print_current_info(struct tm* current_time, int temp_digit, int temp_decimal)
{
    sprintf(buffer, "%02d:%02d:%02d %d.%1d", current_time->tm_hour, current_time->tm_min, current_time->tm_sec, temp_digit, temp_decimal);
    print_func(buffer);
}


void check_id_number(int argc)
{
    if (optind >= argc)
    {
        fprintf(stderr, "No port number provided.\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void check_port_number()
{
    if (port_number == 0)
    {
        fprintf(stderr, "Invalid port number provided.\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void check_host()
{
    //check if the host name is empty
    if (host_name == NULL)
    {
        fprintf(stderr, "No valid host name provided.\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void check_log_file()
{
    //check if the log file is empty
    if (log_file_name == 0)
    {
        fprintf(stderr, "No valid log file provided.\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void check_pass_in_arg(int argc, char **argv)
{
    //check whether the id number and display error message
    check_id_number(argc);
    
    //store the port number
    port_number = atoi(argv[optind]);
    
    //check if the port number is valid
    check_port_number();
    
    //check the host name
    check_host();
    
    //check the log file
    check_log_file();
}

void set_up_server_address()
{
    //use memset to initalize to zero
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)(server -> h_addr), (char *)&serv_addr.sin_addr.s_addr, server->h_length);
}

void set_up_port()
{
    serv_addr.sin_port = htons(port_number);
}

void connect_to_server()
{
    int check = 0;
    check = connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    check_connect_error(check);
}

void write_id_socket()
{
    dprintf(sock_fd, "ID=%s\n", id_arg);
}

void write_id()
{
    sprintf(id_buffer, "ID=%s\n", id_arg);
    write_id_socket();
}

void check_sock_fd_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "socket() fail.\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void check_gethostbyname_error(struct hostent* check)
{
    if (check == NULL)
    {
        fprintf(stderr, "gethostbyname() fail.\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void write_to_id_buffer()
{
    fputs(id_buffer, log_file_name);
}

void write_log_id()
{
    if (log_flag == 1 && log_file_name != NULL)
    {
        write_to_id_buffer();
    }
}

void print_to_buffer(struct tm* current_time, int temp_decimal)
{
    sprintf(command_buffer, "%02d:%02d:%02d %d.%1d\n", current_time->tm_hour, current_time->tm_min, current_time->tm_sec, 68, temp_decimal);
}

void print_to_socket(struct tm* current_time, int temp_decimal)
{
     dprintf(sock_fd, "%02d:%02d:%02d %d.%1d\n", current_time->tm_hour, current_time->tm_min, current_time->tm_sec, 68, temp_decimal);
}

void write_to_command_buffer()
{
    if (log_flag == 1 && log_file_name != NULL)
    {
        fputs(command_buffer, log_file_name);
    }
}

void open_read_file()
{
    //read only
    read_file = fdopen(sock_fd, "r");
}

//Based from resources : https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
int main(int argc, char **argv)
{
    //set up the required options
    static struct option long_options[] = {
        {"id", required_argument, NULL, 'i'},
        {"host", required_argument, NULL, 'h'},
        {"scale", required_argument, NULL, 's'},
        {"period", required_argument, NULL, 'p'},
        {"log", required_argument, NULL, 'l'},
        {0, 0, 0, 0}
    };
    
    int optionIndex = 0;
    
    while ((optionIndex = getopt_long(argc, argv, "i:s:p:l:h:", long_options, NULL)) != -1)
    {
        switch(optionIndex)
        {
            case 'i':
                id_flag = 1;
                //need a var to store the number for id
                id_arg = optarg;
                break;
            case 'h':
                host_flag = 1;
                //need a var to store the name of the host
                host_name = optarg;
            case 's':
                scale_flag = 1;
                char check_arg = optarg[0];
                
                if (check_arg == 'C' || check_arg == 'F')
                {
                    scale_arg = optarg[0];
                }
                break;
            case 'p':
                period_flag = 1;
                period_arg = atoi(optarg);
                break;
            case 'l':
                log_flag = 1;
                
                //allow both read and write at the same time
                log_file_name = fopen(optarg, "w+");
                //log_file_name = optarg;
                fopen_error(log_file_name);
                
                break;
            default:
                //print the unrecognized error message and exit with code 1
                fprintf(stderr, "Unrecognized argument: %s\n. Proper usage : lab4c --id=id_number --host=host_name --scale=F --scale=C --period=seconds --log=line_of_text\n", strerror(optionIndex));
                exit(1);
        }
    }
    
    //check all the required pass in argument
    check_pass_in_arg(argc, argv);
    
    //Based from resources : https://www.geeksforgeeks.org/socket-programming-cc/
    //set up the socket
    //start reading into the socket file descriptor
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    check_sock_fd_error(sock_fd);
    
    open_read_file();
    
    //set up the server
    server = gethostbyname(host_name);
    check_gethostbyname_error(server);
    
    //set up the server address
    set_up_server_address();
    
    //set up the port number
    set_up_port();
    
    //connect to the server
    connect_to_server();
    
    //display the id number
    write_id();
    
    //write to log file for id name
    write_log_id();
    
    //alloctae variable to read from stdin input
    command_buffer = (char *)malloc(60 * sizeof(char));
    malloc_error(command_buffer);
    
    copy_buffer = (char *)malloc(60 * sizeof(char));
    malloc_error(command_buffer);
    
    //Based from resources : https://drive.google.com/drive/folders/0B6dyEb8VXZo-N3hVcVI0UFpWSVk
    //set up the temperature sensor and board button
    temperature_sensor = mraa_aio_init(temp_sensor_init);
    mraa_aio_init_error(temperature_sensor);
    
    button = mraa_gpio_init(button_init);
    //mraa_gpio_init_error(button);
    
    //To specify as the input button
    check = mraa_gpio_dir(button, MRAA_GPIO_IN);
    //mraa_gpio_dir_error(check);
    
    //when the button is pressed, call the shut down function
    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &shut_down_func, NULL);
    
    //set up the poll and socket
    //Based from resources : http://www.unixguide.net/unix/programming/2.1.2.shtml
    struct pollfd poll_fds[1];
    poll_fds[0].fd = sock_fd;
    poll_fds[0].events = POLLIN | POLLHUP | POLLERR;
    
    //check variable for poll()
    int pollCheck = 0;
    
    //Based from resources : https://drive.google.com/drive/folders/0B6dyEb8VXZo-N3hVcVI0UFpWSVk
    while (1)
    {
        //get time of the day
        check_error = gettimeofday(&clock_time, 0);
        gettimeofday_error(check_error);
        
        //check the time first
        if (report_flag && clock_time.tv_sec >= follow_up_time)
        {
            //record the current temperature
            current_temperature = record_temperature_func();
            //get the local time and check error
            current_time = localtime(&clock_time.tv_sec);
            localtime_error(current_time);
            
            //separate the temperature into digit and decimal
            int print_temp = current_temperature * 10;
            //int temp_digit = abs(print_temp / 10);
            int temp_decimal = abs(print_temp % 10);
            
            print_to_buffer(current_time, temp_decimal);
            
            print_to_socket(current_time, temp_decimal);
            
            write_to_command_buffer();
            
            //find the follow up time first
            follow_up_time = clock_time.tv_sec + period_arg;
        }
        
        //set up the poll command
        //Based from resources : http://www.unixguide.net/unix/programming/2.1.2.shtml
        pollCheck = poll(poll_fds, 1, 0);
        poll_error(pollCheck);
        
        if (pollCheck > 0)
        {
            //read from keyboard and check whether the standard input is available
            if ((poll_fds[0].revents & POLLIN))
            {
                fgets(read_buffer, 60, read_file);
                handle_command(read_buffer);
            }
        }
        
    }
    
    free(command_buffer);
    
    mraa_aio_close(temperature_sensor);
    mraa_gpio_close(button);
    
    exit(0);
}
