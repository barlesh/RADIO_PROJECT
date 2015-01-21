/*
 * radio_listener.h
 *
 *  Created on: Jan 6, 2015
 *      Author: barlesh
 */

#ifndef RADIO_LISTENER_H_
#define RADIO_LISTENER_H_

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






uint16_t get_command(char *command,char**  args);
int parse_command(char* cmd, char** args);
uint16_t get_command(char *command,char**  args);

#endif /* RADIO_LISTENER_H_ */
