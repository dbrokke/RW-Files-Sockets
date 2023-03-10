/*
 * Auth: Danny Brokke (dbrokke@unomaha.edu)
 * Date: 07-24-22  (Due: 07-24-22)
 * Course: CSCI-3550 (Sec: 850)
 * Desc:  Using sockets to read from files and write into new files.
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
char* buf = NULL;
ssize_t bytes_read;
int bytes_written;
int fd_out;
int sockfd;
int val;
int cl_sockfd;
struct sockaddr_in cl_sa;
socklen_t cl_sa_size;
struct in_addr ia;
int i;
unsigned short int port;
char fname[80];

/* Signal interruption handler function for when the interruption is called
*/
void SIGINT_handler(int sig)
{
    fprintf(stderr, "\nserver: Server interrupted. Shutting down.\nGoodbye!\n");
    cleanup();
    exit(EXIT_FAILURE);
}

int main( int argc, char* argv[] )
{
    /*
     * Signal handler call.
     */
    signal(SIGINT, SIGINT_handler);

    /*
     * Creating my counter (i). Starting off by checking for a privileged port.
    */
    i = 0;
    if (atoi(argv[1]) <= 1024 && atoi(argv[1])>= 0)
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
    if (inet_aton("127.0.0.1", &ia) == 0)
    {
        fprintf(stderr, "ERROR: Setting the IP address.\n");
    }

    /*
     * Filling the socket address structure fields
    */
    cl_sa.sin_family = AF_INET;
    cl_sa.sin_addr = ia;
    port = (unsigned short int) atoi(argv[1]); 
    cl_sa.sin_port = htons(port);
        
    /*
     * Binding the socket
    */
    if( bind(sockfd, (struct sockaddr *) &cl_sa, sizeof(cl_sa)) != 0)
    {
        fprintf(stderr, "server: ERROR: Failed to bind socket.\n");
        cleanup();
    }

    /*
     * Turning the socket into a "listening" socket
    */
    if (listen(sockfd, 32) != 0)
    {
        fprintf(stderr, "server: ERROR: listen(): Failed.\n");
    }
    

    

    

    while(1)
    {
        
        /*
         * Resetting and incrementing necessary values. Also checking to make sure malloc worked with buf.
        */ 
        i = i + 1;
        cl_sa_size = sizeof(cl_sa);
        /*
        memset( (void *) &cl_sa, 0, sizeof(cl_sa)); */
        buf = (void *) malloc(BUF_SIZE);
        if (buf == NULL)
        {
            fprintf( stderr, "server: ERROR Failed to allocate memory.\n");
            exit(EXIT_FAILURE);
        }

        printf("server: Awating TCP connections over port %d\n", port);
        /*
         * Running the accept function with previously crafted values. If accept works, it goes into reading the file.
        */
        cl_sockfd = accept(sockfd, (struct sockaddr *) &cl_sa, &cl_sa_size );
        if (cl_sockfd > 0)
        {
           printf("server: Connection accepted!\n");
           printf("server: Receiving file...\n");
           bytes_read = recv(cl_sockfd, (void *) buf, BUF_SIZE, MSG_WAITALL);
           if (bytes_read > 0)
           {
                /*
                 * Creating a file with the right naming convention. Then opening said file. If it can't open it produces an error.
                */
                sprintf(fname, "file-%02d.dat", i);
                fd_out = open(fname, O_CREAT|O_WRONLY|O_TRUNC,  S_IRUSR|S_IWUSR );
                if(fd_out < 0)
                {
                    fprintf(stderr, "server: ERROR: Could not create %s\n", fname);
                    cleanup();
                    exit(EXIT_FAILURE);
                }

                /*
                 * If the file can be created the buffer is written into it. If not all bytes are written an error
                 * is produced.
                */
                bytes_written = write( fd_out, buf, (size_t) bytes_read);
                if (bytes_written != bytes_read)
                {
                    fprintf(stderr, "server: ERROR: Error writing all bytes.\n");
                    cleanup();
                    exit(EXIT_FAILURE);
                }

           }

            /*
             * If end of file, close. Else produce an error.
            */
           else if (bytes_read == 0)
           {
                /*close(cl_sockfd); 
                */
                close(cl_sockfd);
                printf("server: Connection closed.\n");
                printf("server: Saving file \"%s\"\n", fname);
                printf("server: Done.\n");
           }
           else
           {
                fprintf(stderr, "server ERROR: Reading from socket.\n");
           }
        }

        /*
         * Produce an error if the accept function doesn't work. Let the client know the file is being saved.
        */
        else
        {
            fprintf(stderr, "ERROR: While attempting to accept a connection.\n");
        }
        close(cl_sockfd);
        printf("server: Connection closed.\n");
        printf("server: Saving file: \"%s\"\n", fname);
        printf("server: done\n\n"); 
    }
    /*
    if (sockfd > -1)
    {
        close(sockfd);
        sockfd = -1;
    } */
    
}

void cleanup(void)
{
    /* If the buffer isn't empty, empty it and point it back to NULL. 
    */
    if (buf != NULL)
    {
        free(buf);
        buf = NULL;
    }

    /* If sockfd is still open, close it and set it to an incorrect value.
    */
    if (sockfd >= 0)
    {
        close(sockfd);
        sockfd = -1;
    }

    /* If cl_sockfd is still open, close it and set it to an incorrect value.
    */
    if(cl_sockfd >= 0)
    {
        close(cl_sockfd);
        cl_sockfd = -1;
    }

    /* If fd_out is still open, close it and set to an incorrect value.
    */
    if(fd_out >= 0)
    {
        close(fd_out);
        fd_out = -1;
    }

    exit(EXIT_SUCCESS);
}

