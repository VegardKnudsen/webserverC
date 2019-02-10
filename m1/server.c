#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>

#define PORT 80
#define BACKLOG 10
#define GET_HEAD_SIZE 2048
#define UID 1000
#define GID 1000

int socketConf();
int deamonize();
int bodyConf(int, char*);
char* getTime();
int interpretRequest(int, char*, char*);

int main(){
	int server_sd, client_sd;
	char buf[GET_HEAD_SIZE];

	FILE* err;
	char *httpRequest;
	char *filePath;

	//Logs errors
	err = fopen("/var/log/webserver_error.log", "a");
	if(err == 0){
		perror("/var/log/webserver_error.log");
		exit(1);
	}
	dup2(fileno(err), 2);
	fclose(err);

	//Changes root directory
	chroot("/var/www");

	//Configures socket for server
	server_sd = socketConf();

	//Deamonizes the process
	deamonize();

	listen(server_sd, BACKLOG);
	while(1){
		client_sd = accept(server_sd, NULL, NULL);

		if(fork() == 0){
			//Allocates memory space for request and filepath
			httpRequest = malloc(7);
			filePath = malloc(1000);

			//Request
			interpretRequest(client_sd, httpRequest, filePath);

			//Response
			bodyConf(client_sd, filePath);

			//Terminate
			shutdown(client_sd, SHUT_RDWR);
			exit(0);
		}
		else{
			close(client_sd);
		}
	}
	//Main end
	return 0;
}

int socketConf(){
	int sd;
	struct sockaddr_in server_address;

	sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	
	server_address.sin_family			= AF_INET;
	server_address.sin_port				= htons((u_short)PORT);
	server_address.sin_addr.s_addr		= htonl(INADDR_ANY);

	if(bind(sd, (struct sockaddr*)&server_address, sizeof(server_address)) != 0){
		perror(getTime());
		exit(1);
	}

	setuid((uid_t) UID);
	setgid((gid_t) GID);

	return sd;
}

int deamonize(){
	if(fork()){
		raise(SIGSTOP);
		exit(0);
	}

	setsid();
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);

	if(fork()){
		exit(0);
	}

	close(0);

	return 0;
}

int bodyConf(int sd, char* filePath){
	FILE *fp;
	int buffer_size = 1024;
	char* buff[buffer_size];
	int x;

	fp = fopen(filePath, "r");

	if(fp == 0){
		perror(getTime());
		dprintf(sd, "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\nFile not found");
	}
	else{
		x = 1;
		while(x = fread(buff, 1, 1024, fp)){
			write(sd, buff, x);
		}
	}
}

char* getTime(){
	time_t curtime = time(NULL);
	struct tm *loc_time;

	loc_time = localtime(&curtime);
	
	return ("%s %s\n", strtok(asctime(loc_time), "\n"));	
}

int interpretRequest(int sd, char* httpRequest, char* filePath){
	char* buff;
	char* token;

	buff = malloc(4096);

	read(sd, buff, 4096);
	token = strtok(buff, " ");

	strcpy(filePath, "/");

	strcpy(httpRequest, token);
	strcat(filePath, strtok(NULL, " "));

	free(buff);

	return 0;
}
