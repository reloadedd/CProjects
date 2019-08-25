#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // for memset()
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>     // for close()
#include <time.h>

#define AUTHOR "rel0adedd"
#define VERSION "v1.0"
#define CONNECTIONS 5
#define WEBROOT "./webroot"
#define PORT 80 /* Root privileges are needed to bind any port below 1024 */

void handle_connection(int, struct sockaddr_in *);
void not_found(int, int);
off_t get_file_size(char *);
char *get_current_time();
void recv_line(int, char *);
void fatal(char *);

int main() {
    int sockfd, client_sockfd;
    struct sockaddr_in server_address, client_address;
    int yes = 1;
    socklen_t sin_size;

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        fatal("when creating the socket.\n");

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
        fatal("when setting socket option SO_REUSEADDR.\n");

    // Initialize the server with the IP and PORT
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    memset(server_address.sin_zero, '\0', 8);
    
    // Binding the socket
    if (bind(sockfd, (struct sockaddr *) &server_address, sizeof(struct sockaddr)) == -1)
        fatal("when binding the socket. Have you tried running it with sudo?\n");

    // Make him listen
    if (listen(sockfd, CONNECTIONS) == -1)
        fatal("when listening for incoming connections.\n");
    printf("\tThe server is ready to go. Talk with it on port %d.\n", PORT);
    printf("-----------------------------------------------------------\n");

    // Infinite loop for accepting connections
    for (;;) {
        sin_size = sizeof(struct sockaddr_in);
        client_sockfd = accept(sockfd,(struct sockaddr *) &client_address, &sin_size);
        if (client_sockfd == -1)
            fatal("when accepting connection.");

        handle_connection(client_sockfd, &client_address);   
    }

    return 0;
}

/* This function handles thoroughly the connection.
 * It handles the GET / HEAD requests, as well as 200 and 404 response codes.
 * At the end, it closes gracefully the socket descriptor.
 */
void handle_connection(int socket_descriptor, struct sockaddr_in * client_addr) {
    char *ret, request[256], resource[256]; // Be careful not to overflow. I told you!
    char client_ip[INET_ADDRSTRLEN];    // space to hold the IPv4 string
    int client_port;              // the port that client used to connect

    // Get the client's IP address and port number
    inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    client_port = ntohs(client_addr->sin_port);
    
    recv_line(socket_descriptor, request);  // Don't need the value returned

    ret = strstr(request, " HTTP");  // Search for something that might respect the
    // standard, for instance: HTTP/1.1
    printf("Got connection from %s:%d ---> %s\n", client_ip, client_port, request);

    if (ret == NULL) {
        send(socket_descriptor, "You are talking with the wrong server\r\n", 40, 0);
    }else{  // Until now, it's fine
        *ret = 0;   // Terminate the buffer at the end of the URL
        ret = NULL;     // Used to mark an invalid request
        
        /* The structure of a GET request (same for head) is something like this
         * GET URL HTTP/1.1 (for instance)
         * So, after this bit of code, the ret will contain the URL, or NULL if it
         * didn't found a HEAD or GET request
         */
        if (strncmp(request, "GET ", 4) == 0)   // It's a GET request
            ret = request + 4;
        else if (strncmp(request, "HEAD ", 5) == 0) // It's a HEAD request
            ret = request + 5;
        
        if (ret == NULL) {
            send(socket_descriptor, "Only GET or HEAD here, nothing more, nothing less.\r\n", 
            53, 0);
        }else{  // So, apparently it's a valid HTTP request
            FILE *fptr;
            
            if (ret[strlen(ret) - 1] == '/')    // For resources ending with '/'
                strcat(ret, "index.html");      // add 'index.html' to the end
            strcpy(resource, WEBROOT);
            strcat(resource, ret);

            if ((fptr = fopen(resource, "rb")) == NULL)    // Open for read-only
                not_found(socket_descriptor, (ret == request + 4));  // Sends a 404 NOT FOUND
            else{   // Sends a 200 OK
                printf("\t\t\tServer Response: HTTP/1.1 200 OK\n\n");

                if (ret == request + 5) {   // Then it's a HEAD request
                    send(socket_descriptor, "HTTP/1.1 200 OK\r\n", 18, 0);
                    send(socket_descriptor, get_current_time(), 33, 0);
                    send(socket_descriptor, "Server: An awesome one\r\n", 25, 0);
                }else{                      // Or, a GET request
                    char *page;
                    off_t filesize = get_file_size(resource);

                    if ((page = malloc(filesize + 1)) == NULL) {
                        shutdown(socket_descriptor, SHUT_RDWR);
                        fatal("when allocating memory for the file requested.\n");
                    }

                    fread(page, sizeof(char), filesize, fptr);
                    send(socket_descriptor, page, filesize, 0);
                    free(page);
                }
            fclose(fptr);   // close the file 
            }   // End if block for file found/not found
        } // End if block for valid request
    } // End if block for valid HTTP

    shutdown(socket_descriptor, SHUT_RDWR);
}  

/* This function handles the case when the resource requested is not found on the webserver.
 * It sends some headers (in case of HEAD request) or some headers + a 404 page (in case of GET request)
 */
void not_found(int sockfd, int request_type) {
    printf("\t\t\tServer Response: HTTP/1.1 404 NOT FOUND\n\n");

    if (request_type == 0)  {   // This means it's a HEAD request
        send(sockfd, "HTTP/1.1 404 NOT FOUND\r\n", 25, 0);
        send(sockfd, get_current_time(), 33, 0);
        send(sockfd, "Server: An awesome one\r\n", 25, 0);
    }else{  // or it's GET request
        FILE *fptr;
        char filename[25], *page;
        off_t filesize;

        strcpy(filename, WEBROOT);
        strcat(filename, "/404.html");

        if ((fptr = fopen(filename, "rb")) == NULL) {  // 
            shutdown(sockfd, SHUT_RDWR);
            fatal("404.html file was, apparently, not found.\n");
        }

        filesize = get_file_size(filename);
        if ((page = malloc(filesize + 1)) == NULL) {
            shutdown(sockfd, SHUT_RDWR);
            fatal("when allocating memory for 404.html page.\n");
        }

        fread(page, sizeof(char), filesize, fptr);
        send(sockfd, page, filesize, 0);
        free(page);
    }
}

// Returns the file size
off_t get_file_size(char *filename) {
    struct stat file_status;

    if (stat(filename, &file_status) == 0)
        return file_status.st_size;
    
    return -1;  // on failure
}

// Returns the current time as a string
char *get_current_time() {
    time_t rawtime;
    struct tm *info;
    static char current_time[33];

    time(&rawtime);
    info = localtime(&rawtime);

    strcpy(current_time, "Date: ");
    strcat(current_time, asctime(info));
    current_time[strlen(current_time) - 1] = 0;     // eliminate that newline char
    strcat(current_time, "\r\n");

    return current_time;
}

// This function reads only a line at a time. It stops when the CRLF characters are found.
void recv_line(int sockfd, char *buffer) {
    const char CRLF[] = "\r\n"; 
    int eol_matched = 0, crlf_size = strlen(CRLF);

    while(recv(sockfd, buffer, 1, 0) == 1) { 
        if(*buffer == CRLF[eol_matched]) { 
            eol_matched++;  // Found one character that marks the end of line
            if(eol_matched == crlf_size) { 
                buffer[1 - crlf_size] = '\x00';   // Terminate the NTBS (Null-Terminated Byte String)
                return;
            }
        } else 
            eol_matched = 0;    // Start again
        
        buffer++;   // Read another character
    }
}

// It doesn't do too much
void fatal(char *buffer) {
    printf("[Error] %s", buffer);
    exit(EXIT_FAILURE);
}