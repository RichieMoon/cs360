/**
*
*Richie Zhang
*CS360 final project
*mftp: running commands through server and client
*
**/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#define MY_PORT_NUMBER 49999

int portnumber(int controlfd);
int isregular(char *path);
