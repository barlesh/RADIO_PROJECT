

#include "control.h"
#define DEBUG			1
#define TCP_PACKET_SIZE		100
#define WELCOME_LEN		3
#define HANDSHAKE_LEN		3

int state,numStations,socket_fd;

int main(int argc, char *argv[])
{
	int socket_fd, station;

	argv[0]="radio_controler";
	argv[1]="132.72.106.236";
	argv[2]="4444";
	argv[3]="5555";
	argc=4;

	if(argc!=4) {printf("wrong number of arguments(%d)\n",argc); exit(-1);}
	if (argv[1] == NULL){fprintf(stderr,"ERROR, no such host\n");exit(-1);}
	state=0;
	socket_fd=0;

	while(state!=-1){
		switch(state){
		case 0:
			if(DEBUG){printf("start state %d\n", state);}
			/*init socket and connect to server*/
			state=state_connect( argv);
			break;

		case 1:
			if(DEBUG){printf("start state %d\n", state);}
			state=state_handshake(argv);

			break;

		case 2:
			if(DEBUG){printf("start state %d\n", state);}
			/*got welcome massege now need to ask for a station*/
			state=state_setStation(&station);
			/*setStation massage sent succesfully*/
			break;

		case 3:
			if(DEBUG){printf("start state %d\n", state);}
			/*wait for announce stdin off*/
			state=state_wait_for_annouce(station);
			/*got announce! back to state 2 (wait for new input from user (new station))*/
			break;

		}//switch
	}//while
}//main


/**
 * state 0
 *
 */
int state_connect(char ** argv )
{
	int try=0;
	struct sockaddr_in my_addr;

	if(state!=0){printf("something very bad happened");close(socket_fd);}

	/*init socket*/
	if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)	{perror("Client-socket() error");exit(-1);}
	else{if(DEBUG){printf("Client-socket() OK\n");}}
	/*bind*/
	memset((char*)&my_addr, '0' , sizeof(my_addr));
	my_addr.sin_family= AF_INET;
	my_addr.sin_port=htons(atoi(argv[3]));
	my_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(socket_fd, (struct sockaddr *)&my_addr, sizeof(my_addr))==-1) {perror("bind()");}
	if(DEBUG){printf("bind Succeeded! \n");}
	/*connect to server*/
	while((connect_to_server(argv))!=1)	{
		try++;
		if(try==5)	{try=0;printf("tried to connect for 5 times. stop trying\n");exit(-1);}
	}
	return 1;
	/*connected*/
}

int state_handshake(char ** argv )
{
	fd_set sockset;
	struct timeval timeout;
	int ans, try,massage_index;
	char buff[TCP_PACKET_SIZE], s_buff[2];

	numStations=0;
	FD_ZERO(&sockset);
	FD_SET(socket_fd, &sockset);
	/*
	if(select(socket_fd + 1, &sockset, NULL, NULL, NULL)==-1) {perror("select():");}

	else
		if (FD_ISSET(socket_fd, &sockset)){	//not suppose to recieve anything now
			close(socket_fd); return 0;

		}
	 */
	if(DEBUG){printf("pass select\n");}
	while(send_Hallo(argv)==0){try++; if(try==5){printf("tried to send hello for 5 times. stop trying\n");exit(-1);} }
	timeout.tv_sec = 0;
	timeout.tv_usec = 200000;	//0.2 [sec]
	ans=select(socket_fd+1, &sockset, NULL,NULL,&timeout);
	if(ans==-1) {perror("select():");}
	if(ans==0)	{printf("time out! - disconnecting\n"); close(socket_fd); return 0;}
	if(ans>0){	//recieved data
		if((ans=recv(socket_fd, buff, TCP_PACKET_SIZE, 0))==-1) {perror("recv():");}
		if(DEBUG){printf("ans is: %d\n",ans);}
		if(ans!=WELCOME_LEN || buff[0]!=0 ) {
			if(buff[0]==2){print_invalid_command(buff);}
			else
			{printf("wrong massege recived. disconnecting\n"); close(socket_fd); }
			return 0;
		}
	}
	/*got welcome - ready to start*/
	buff[3]='\0';
	if(DEBUG){printf("buff[1] is: %hu\n",((uint16_t)buff[1]));}
	if(DEBUG){printf("buff[2] is: %hu\n",buff[2]);}

	numStations=((uint16_t)buff[1])*256+((uint16_t)buff[2]);
	if(DEBUG){printf("numStations is: %d\n",numStations);}

	return 2;
}

int state_setStation(int* station)
{
	fd_set sockset;
	int ans, try, flag=1;
	char buff[TCP_PACKET_SIZE];
	printf("please insert station number between 1-%d\n", numStations);
	FD_ZERO(&sockset);
	FD_SET(socket_fd, &sockset);
	FD_SET(0, &sockset);
	while (flag){
		ans=select(socket_fd+1, &sockset, NULL,NULL,NULL);
		if(ans==-1) {perror("select():");}
		if (FD_ISSET(socket_fd, &sockset)){
			if(buff[0]==2){print_invalid_command(&buff);}
			else
				printf("wrong massage received. disconnecting\n"); close(socket_fd); return 0;
		}
		if (FD_ISSET(0, &sockset)){
			if(read(0, buff, 50)==-1){perror("read():");}
			if((buff[0]=='q') ) {	printf("QUIT. goodbye\n"); exit(-1);}
			//########################################################## TODO
			if (atoi(buff)>numStations || atoi(buff)<1){
				printf("station not exist or invalid input. try again\n");
				continue;
			}
			else
				flag=0;

			*station=atoi((char*)buff);
			while(send_SetStation(*station)==0){try++; if(try==5){printf("tried to send SetStation for 5 times. stop trying\n");exit(-1);} }
			/*SetStation sent successfully!- move to state 3 - wait for announce of new SetStation*/
			return 3;
		}
	}
	return 0;
}

int state_wait_for_annouce(int station)
{
	fd_set sockset;
	int ans, n;
	struct timeval timeout;
	char buff[TCP_PACKET_SIZE];
	FD_ZERO(&sockset);
	FD_SET(socket_fd, &sockset);
	timeout.tv_sec = 0;
	timeout.tv_usec = 2000000;	//2 [sec]
	ans=select(socket_fd+1, &sockset, NULL,NULL,&timeout);
	if(ans==-1) {perror("select():");}
	if(ans==0) {printf("time out! - disconnecting\n"); close(socket_fd); return 0;}

	if (FD_ISSET(socket_fd, &sockset)){	//Check if announce
		memset((void*)&buff, 0, TCP_PACKET_SIZE);
		if( (ans=recv(socket_fd, &buff, 2, 0))==-1) {perror("recv():");}
		if(((uint8_t)buff[0])!=1) {printf("wrong massage received. disconnecting\n"); close(socket_fd); return 0;}
		/*got announce - get name and back to state 2*/
		n=buff[1];
		memset((void*)&buff, 0, TCP_PACKET_SIZE);
		read(socket_fd, &buff, n);
		buff[n]='\0';
		printf("playing station number %d - song name %s\n", station, &buff);
		return 2;

	}
	return 0;
}



void print_invalid_command(char* buff)
{

}
int send_Hallo(char ** argv)
{

	int ans, n;
	uint8_t buff[HANDSHAKE_LEN];
	client_command hello;
	uint8_t msb, lsb;

	hello.command_type=0;
	hello.data=atoi(argv[3]);

	msb=(uint8_t)( hello.data /256);
	lsb = ((uint8_t)hello.data & 0X00FF);

	buff[0]=hello.command_type;
	buff[1]=msb;
	buff[2]=lsb;

	if((ans=send(socket_fd, buff, HANDSHAKE_LEN, 0))==-1) {
		perror("send():");
		return 0;
	}
	return 1;


}

int send_SetStation(int station)
{
	int ans, n;
	uint8_t buff[HANDSHAKE_LEN];
	client_command station_s;
	uint8_t msb, lsb;

	station_s.command_type=1;
	station_s.data=station;

	msb=(uint8_t)( station_s.data /256);
	lsb = ((uint8_t)station_s.data & 0X00FF);

	buff[0]=station_s.command_type;
	buff[1]=msb;
	buff[2]=lsb;

	if((ans=send(socket_fd, buff, HANDSHAKE_LEN, 0))==-1) {
		perror("send():");
		return 0;
	}
	return 1;


}

int connect_to_server(char ** argv){
	char servername[50];
	struct sockaddr_in serv_addr;
	struct hostent *hostp;
	int serverPort=atoi(argv[2]);
	/*set parameters to connect to server by socket_fd*/

	strcpy(servername, argv[1]);
	memset(&serv_addr, 0, sizeof(serv_addr));
	/*i think that if host name isn't good address we try to find it at net*/
	if((serv_addr.sin_addr.s_addr=inet_addr(servername)) == INADDR_NONE){
		if(DEBUG){printf("going to search addr at net\n");}
		if((hostp=gethostbyname(servername))==NULL){printf("server not found!\n");return -1;}
		else{memcpy(&serv_addr.sin_addr, hostp->h_addr, sizeof(serv_addr.sin_addr));}
	}
	if(DEBUG){printf("pass address of server\n");}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(serverPort);
	/*connect to server*/
	if(connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		perror("Client-connect() error");	close(socket_fd);	return(-1);
	}
	else{printf("Connection established...\n");	return 1;}
	return 0;
}
