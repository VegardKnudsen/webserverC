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

#include "listing.h" 

#define PORT 8080
#define BACKLOG 10
#define GET_HEAD_SIZE 2048
#define UID 1000
#define GID 1000

int socketConf();
int deamonize();
int bodyConf(int, char*);
char* getTime();
int interpretRequest(int, char*, char*);

//m2
int getDir(char*);
int getMime(char*, char*, FILE*);
int response(int, char*, FILE*);
int logAccess(int, char*, FILE*);
int mimeSearch(char*, char*);

int main(int argc, char* argv[]){
	int server_sd, client_sd;
	char buf[GET_HEAD_SIZE];

	FILE* err;
	FILE* mime;
	FILE* access;
	char *httpRequest;

	char *filePath;
	char *logPath = "/var/log/webserver_error.log";
	char *accessPath = "/var/log/webserver_access.log";
	char *mimePath = "/etc/mime.types";

	char *newRootDir = "/var/www";

	for(int i = 0; i<argc; i++){
		if(strcmp(argv[i], "--log-file") == 0 && strcmp(argv[i+1], "--chroot-dir") != 0){
			logPath = argv[i+1];
		}
		else if(strcmp(argv[i], "--chroot-dir") == 0 && strcmp(argv[i], "--log-file") != 0){
			newRootDir = argv[i+1];
		}
	}


	//Logs errors
	err = fopen(logPath, "a");
	if(err == 0){
		perror(logPath);
		exit(1);
	}
	dup2(fileno(err), 2);
	fclose(err);

	mime = fopen(mimePath, "r");
	if(access == 0){
		perror(mimePath);
		exit(1);
	}

	access = fopen(accessPath, "a");
	if(access == 0){
		perror(accessPath);
		exit(1);
	}

	if(chroot(newRootDir)){
		perror(newRootDir);
		exit(1);
	}

	//Changes root directory
	//chroot("/var/www");

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
			response(client_sd, filePath, mime);

			//Log access
			logAccess(client_sd, filePath, access);

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

/*
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
*/


int response(int sd, char* filePath, FILE* mime){
	int buffSize = 1024;
	char buff[buffSize];
	int x;
	FILE* fp;

	fp = fopen(filePath, "r");

	if(fp == 0){
		perror(getTime());
		dprintf(sd, "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\nFile not found");
	}
	else{
		x = getMime(filePath, buff, mime);

		if(x == -1){
			write(sd, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n", 44);
			write(sd, "<link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\" />", 65);
			write(sd, "<body>", 6);
			write(sd, "<img src=\"USN_logo.png\" height=\"100\" width=\"100\" />", 57);
			write(sd, "</body>", 7);
		}
		else if(x == -2){
			perror(getTime());
			write(sd, "HTTP/1.1 510 Not Extended\nContent-Type: text/plain\n\nFurther extentions required", 82);
		}
		else if(x == -3){
			perror(getTime());
			write(sd, "HTTP/1.1 415 Unsupported Media Type\nContent-Type: text/plain\n\nMedia type not supported", 89);
		}
		else{
			write(sd, "HTTP/1.1 200 OK\nContent-Type: ", 31);
			write(sd, buff, x);
			write(sd, "\n\n", 4);

			while(x = fread(buff, 1, 1024, fp)){
				write(sd, buff, x);
			}
		}
	}
}

char* getTime(){
	time_t currentTime;
	struct tm *timeInfo;

	time(&currentTime);
	timeInfo = localtime(&currentTime);
	
	return strtok(asctime(timeInfo), "\n");	
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

int getDir(char* path){
	int x;
	int ret;
	char lastChar;

	x = 0;
	lastChar = '/';

	while(path[x]!= '\0'){
		lastChar = path[x];
		x++;
	}

	if(lastChar == '/'){
		return 1;
	}
	else{
		return 0;
	}
}

int mimeSearch(char* str, char* target){
	int x, y;
	int equal;

	equal = 1;
	x = y = 0;

	while(str[x] != '\t'){
		x++;
	}
	
	while(str[x] == '\t'){
		x++;
	}

	while(str[x] != '\0' && str[x] != '\n' || equal){
		if(target[y] == '\0'){
			return 1;
		}
		if(str[x] == target[y] && equal){
			y++;
		}
		else if(str[y] == ' '){
			equal = 1;
		}
		else{
			equal = 0;
			y = 0;
		}
	}
	
	if(equal){
		return 1;
	}
	else{
		return 0;
	}
}

int getMime(char* filePath, char* buff, FILE* mime){
	size_t len = 1024;
	ssize_t read;
	char *ext;
	int sub;

	ext = strrchr(filePath, (int)'.');
	if(ext == NULL){
		perror(getTime());
		return -1;
	}
	*ext++;

	if(mime == NULL){
		perror(getTime());
		return -2;
	}

	while((read = getline(&buff, &len, mime)) != -1){
		if(mimeSearch(buff, ext)){
			printf("%s:%s\n", buff, ext);
			sub = strlen(strchr(buff, (int)'\t'));
			rewind(mime);
			return read - sub;
		}
	}
	rewind(mime);
	return -3;
}

//Logs access to files on system
int logAccess(int s, char* filePath, FILE* access){
	socklen_t len;
	struct sockaddr_storage addr;
	char ipstr[INET6_ADDRSTRLEN];
	int port;

	len = sizeof addr;
	getpeername(s, (struct sockaddr*)&addr, &len);

	if (addr.ss_family == AF_INET) {
		struct sockaddr_in *s = (struct sockaddr_in *)&addr;
		port = ntohs(s->sin_port);
		inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
	} else { // AF_INET6
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
		port = ntohs(s->sin6_port);
		inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
	}

	fprintf(access,"%s tried to access: %s, at: %sn", ipstr, filePath, getTime());
}