#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include "../Common/common.h"
#include "server.h"

int socketFileDescriptor;
struct sockaddr_in serverAddress, remoteAddress;
socklen_t len;

char *root;
char errorMessage[PATH_MAX + 100];

off_t getFileSize(char *path) {


    struct stat entranceInfo;

    if (lstat(path, &entranceInfo) < 0) {
        snprintf(errorMessage, sizeof(errorMessage), "\nError at lstat: %s\n", path);
        displayError(errorMessage);
    }

    return entranceInfo.st_size;
}

void handleRequest(int connectionFileDescriptor) {
    char buff[PATH_MAX], newPath[PATH_MAX];
    int n;
    int filesToUpdateCount;

    FileMetadata *filesToUpdate;

    // Tell the client first how many files are on the server

    snprintf(buff, 10, "%d", pathsCount);

    if (write(connectionFileDescriptor, buff, 10) < 0) {
        snprintf(errorMessage, sizeof(errorMessage), "\nError while writing path count from the server:\n");
        displayError(errorMessage);
    }

    for (int i = 0; i < pathsCount; i++) {

        if (write(connectionFileDescriptor, &ownFiles[i], sizeof(FileMetadata)) < 0) {
            snprintf(errorMessage, sizeof(errorMessage), "\nError while writing some FileMetadata from the server :\n");
            displayError(errorMessage);
        }

    }

    n = read(connectionFileDescriptor, buff, 10);
    if (n < 0) {
        snprintf(errorMessage, sizeof(errorMessage), "\nError while reading from connectionFileDescriptor");
        displayError(errorMessage);
    }
    buff[n] = 0;

    filesToUpdateCount = atoi(buff);

    #if defined DEBUG_MODE
        printf("[debug - server]: client needs %d files updated.\n", filesToUpdateCount);
    #endif

    if ((filesToUpdate = (FileMetadata *) malloc(filesToUpdateCount * sizeof(FileMetadata))) == NULL) {
        snprintf(errorMessage, sizeof(errorMessage), "\nError while allocating memory to filesToUpdate :\n");
        displayError(errorMessage);
    }

    for (int i = 0; i < filesToUpdateCount; i++) {
        n = read(connectionFileDescriptor, buff, PATH_MAX);
        if (n < 0) {
            snprintf(errorMessage, sizeof(errorMessage), "\nError while reading filesToUpdate");
            displayError(errorMessage);
        }
        buff[n] = 0;

         #if defined DEBUG_MODE
             printf("[debug - server]: client needs file '%s' updated.\n", buff);
         #endif

        filesToUpdate[i] = ownFiles[getPathIndex(buff)];
    }

    for (int i = 0; i < filesToUpdateCount; i++) {
        snprintf(newPath, PATH_MAX, "%s/%s", root, filesToUpdate[i].path);

        #if defined DEBUG_MODE
             printf("[debug - server]: file '%s' is updating to client.\n", newPath);
        #endif
        snprintf(buff, 48, "%lu %lu", filesToUpdate[i].size, filesToUpdate[i].timeStamp);

        if(write(connectionFileDescriptor, buff, 48)<0){
            snprintf(errorMessage, sizeof(errorMessage), "\nError while writing filesToUpdate stuff");
            displayError(errorMessage);
        }

        if (filesToUpdate[i].isRegularFile) // is not a directory
        {
            int fd;
            int n;

            if ((fd = open(newPath, O_RDONLY)) == -1) {
                snprintf(errorMessage, sizeof(errorMessage), "\nError while opening file :%s\n", newPath);
                displayError(errorMessage);
            }

            off_t size = getFileSize(newPath);

            #if defined DEBUG_MODE
                 printf("[debug - server]: file '%s', size=%ld\n", newPath, size);
            #endif

            snprintf(buff, 12, "%ld", size);
            if(write(connectionFileDescriptor, buff, 12)<0){
                snprintf(errorMessage, sizeof(errorMessage), "\nError while writing filesToUpdate stuff 2");
                displayError(errorMessage);
            }

            n=read(fd, buff, size);
            if(n<0){
                snprintf(errorMessage, sizeof(errorMessage), "\nError while reading filesToUpdate stuff 2");
                displayError(errorMessage);
            }

            if(write(connectionFileDescriptor, buff, size)<0){
                snprintf(errorMessage, sizeof(errorMessage), "\nError while writing filesToUpdate stuff 3");
                displayError(errorMessage);
            }

            if (close(fd) < 0) {
                snprintf(errorMessage, sizeof(errorMessage), "\nError while closing update file:%s\n",newPath);
                displayError(errorMessage);
            }

            #if defined DEBUG_MODE
                 printf("[debug - server]: file '%s' successfully updated.\n", filesToUpdate[i].path);
            #endif
        }
    }

    if(close(connectionFileDescriptor)<0){
        snprintf(errorMessage, sizeof(errorMessage), "\nError while closing connection file descriptor");
        displayError(errorMessage);
    }
    exit(0);
}

void acc() {
    len = sizeof(remoteAddress);

    int connectionFileDescriptor = accept(socketFileDescriptor, (struct sockaddr *) &remoteAddress, &len);
    pid_t pid;

    if ((pid = fork()) == 0) {
        printf("pid %d\n", getpid());
        handleRequest(connectionFileDescriptor);
    } else {
        acc();
    }
}

void serverSetup() {
    if ((socketFileDescriptor = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        snprintf(errorMessage, sizeof(errorMessage), "\nError while server setup :\n");
        displayError(errorMessage);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ip);
    serverAddress.sin_port = htons(port);

    if (bind(socketFileDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        snprintf(errorMessage, sizeof(errorMessage), "\nBinding failure :\n");
        displayError(errorMessage);
    }

    listen(socketFileDescriptor, 5);

    getAllFilesPaths(root);
    getAllFilesMetadata(root);

    acc();
}

int main(int argc, char *args[]) {
    if (argc != 3) {
        printf("Usage %s <config_file_path> <root>.\n", args[0]);
        exit(1);
    }

    // signal(SIGCHLD, SIG_IGN);

    readConfigurationParameters(args[1]);
    root = args[2];
    serverSetup();

    return 0;
}