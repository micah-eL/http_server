#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include "helper.h"


// Handle SIGCHLD signal - cleans up the child's entry in the process table
void sigchld_handler(int s) {
	(void)s; // For unused variable warning
	int saved_errno = errno; // waitpid() might overwrite errno
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

// Parse HTTP requests
void parse_request(char *request, char ***parsed_request) {
    /*
    Example request (request points to the char 'G', i.e. printing request + 1 yields Request: ET / HTTP... etc.):
        Request: GET / HTTP/1.1
        Host: localhost:12000
        Connection: keep-alive
        Cache-Control: max-age=0
        sec-ch-ua: " Not A;Brand";v="99", "Chromium";v="96", "Google Chrome";v="96"
        sec-ch-ua-mobile: ?0
        sec-ch-ua-platform: "Linux"
        Upgrade-Insecure-Requests: 1
    */
    
    // Grab request method and resource path
    char *start_of_resource_path = strchr(request, ' ') + 1;
    char *end_of_resource_path = strchr(start_of_resource_path, ' '); 
    int request_method_size = start_of_resource_path - 1 - request; // Length is +1 to save space for \0
    int resource_path_size = end_of_resource_path - start_of_resource_path; // Length is +1 to save space for \0
    char *parsed_request_method = malloc(request_method_size);
    char *parsed_resource_path = malloc(resource_path_size);
    strncpy(parsed_request_method, request, request_method_size);
    strncpy(parsed_resource_path, start_of_resource_path, resource_path_size);
    parsed_request_method[request_method_size] = '\0';
    parsed_resource_path[resource_path_size] = '\0';
    (*parsed_request)[0] = parsed_request_method; // Remember multi-dimensional operator precedence
    (*parsed_request)[1] = parsed_resource_path;    // these lines are equivalent to parsed_request[0][0] and parsed_request[0][1]
}

// Load requested file into buffer
void *loadfile(char *file, int *size) {
    FILE *fp;
    long lSize;
    char *buffer;

    fp = fopen(file , "rb");
    if(!fp) {
        perror(file),exit(1);
    }

    fseek(fp , 0L , SEEK_END);
    lSize = ftell( fp );
    rewind(fp);

    // Allocate memory for entire content
    buffer = calloc( 1, lSize+1 );
    if(!buffer) {
        fclose(fp),fputs("memory alloc fails",stderr),exit(1);
    }

    // Copy the file into the buffer
    if(fread(buffer , lSize, 1 , fp) != 1) {
        fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);
    }
    
    size = (int *)lSize;
    fclose(fp);
    return buffer;
}
