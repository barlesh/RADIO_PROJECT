/*
 * listener.c
 *
 *  Created on: Jan 7, 2015
 *      Author: barlesh
 */



#include "listener.h"

#define UDP_PACKET_SIZE		1024
#define DEBUG				1

void main(int argc,char* argv[])
{
	char buff[UDP_PACKET_SIZE];
	char arg[20];
	int num_of_byte;
	int socket_fd;
	struct sockaddr_in my_addr;
	int addr_len = sizeof(my_addr);
	uint16_t udpPort;

	if(argc!=2) {printf("error! wrong number of arguments(%d)\n", argc); exit(1);}
	udpPort=atoi((char*)argv[1]);

	if((socket_fd=socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP))==-1) {perror("socket");}

	memset((char*)&my_addr, '0' , sizeof(my_addr));
	my_addr.sin_family= AF_INET;
	my_addr.sin_port=htons(udpPort);
	my_addr.sin_addr.s_addr=htonl(INADDR_ANY);

	if(bind(socket_fd, (struct sockaddr *)&my_addr, sizeof(my_addr))==-1) {perror("bind");}

	while(1){
		if((num_of_byte=recvfrom(socket_fd, buff, UDP_PACKET_SIZE, 0, &my_addr, &addr_len))==-1) {perror("recvfrom");}
		if(write(1, buff, num_of_byte)<0) {perror("write");}
	}
}
