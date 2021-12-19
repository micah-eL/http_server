#ifndef HELPER_H
#define HELPER_H


void sigchld_handler(int);

void parse_request(char *, char ***);

void *loadfile(char *, int *);


#endif