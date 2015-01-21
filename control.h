

#ifndef CONTROL_H_
#define CONTROL_H_

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <time.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>

typedef struct client_command{
	uint8_t command_type;
	uint16_t data;
}client_command;

int connect_to_server(char ** argv);
void print_invalid_command(char* buff);
int send_Hallo(char ** argv);
int send_SetStation(int station);

int state_connect(char ** argv);
int state_handshake(char ** argv);
int state_setStation(int* station);

#endif /* CONTROL_H_ */
