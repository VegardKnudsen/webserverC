#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int writeLabel(int sd, char* tagName, char* contentType){
    char* buff;

    buff = malloc(5000);

    strcpy(buff, "<");
    strcat(buff, tagName);
    strcat(buff, ">");

    strcat(buff, contentType);
    strcat(buff, "</");
    strcat(buff, tagName);
    strcat(buff, ">");

    write(sd, buff, strlen(buff));

    free(buff);

    return 0;
}

//Writes clickable filepaths in a table
int writeATags(int sd, char* temp, char* filePath){
    char* buff;

    buff = malloc(5000);

    strcpy(buff, "<td>");
    strcat(buff, "<a href ='");
    strcat(buff, filePath);

    if(filePath[strlen(filePath) - 1] != '/'){
        strcat(buff, "/");
    }

    strcat(buff, temp);
    strcat(buff, "'>");

    strcat(buff, temp);
    strcat(buff, "</a></td>");

    printf("%s\n", buff);

    write(sd, buff, strlen(buff));

    free(buff);

    return 0;
}

//Displays directory in a table
void directoryList(int sd, char* filePath){
    struct stat     stat_buffer;
    struct dirent   *ent;
    DIR             *dir;
    char            *temp;
    char            *buff;

    if((dir = opendir(filePath)) == NULL){
        perror("");
        exit(1);
    }

    chroot("/var/www");

    chdir(filePath);

    buff = malloc(2000);
    temp = malloc(200);

    strcpy(buff, "Catalogue :");
    strcat(buff, filePath);
    writeLabel(sd, "h1", buff);

    write(sd, "<table>", 7);

    write(sd, "<tr>", 4);
    writeLabel(sd, "th", "Rights");
    writeLabel(sd, "th", "UID");
    writeLabel(sd, "th", "GID");
    writeLabel(sd, "th", "Name");
    write(sd, "</tr>", 5);

    while((ent = readdir(dir)) != NULL){
        if(stat (ent->d_name, &stat_buffer) < 0){
            perror("");
            exit(2);
        }

        write(sd, "<tr>", 4);

        sprintf(temp, "%o", stat_buffer.st_mode & 0777);
        writeLabel(sd, "td", temp);
        sprintf(temp, "%d", stat_buffer.st_uid);
        writeLabel(sd, "td", temp);
        sprintf(temp, "%d", stat_buffer.st_gid);
        writeLabel(sd, "td", temp);
        sprintf(temp, "%s", ent->d_name);

        writeATags(sd, temp, filePath);
    }
    write(sd, "</tables", 8);

    free(temp);
    free(buff);
    closedir(dir);
}
