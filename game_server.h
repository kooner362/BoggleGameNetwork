#ifndef GAME_SERVER_H
#define GAME_SERVER_H


#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <math.h>
#include "read_line.h"
#include "generator.h"
#include "game.h"
#define INPUT_BUFFER_SIZE 512
#define INPUT_ARG_MAX_NUM 12
#define DELIM "> \r\n"


int new_connection(int fd, struct sockaddr_in address);
void addclient(int fd, struct in_addr addr);
void removeclient(int fd);
void write_to_socket(char *s, int size, int fd);
void broadcast(char *s, int size);
#endif
