#ifndef HELPER_H
#define HELPER_H


void sigchld_handler(int);

void parse_request(char *, char ***);

void *load_file(char *, int *);


#endif