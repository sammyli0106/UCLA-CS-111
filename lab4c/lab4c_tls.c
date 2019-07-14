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
#include <openssl/ssl.h>
#include <openssl/err.h>

//define constant values
#define temp_sensor_init 1
#define button_init 60
#define buffer_size 60
#define null_byte '\0'
#define tab '\t'
#define blank ' '
#define file_const 0666
#define new_line '\n'
#define null_byte '\0'
#define period_const 7
#define poll_const 100
#define zero 0

//variable that store the passed argument
//default to one second
int period_arg = 1;
//default to store one char with letter F
char scale_arg = 'F';
//default to zero
char* log_file_name = 0;
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
int id_arg = 0;
int host_flag = 0;
char id_buffer[60];
char temp_buffer[60];
int read_bytes = 0;
char output_buffer[60];

//SSL variable
const SSL_METHOD* ssl_meth;
SSL_CTX* ssl_context;
SSL* ssl_client;
int ssl_check;

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

void ssl_write_shutdown_buffer()
{
    int shutdown_buffer_length = strlen(shutdown_buffer);
    
    SSL_write(ssl_client, shutdown_buffer, shutdown_buffer_length);
}

void log_write_shutdown_buffer()
{
    if (log_flag == 1)
    {
        int shutdown_buffer_length = strlen(shutdown_buffer);
        
        write(logfile_fd, shutdown_buffer, shutdown_buffer_length);
    }
}

void print_shutdown_info()
{
    print_shutdown_buffer();
    
    ssl_write_shutdown_buffer();
    
    log_write_shutdown_buffer();
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

void print_to_logfile(char *input)
{
    if (log_flag == 1)
    {
        //fprintf(log_file_name, "%s\n", input);
        char report[strlen(input)];
        strcpy(report, input);
        report[strlen(report)] = '\n';
        
        size_t write_bytes = strlen(input) + 1;
        
        write(logfile_fd, report, write_bytes);
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

void copy_into_buffer(char* command_buffer)
{
    strcpy(output_buffer, command_buffer);
}

void assign_new_line(int output_buffer_length)
{
    output_buffer[output_buffer_length] = new_line;
}

size_t calculate_bytes(char* command_buffer)
{
    return (strlen(command_buffer) + 1);
}

void print_command_to_log(char* command_buffer)
{
    if (log_flag == 1)
    {
        copy_into_buffer(command_buffer);
        
        int output_buffer_length = strlen(output_buffer);
        
        assign_new_line(output_buffer_length);

        size_t bytes = calculate_bytes(command_buffer);
        
        write(logfile_fd, output_buffer, bytes);
    }
}

void handle_command(char* command_buffer)
{
    int log_length = strlen("LOG");
    int period_length = strlen("PERIOD=");
    
    print_command_to_log(command_buffer);
    
    if (strcmp(command_buffer, "SCALE=F") == 0)
    {
        scale_arg = 'F';
    }
    else if (strcmp(command_buffer, "SCALE=C") == 0)
    {
        scale_arg = 'C';
    }
    else if (strcmp(command_buffer, "STOP") == 0)
    {
        report_flag = 0;
    }
    else if (strcmp(command_buffer, "START") == 0)
    {
        report_flag = 1;
    }
    else if (strncmp(command_buffer, "LOG", log_length) == 0)
    {
        //there is nothing here
        //handle at above
    }
    else if (strcmp(command_buffer, "OFF") == 0)
    {
        //call the shut down function and print out the related info
        shut_down_func();
    }
    else if (strncmp(command_buffer, "PERIOD=", period_length) == 0)
    {
        //period case
        period_arg = atoi(command_buffer + period_const);
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

void write_id()
{
    sprintf(id_buffer, "ID=%d\n", id_arg);
}

void check_sock_fd_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "socket() error provided.\n");
        exit(1);
    }
}

void creat_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "creat() failed.\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void create_fd()
{
    logfile_fd = creat(log_file_name, file_const);
}


void create_logfile()
{
    create_fd();
    
    creat_error(logfile_fd);
}

void gethostbyname_error()
{
    if (server == NULL)
    {
        fprintf(stderr, "gethostbyname() error\n");
        exit(1);
    }
}

void ssl_context_error()
{
    if (ssl_context == NULL)
    {
        fprintf(stderr, "SSL context() error\n");
        exit(1);
    }
}

void ssl_set_fd_error(int ssl_check)
{
    if (ssl_check == 0)
    {
        fprintf(stderr, "SSL_set_fd() error\n");
        exit(1);
    }
}

void ssl_connect_error(int ssl_check)
{
    if (ssl_check <= 0)
    {
        fprintf(stderr, "SSL_connect() error\n");
        exit(1);
    }
}

void ssl_write_error()
{
    if (ssl_check <= 0)
    {
        fprintf(stderr, "SSL_write() error\n");
        exit(1);
    }
}

void initalize_ssl()
{
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
}


void write_id_buffer()
{
    sprintf(id_buffer, "ID=%d\n", id_arg);
}

void ssl_write_id_buffer()
{
    ssl_check = SSL_write(ssl_client, id_buffer, strlen(id_buffer));
    ssl_write_error(ssl_check);
}

void log_write_id_buffer()
{
    if (log_flag == 1)
    {
        int length = strlen(id_buffer);
        
        write(logfile_fd, id_buffer, length);
    }
}

void display_id()
{
    write_id_buffer();
    
    ssl_write_id_buffer();
    
    log_write_id_buffer();
}

void print_current_time_buffer(int temp_decimal)
{
    sprintf(temp_buffer, "%02d:%02d:%02d %d.%1d\n", current_time->tm_hour, current_time->tm_min, current_time->tm_sec, 68, temp_decimal);
}

void ssl_write_current_time_buffer()
{
    SSL_write(ssl_client, temp_buffer, strlen(temp_buffer));
}

void print_log_current_time_buffer()
{
    if (log_flag == 1)
    {
        int length = strlen(temp_buffer);
        write(logfile_fd, temp_buffer, length);
    }
}

void print_current_time(int temp_decimal)
{
    print_current_time_buffer(temp_decimal);
    
    ssl_write_current_time_buffer();
    
    print_log_current_time_buffer();
}

void ssl_read_buffer()
{
    read_bytes = SSL_read(ssl_client, command_buffer, strlen(command_buffer));
}

void assign_null_byte(int num)
{
    copy_buffer[num] = null_byte;
}

void assign_copy_buffer(int num, int i)
{
    copy_buffer[num] = command_buffer[i];
}

int check_command_buffer(int i)
{
    return (command_buffer[i] == new_line);
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
                id_arg = atoi(optarg);
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
                //log_file_name = fopen(optarg, "w+");
                log_file_name = optarg;
                //fopen_error(log_file_name);
                
                break;
            default:
                //print the unrecognized error message and exit with code 1
                fprintf(stderr, "Unrecognized argument: %s\n. Proper usage : lab4c --id=id_number --host=host_name --scale=F --scale=C --period=seconds --log=line_of_text\n", strerror(optionIndex));
                exit(1);
        }
    }
    
    //check all the required pass in argument
    check_pass_in_arg(argc, argv);
    
    create_logfile();
    
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    check_sock_fd_error(sock_fd);
    
    //set up the server
    server = gethostbyname(host_name);
    gethostbyname_error();
    
    //set up the server address
    set_up_server_address();
    
    //set up the port number
    set_up_port();
    
    //connect to the server
    connect_to_server();
    
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
    
    initalize_ssl();
    
    ssl_meth = SSLv23_client_method();
    ssl_context = SSL_CTX_new(ssl_meth);
    if (ssl_context == NULL)
    {
        fprintf(stderr, "SSL context error\n");
        exit(1);
    }
    
    ssl_client = SSL_new(ssl_context);
    ssl_context_error();
    
    //connect SSL
    ssl_check = SSL_set_fd(ssl_client, sock_fd);
    ssl_set_fd_error(ssl_check);
    
    ssl_check = SSL_connect(ssl_client);
    ssl_connect_error(ssl_check);
    
    //display the id number
    
    display_id();
    
    //alloctae variable to read from stdin input
    command_buffer = (char *)malloc(60 * sizeof(char));
    malloc_error(command_buffer);
    
    copy_buffer = (char *)malloc(60 * sizeof(char));
    malloc_error(command_buffer);
    
    
    //set up the poll and socket
    //Based from resources : http://www.unixguide.net/unix/programming/2.1.2.shtml
    struct pollfd poll_fds[1];
    poll_fds[0].fd = sock_fd;
    poll_fds[0].events = POLLIN;
    
    //check variable for poll()
    int pollCheck = 0;
    
    int index = 0;
    
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
            
            print_current_time(temp_decimal);
            
            //find the follow up time first
            follow_up_time = clock_time.tv_sec + period_arg;
        }
        
        //set up the poll command
        //Based from resources : http://www.unixguide.net/unix/programming/2.1.2.shtml
        pollCheck = poll(poll_fds, 1, 0);   //if not use poll const
        poll_error(pollCheck);
        
        if (pollCheck > 0)
        {
            //read from keyboard and check whether the standard input is available
            if ((poll_fds[0].revents & POLLIN))
            {
                ssl_read_buffer();
                
                for (int i = 0; i < read_bytes; i++)
                {
                    if (check_command_buffer(i))
                    {
                        assign_null_byte(index);
                        
                        handle_command(copy_buffer);
                        index = zero;
                    }
                    else
                    {
                        assign_copy_buffer(index, i);
                        index++;
                    }
                }

            }
        }
        
    }
    
    free(command_buffer);
    
    mraa_aio_close(temperature_sensor);
    mraa_gpio_close(button);
    
    exit(0);
}
