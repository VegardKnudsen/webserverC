#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "listing.h"

#define BACKLOG 20
#define PORT 80
#define GET_HEAD_SIZE 2048
#define UID 1000
#define GID 1000

int socketConf();
int deamonize();
char* getTime();
int interpretRequest(int, char*, char*);

int isDir(char*);
int getMime(char*, char*, FILE*);
//int logAccess(int, char*, FILE*);
int response(int, char*, FILE*);

int main(int argc, char* argv[]){

	int server_sd, client_sd;
	char buf[GET_HEAD_SIZE];
	FILE* err;
	FILE* mime;
	//FILE* access;
	
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	char *httpRequest;
	char *filePath;
	char *logPath = "/var/log/webserver_error.log";
	//char *accessPath = "/var/log/webserver_access.log";
	char *newRootDir = "/var/www";
	char *mimePath = "/etc/mime.types";

	for (int i = 0; i<argc; i++){
		if (strcmp(argv[i], "--log-file") == 0 && strcmp(argv[i+1], "--chroot-dir") != 0) {
			logPath = argv[i+1];
		} 
        else if(strcmp(argv[i], "--chroot-dir") == 0 && strcmp(argv[i], "--log-file") != 0) {
			newRootDir = argv[i+1];
		}
	}

	err = fopen(logPath, "a");
	if(err == 0){
		perror(logPath);
		exit(1);
	}
	dup2(fileno(err), 2);
	fclose(err);

	mime = fopen(mimePath, "r");
	if(mime == 0){
		perror(mimePath);
		exit(1);
	}

    /*
	access = fopen(accessPath, "a");
	if(access == 0){
		perror(accessPath);
		exit(1);
	}
    */
	
	if(chroot(newRootDir)){
		perror(newRootDir);
		exit(1);
	}
	
	server_sd = socketConf();
	deamonize();
	listen(server_sd, BACKLOG);

	while(1){
		client_sd = accept(server_sd, NULL, NULL);		
		// Child
		if(fork()==0){
			httpRequest = malloc(7);
			filePath = malloc(1000);
			
			interpretRequest(client_sd, httpRequest, filePath);

			response(client_sd, filePath, mime);

			//logAccess(client_sd, filePath, access);

			shutdown(client_sd, SHUT_RDWR);
			exit(0);
		}

		else{
			close(client_sd);
		}
	}

	return 0;
}

int socketConf(){
	int sd;
	struct sockaddr_in server_address;

	sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	

	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons((u_short)PORT);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sd, (struct sockaddr *)&server_address, sizeof(server_address)) != 0){
		perror(getTime());
		exit(1);		
	}

	setuid((uid_t) UID );
	setgid((gid_t) GID );

	return sd;
}

int deamonize(){

	if(fork()){
		raise(SIGSTOP);
		exit(0); //parent dies
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

int response(int sd, char* filePath, FILE* mime){
	int buffSize = 1024;
	char buff[buffSize];
	int x;
    
	FILE *fp;

	fp = fopen(filePath, "r");

	if(fp==0){
		perror(getTime());
		write(sd, "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\nFile not found", 63);
	}
	else{

		x = getMime(filePath, buff, mime);
		// error handling
		if(x == -1) { //. not in string, guess it is a folder
			write(sd, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n", 41);
			write(sd, "<link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\" /> ", 60);
			write(sd, "<body>", 6);
			//write(sd, "<img src=\"USN_logo.png\" height=\"100\" width=\"100\"/>", 50);
            dprintf(sd, "<img src=\"USN_logo.png\" height=\"200px\" width=\"200px\"/>");
			directoryList(sd, filePath);
			write(sd, "</body>", 7);
		} else if (x == -2) {
			perror(getTime());
			write(sd, "HTTP/1.1 510 Not Extended\nContent-Type: text/plain\n\nFurther extentions required", 79);
		} else if (x == -3) {
			perror(getTime());
			write(sd, "HTTP/1.1 415 Unsupported Media Type\nContent-Type: text/plain\n\nMedia type not supported", 86);
		} else {
			write(sd, "HTTP/1.1 200 OK\nContent-Type: ", 30);
			write(sd, buff, x);
			write(sd, "\n\n", 2);
			// Body
			while (x = fread(buff, 1, 1024, fp)) {
	 			write(sd, buff, x);	
			}
		}
	}
}

char* getTime(){
	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	return strtok(asctime(timeinfo), "\n");
}

int interpretRequest(int sd, char* httpRequest, char* filePath){
	char* buff;
	char* token;

	buff = malloc(5000);

	read(sd, buff, 5000);
	token = strtok(buff, " ");	

	// method of splitting string
	strcpy(httpRequest, token);
	strcpy(filePath, strtok(NULL, " "));

	free(buff);

	return 0;
}

int isDir(char* path){
	int i;
	int ret;
	char lastChar;


	i = 0;
	lastChar = '/';

	while(path[i] != '\0'){
		lastChar = path[i];
		i++;
	}

	if(lastChar == '/')
		return 1;
	
	else
		return 0;

}

int mimeSearch(char* str, char* target){
	int i, j;
	int equal;

	equal = 1;
	i = j = 0;

	while(str[i] != '\t')
		i++;

	while(str[i] == '\t')
		i++;

	while(str[i] != '\0' && str[i] != '\n' || equal) {
		if(target[j] == '\0')
			return 1; // true
		if(str[i] == target[j] && equal)
			j++;
		else if(str[i] == ' ')
			equal = 1;
		else{ // uequale bokstaver
			equal = 0;
			j = 0;
		}
		i++;
	}
	if(equal)
		return 1;
	else
		return 0; // not found
}

int getMime(char* filePath, char* buff, FILE* mime){

	size_t len = 1024;
	ssize_t read;

	char *ext;
	int sub;
	
	ext = strrchr(filePath, (int)'.');
	if (ext == NULL) {
		perror(getTime());
		return -1;
	}
	*ext++;

	if (mime == NULL) {
		perror(getTime());
		return -2;
	}

	while ((read = getline(&buff,&len, mime)) != -1) {
		if (mimeSearch(buff, ext)){
			printf("%s:%s\n", buff, ext);
			sub = strlen(strchr(buff, (int)'\t'));
			rewind(mime);
			return read - sub;			
		}
	}
	rewind(mime);

	return -3;
}
/*
int logAccess(int s, char* filePath, FILE* access){
	socklen_t len;
	struct sockaddr_storage addr;
	char ipstr[INET6_ADDRSTRLEN];
	int port;

	len = sizeof addr;
	getpeername(s, (struct sockaddr*)&addr, &len);

	// deal with both IPv4 and IPv6:
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
*/