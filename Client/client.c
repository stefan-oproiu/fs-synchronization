#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "client.h"

char *ip;
int port;
int socketFileDescriptor;

struct sockaddr_in serverAddress;
char *root;

char errorMessage[PATH_MAX + 100];

void sendMessage() {
    char buff[1024];
    sprintf(buff, "test message.");

    if (write(socketFileDescriptor, buff, strlen(buff)) < 0) {
        snprintf(errorMessage, sizeof(errorMessage), "\nError writing message:%s\n", buff);
        displayError(errorMessage);
    }
    buff[0] = 0;

    int n;
    n = read(socketFileDescriptor, buff, sizeof(buff));
    if (n < 0) {
        snprintf(errorMessage, sizeof(errorMessage), "\nError reading message :\n");
        displayError(errorMessage);
    }
    buff[n] = 0;

    if (n) {
        fprintf(stdout, "%s\n", buff);
    }
}

void handle() {
    char buff[1024];
    char newPath[PATH_MAX];

    FileMetadata *serverFiles;

    int serverFilesCount;
    int n = 0, i, size;

    n = read(socketFileDescriptor, buff, 10);
    if (n < 0) {
        snprintf(errorMessage, sizeof(errorMessage), "\nError reading from socketFileDescriptor in clint.c :\n");
        displayError(errorMessage);
    }
    buff[n] = 0;

    serverFilesCount = atoi(buff);

    #if defined DEBUG_MODE
         printf("[debug - client]: server tells that are %d files there\n", serverFilesCount);
    #endif

    if ((serverFiles = (FileMetadata *) malloc(serverFilesCount * sizeof(FileMetadata))) == NULL) {
        snprintf(errorMessage, sizeof(errorMessage), "\nError allocating memory for server file :\n");
        displayError(errorMessage);
    }

    for (i = 0; i < serverFilesCount; i++) {
        if (read(socketFileDescriptor, &serverFiles[i], sizeof(FileMetadata)) < 0) {
            snprintf(errorMessage, sizeof(errorMessage), "\nError reading server files in client.c :\n");
            displayError(errorMessage);
        }
    }

    getFilesToUpdate(serverFiles, serverFilesCount);

    #if defined DEBUG_MODE
        printf("[debug - client]: %d files need updated/added\n", filesToUpdateCount);
    #endif

    snprintf(buff, 10, "%d", filesToUpdateCount);
    if (write(socketFileDescriptor, buff, 10) < 0) {
        snprintf(errorMessage, sizeof(errorMessage), "\nError writing file to update count  :\n");
        displayError(errorMessage);
    }

    for (i = 0; i < filesToUpdateCount; i++) {
        #if defined DEBUG_MODE
             printf("[debug - client]: sending '%s' to server\n", filesToUpdate[i].path);
        #endif

        if (write(socketFileDescriptor, filesToUpdate[i].path, PATH_MAX)) {
            snprintf(errorMessage, sizeof(errorMessage), "\nError writing file to update  :\n");
            displayError(errorMessage);
        }
    }

    for (i = 0; i < filesToUpdateCount; i++) {
        int idx;
        unsigned int size;
        unsigned long timestamp;

        snprintf(newPath, PATH_MAX, "%s/%s", root, filesToUpdate[i].path);

        #if defined DEBUG_MODE
            printf("[debug - client]: file '%s' is being updated.\n", newPath);
        #endif

        n = read(socketFileDescriptor, buff, 48);
        if(n<0){
            snprintf(errorMessage, sizeof(errorMessage), "\nError reading files to update count in client.c :\n");
            displayError(errorMessage);
        }
        buff[n] = 0;

        //printf("[debug] buff=%s\n", buff);

        if (!filesToUpdate[i].isRegularFile) // is a directory
        {
            mkdir(newPath, ACCESSPERMS);
        }
        else // is a file, get data from socket and write into it
        {
            int fd = open(newPath, O_CREAT | O_TRUNC | O_WRONLY, 0755);
            off_t size;

            if (fd < 0) {
                snprintf(errorMessage, sizeof(errorMessage), "\nError opening file: %s \n", newPath);
                displayError(errorMessage);
            }

            if(read(socketFileDescriptor, buff, 12)<0){
                snprintf(errorMessage, sizeof(errorMessage), "\nError reading from file in client.c: %s :\n", newPath);
                displayError(errorMessage);
            }
            size = strtol(buff, NULL, 10);

            if(read(socketFileDescriptor, buff, size)<0){
                snprintf(errorMessage, sizeof(errorMessage), "\nError reading from file in client.c: %s :\n", newPath);
                displayError(errorMessage);
            }

            if(write(fd, buff, size)<0){
                snprintf(errorMessage, sizeof(errorMessage), "\nError writing from file in client.c: %s :\n", newPath);
                displayError(errorMessage);
            }

            if (close(fd) < 0) {
                snprintf(errorMessage, sizeof(errorMessage), "\nError closing file in client.c: %s :\n", newPath);
                displayError(errorMessage);
            }
        }

        if ((idx = getPathIndex(newPath)) == -1) // file/dir does not exists in the list => add it
        {
            strcpy(ownFiles[pathsCount].path, newPath);
            ownFiles[pathsCount].size = size;
            ownFiles[pathsCount].timeStamp = timestamp;
            pathsCount++;
        }
        else // update its stats
        {
            ownFiles[pathsCount].size = size;
            ownFiles[pathsCount].timeStamp = timestamp;
        }

        #if defined DEBUG_MODE
             printf("[debug - client]: file '%s' successfully updated.\n", newPath);
        #endif
    }
}

void clientSetup() {
    if ((socketFileDescriptor = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ip);
    serverAddress.sin_port = htons(port);

    if (connect(socketFileDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        exit(EXIT_FAILURE);
    }

    getAllFilesPaths(root);
    getAllFilesMetadata(root);

    handle();
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("USAGE: %s <config_file_path> <root>.\n", argv[0]);
        exit(1);
    }

    if (argv[2][strlen(argv[2]) - 1] == '/') {
        printf("[ERROR]: Root shouldn't end with the '/' character.\n");
        exit(1);
    }

    readConfigurationParameters(argv[1]);

    root = argv[2];
    clientSetup();

    return 0;
}