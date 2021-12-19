#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include "utils/helper.h"

#define MAXDATASIZE 512
#define PORT "12000"
#define BACKLOG 8 


int main() {
    struct sigaction sa;

    int welcome_socket_fd, client_socket_fd, response_code;
    struct addrinfo hints, *server_info, *itr;
    
    // SERVER SETUP
    // Setup hints addrinfo
    memset(&hints, 0, sizeof hints); 
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 

    // Generate all possible addrinfo structs for local IP and port number
    if ((response_code = getaddrinfo(NULL, PORT, &hints, &server_info)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(response_code));
		return 1;
	}

    // Loop through all addrinfo structs and bind our "welcome" port to the first we can
	for(itr = server_info; itr != NULL; itr = itr->ai_next) {
		if ((welcome_socket_fd = socket(itr->ai_family, itr->ai_socktype, itr->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (bind(welcome_socket_fd, itr->ai_addr, itr->ai_addrlen) == -1) {
			close(welcome_socket_fd);
            perror("server: bind");
			continue;
		}

		break;
	}

    if (itr == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		return 1;
	}

	// After binding (or failing to bind), we no longer neeed the linked list of addrinfo structs
	freeaddrinfo(server_info);

    // Setup "welcome" socket to listen for incoming connections
    if (listen(welcome_socket_fd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

    // Set up SIGCHLD signal handler
    // Remove child process entry from process table
    sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("Server listening on port %s ...\n", PORT);


    // SEND AND RECEIVE DATA
    // The following is used for setting up the client socket file descriptor
    struct sockaddr_storage client_addr;
    socklen_t sin_size;

    // Accept client connection request, fork process, then send and recieve messages
    while(1) {
        sin_size = sizeof(client_addr);
		client_socket_fd = accept(welcome_socket_fd, (struct sockaddr *)&client_addr, &sin_size);
		if (client_socket_fd == -1) {
			perror("accept");
			continue;
		}

		if (!fork()) {
            // Child doesn't need the "welcome" port because this port is still listening on the parent process
			close(welcome_socket_fd); 
			
            // Receive and parse client request
            int num_bytes_read;
            char recv_buffer[MAXDATASIZE];
            if ((num_bytes_read = recv(client_socket_fd, recv_buffer, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
                exit(1);
            }
			recv_buffer[num_bytes_read] = '\0';
            char **parsed_request = malloc(2*sizeof(char *));
			parse_request(recv_buffer, &parsed_request);
            printf("Request method: %s\n", parsed_request[0]);
            printf("Resource path: %s\n", parsed_request[1]);
            
            // Construct and send response
            if (strcmp(parsed_request[0], "GET") == 0) {
                if (strcmp(parsed_request[1], "/") == 0) {
                    char *reply = 
                        "HTTP/1.1 200 OK\n"
                        "Content-Type: text/html\n"
                        "Content-Length:11\n"
                        "Accept-Ranges: bytes\n"
                        "Connection: close\n"
                        "\n"
                        "index\n \r\n\r\n";

                    printf("Sending reply...\n");
                    if (send(client_socket_fd, reply, strlen(reply)+1, 0) == -1) {
                        perror("send");
                        exit(1);
                    }                    
                } else if (strcmp(parsed_request[1], "/cow") == 0) {
                    char *reply = 
                        "HTTP/1.1 200 OK\n"
                        "Content-Type: text/html\n"
                        "Content-Length:9\n"
                        "Accept-Ranges: bytes\n"
                        "Connection: close\n"
                        "\n"
                        "cow\n \r\n\r\n";

                    printf("Sending reply...\n");
                    if (send(client_socket_fd, reply, strlen(reply)+1, 0) == -1) {
                        perror("send");
                        exit(1);
                    }                    
                } else if (strcmp(parsed_request[1], "/pig") == 0) {
                    char *reply = 
                        "HTTP/1.1 200 OK\n"
                        "Content-Type: text/html\n"
                        "Content-Length:9\n"
                        "Accept-Ranges: bytes\n"
                        "Connection: close\n"
                        "\n"
                        "pig\n \r\n\r\n";

                    printf("Sending reply...\n");
                    if (send(client_socket_fd, reply, strlen(reply)+1, 0) == -1) {
                        perror("send");
                        exit(1);
                    }                    
                } else {
                    char *reply = 
                        "HTTP/1.1 404\n"
                        "Content-Type: text/html\n"
                        "Content-Length:47\n"
                        "Accept-Ranges: bytes\n"
                        "Connection: close\n"
                        "\n"
                        "The requested resource could not be found\n \r\n\r\n";

                    printf("Sending reply...\n");
                    if (send(client_socket_fd, reply, strlen(reply)+1, 0) == -1) {
                        perror("send");
                        exit(1);
                    }                
                }
            }

            free(parsed_request);
            close(client_socket_fd);
			exit(0);
		}
        
		close(client_socket_fd);
	}

    return 0;
}