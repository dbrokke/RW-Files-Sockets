/* *********************************************
   Author: Danny Brokke (dannybrokke@live.com)
   Desc: Reading and writing from files.
   *********************************************
*/

/* Including all necessary libraries
*/
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

/* Cleanup function prototype, required by the compiler
*/
void cleanup(void);

/* Defining global variables to be used
*/
#define BUF_SIZE (100*1024*1024)
char* out_buf = NULL;
int bytes_sent;
int bytes_read;
int fd;
int fd_in;
int sockfd;
int val;

struct sockaddr_in cl_sa;
socklen_t cl_sa_size;
struct in_addr ia;
int i;
unsigned short int port;
int fileCount;
int counter;

/* Signal interruption handler function for when the interruption is called
*/
void SIGINT_handler(int sig)
{
    fprintf(stderr, "server: Server interrupted. Shutting down.\n");
    cleanup();
    exit(EXIT_FAILURE);
}

int main( int argc, char* argv[] )
{
    fileCount = argc - 3;
    i = 3;
    if (atoi(argv[2]) <= 1024 && atoi(argv[2])>= 0)
    {
        fprintf(stderr, "ERROR: Port number is privileged.\n");
        exit(EXIT_FAILURE);
    }
    /*
     * Opening a socket
    */
    sockfd = -1;
    sockfd = socket( AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf( stderr, "ERROR: Failed to create socket.\n");
    }


    /*
     * Making the socket reusable.
    */
    val = 1;
    if (setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &val, sizeof(int) ) != 0)
    {
        fprintf( stderr, "ERROR: setsockopt() failed.\n");
    }
    /*
     * Setting the socket address
    */
    if (inet_aton(argv[1], &ia) == 0)
    {
        fprintf(stderr, "ERROR: Setting the IP address.\n");
    }

    /*
     * Filling the socket address structure fields
    */
    cl_sa.sin_family = AF_INET;
    cl_sa.sin_addr = ia;
    port = (unsigned short int) atoi(argv[2]); 
    cl_sa.sin_port = htons(port);


        printf("client: connecting to %s:%s\n", argv[1], argv[2]);
        /*
         * Using the connect function to connect this socket to the server socket
         */ 
        if(connect(sockfd, (struct sockaddr *) &cl_sa, sizeof(cl_sa)) != 0)
        {
            fprintf(stderr, "ERROR: Attempting to connect with the server.\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("client: connection accepted!\n");
        }

    while(fileCount > 0)
    {
        counter = counter + 1;
        cl_sa_size = sizeof(cl_sa);
        out_buf = (void *) malloc(BUF_SIZE);
        if (out_buf == NULL)
        {
            fprintf( stderr, "server: ERROR Failed to allocate memory.\n");
            exit(EXIT_FAILURE);
        }

            /* Letting the user know which file is being read. Then opening that file.
            * If the file can't be opened, an error message is given.
            */ 
            fd_in = open(argv[i], O_RDONLY);
            if (fd_in < 0)
            {
                fprintf(stderr, "ERROR: Failed to open %s\n", argv[i]);
                cleanup();
                exit(EXIT_FAILURE);
            }


            /* Reading the file into the buffer, storing the amount of bytes read into the variable. If no bytes were read then it cleans up and exit fails.
            */
            bytes_read = read(fd_in, out_buf, BUF_SIZE);
            if (bytes_read < 0)
            {
                fprintf(stderr, "ERROR: Unable to read file %s\n", argv[i]);
                cleanup();
                exit(EXIT_FAILURE);
            }



        /*
        * Sending bytes and keeping track fo how many were sent.
        */
        printf("client: Sending: \"%s\"...\n", argv[fileCount+2]);
        bytes_sent = send(sockfd, (const void *) out_buf, 1024, 0);
        

        
        if (bytes_sent != 1024)
        {
            fprintf(stderr, "ERROR: send() failed\n");
        }

        fileCount = fileCount - 1;
        i = i + 1;
        printf("client: Done.\n");
        memset( (void *) &out_buf, 0, sizeof(out_buf));
    }
    
    
   exit(EXIT_SUCCESS);
}

void cleanup(void)
{
    /* If the buffer isn't empty, empty it and point it back to NULL. 
    */
    if (out_buf != NULL)
    {
        free(out_buf);
        out_buf = NULL;
    }
    /* If sockfd is still open, close it and set it to an incorrect value.
    */
    if (sockfd >= 0)
    {
        close(sockfd);
        sockfd = -1;
    }

    exit(EXIT_SUCCESS);
}

