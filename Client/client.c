#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"

char *ip;
int port;
int socketFileDescriptor;

struct sockaddr_in serverAddress;
char *root;

char errorMessage[PATH_MAX + 100];

char *formatdate(time_t val) {
    char *str;

    if ((str = (char *) malloc(48)) == NULL)
        displayError("malloc() error.");

    strftime(str, 48, "%Y%m%d%H%M.%S", localtime(&val));
    return str;
}

int adjustTimestamp(char *filepath, char *timestamp) {
    char cmd[PATH_MAX + 32];
    int status, exitcode;

    snprintf(cmd, sizeof(cmd), "touch -a -m -t %s \"%s\"", timestamp, filepath);

    status = system(cmd);
    exitcode = WEXITSTATUS(status);

    return exitcode;
}

int getIndexFromServerFiles(FileMetadata *serverFiles, int count, char *path) {
    int i;

    for (i = 0; i < count; i++) {
        if (!strcmp(serverFiles[i].path, path))
            return i;
    }

    return -1;
}

int removeDirectory(char *directoryPath) {
    char cmd[PATH_MAX + 64];
    int status, exitcode;

    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", directoryPath);

    status = system(cmd);
    exitcode = WEXITSTATUS(status);

    return exitcode;
}

int removeFile(char *path) {
    char cmd[PATH_MAX + 64];
    int status, exitcode;

    snprintf(cmd, sizeof(cmd), "rm \"%s\"", path);

    status = system(cmd);
    exitcode = WEXITSTATUS(status);

    return exitcode;
}

void getFilesToDelete(FileMetadata *serverFiles, int serverFilesCount) {
    filesToDelete = (FileMetadata *) malloc(sizeof(FileMetadata));

    for (int i = 0; i < pathsCount; i++) {
        int toDelete = 1;

        for (int j = 0; j < serverFilesCount; j++) {
            if (!strcmp(ownFiles[i].path, serverFiles[j].path)) {
                toDelete = 0;
                break;
            }
        }

        if (toDelete) {
            filesToDelete = (FileMetadata *) realloc(filesToDelete, sizeof(FileMetadata) * (++fileToDeleteCount));
            filesToDelete[fileToDeleteCount - 1] = ownFiles[i];
        }
    }
}

void getFilesToUpdate(FileMetadata *serverFiles, int serverFilesCount) {
    filesToUpdate = (FileMetadata *) malloc(sizeof(FileMetadata));

    for (int i = 0; i < serverFilesCount; i++) {
        int different = 0;
        int found = 0;

        for (int j = 0; j < pathsCount; j++) {
            if (!strcmp(serverFiles[i].path, filesToUpdate[j].path)) {
                found = 1;

                if (serverFiles[i].size != filesToUpdate[j].size ||
                    serverFiles[i].timeStamp != filesToUpdate[j].timeStamp ||
                    serverFiles[i].isRegularFile != filesToUpdate[j].isRegularFile) {
                    different = 1;
                    break;
                }
            }
        }

        if (different || !found) {
            filesToUpdate = (FileMetadata *) realloc(filesToUpdate, sizeof(FileMetadata) * (++filesToUpdateCount));
            filesToUpdate[filesToUpdateCount - 1] = serverFiles[i];
        }
    }
}


void handle() {
    char buff[1024];
    char newPath[PATH_MAX];

    FileMetadata *serverFiles;

    int serverFilesCount;
    int n = 0, i, size;

    /*
	* Receiving files count from the server.
	*/
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

    /*
    * Allocate memory for the server files to check if there are outdated/files that need to be deleted.
    */
    if ((serverFiles = (FileMetadata *) malloc(serverFilesCount * sizeof(FileMetadata))) == NULL) {
        snprintf(errorMessage, sizeof(errorMessage), "\nError allocating memory for server file :\n");
        displayError(errorMessage);
    }

    /*
	* Reading them from the socket.
	*/
    for (i = 0; i < serverFilesCount; i++) {
        if (read(socketFileDescriptor, &serverFiles[i], sizeof(FileMetadata)) < 0) {
            snprintf(errorMessage, sizeof(errorMessage), "\nError reading server files in client.c :\n");
            displayError(errorMessage);
        }
    }

    /*
	* Getting files that need to be updated/deleted.
	*/
    getFilesToUpdate(serverFiles, serverFilesCount);
    getFilesToDelete(serverFiles, serverFilesCount);

    /*
	* Telling the server how many files need updated by the client.
	*/
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

    /*
	* Updating the files from the server.
	*/
    for (i = 0; i < filesToUpdateCount; i++) {
        int idx;
        int serverFileIdx = getIndexFromServerFiles(serverFiles, serverFilesCount, filesToUpdate[i].path);

        /*
        * Building the absolute path.
        */

        snprintf(newPath, PATH_MAX, "%s/%s", root, filesToUpdate[i].path);

        #if defined DEBUG_MODE
            printf("[debug - client]: file '%s' is being updated.\n", newPath);
        #endif


        if (!filesToUpdate[i].isRegularFile) // is a directory
        {
            mkdir(newPath, ACCESSPERMS);
        }
        else // is a file, get data from socket and write into it
        {
            int fd = open(newPath, O_CREAT | O_TRUNC | O_WRONLY, 0744);
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

        char *date = formatdate(serverFiles[serverFileIdx].timeStamp);
        adjustTimestamp(newPath, date);

        #if defined DEBUG_MODE
              printf("[debug - client]: timestamp=%s for file %s\n", date, filesToUpdate[i].path);
        #endif

        if ((idx = getPathIndex(newPath)) == -1) // file/dir does not exists in the list => add it
        {
            strcpy(ownFiles[pathsCount].path, newPath);

            ownFiles[pathsCount].size = serverFiles[serverFileIdx].size;
            ownFiles[pathsCount].timeStamp = serverFiles[serverFileIdx].timeStamp;

            pathsCount ++;
        }
        else // update its stats
        {
            ownFiles[idx].size = serverFiles[serverFileIdx].size;
            ownFiles[idx].timeStamp = serverFiles[serverFileIdx].timeStamp;
        }

        #if defined DEBUG_MODE
             printf("[debug - client]: file '%s' successfully updated.\n", newPath);
        #endif

        if (date) {
            free(date);
        }
    }

    /*
    * Deleting the files that are on the clients' machine but not on the server.
    */

    for (i = 0; i < fileToDeleteCount; i++)
    {
        int idx, j;

        snprintf(newPath, PATH_MAX, "%s/%s", root, filesToDelete[i].path);

        #if defined DEBUG_MODE
             printf("[debug - client]: file '%s' is being deleted.\n", newPath);
        #endif

        if (!filesToDelete[i].isRegularFile) // is a directory
        {
            removeDirectory(newPath);
        }
        else // is a file, remove it
        {
            if (strlen(newPath) == strlen(root) + strlen(filesToDelete[i].path) + 1)
                removeFile(newPath);
        }

        if ((idx = getPathIndex(newPath)) != -1)
        {
            for (j = idx; j < pathsCount - 1; j++)
                ownFiles[j] = ownFiles[j + 1];

            ownFiles --;
        }

        #if defined DEBUG_MODE
             printf("[debug - client]: file '%s' successfully deleted.\n", newPath);
        #endif
    }

    if (serverFiles) {
        free(serverFiles);
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