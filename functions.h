
#ifndef _INCLUDE_FUNCTIONS_H
#define _INCLUDE_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <limits.h>

#define FAILURE 1
#define SUCCESS 0



char *stringAllocExing(int len);
char *stringAllocReturning(int len,int *result);
void *bufferAllocReturning(int len,int *result);
int Send(int socket,void *buffer,int len, int flags, int *epipeError);
int checkPort(char *port);

#endif


