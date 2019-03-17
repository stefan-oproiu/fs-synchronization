#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

void displayError(char *msg) {

    fprintf(stderr, "%s:%s\n", msg, strerror(errno));
    exit(errno);
}

int isEmptyDirectory(char *directoryPath) {

    char cmd[1024];
    int status, exitcode;
    snprintf(cmd, 1024, "test $(ls -A \"%s\" 2>/dev/null | wc -l) -ne 0", directoryPath);

    status = system(cmd);
    exitcode = WEXITSTATUS(status);

    return exitcode;
}

int getPathIndex(char *path) {

    for (int i = 0; i < pathsCount; i++) {
        if (!strcmp(ownFiles[i].path, path)) {
            return i;
        }
    }

    return -1;
}

void readConfigurationParameters(char *path) {

    char errorMessage[PATH_MAX + 100];

    FILE *f = fopen(path, "r");

    if (NULL == f) {
        snprintf(errorMessage, sizeof(errorMessage), "Error opening configuration file: %s\n", path);
        displayError(errorMessage);
    }

    ip = (char *) malloc(20);

    if (NULL == ip) {
        snprintf(errorMessage, sizeof(errorMessage), "Configuration file: error at memory allocation for ip: %s\n", path);
        displayError(errorMessage);
    }

    char *s_port = (char *) malloc(20);

    if (NULL == s_port) {
        snprintf(errorMessage, sizeof(errorMessage), "Configuration file: error at memory allocation for port: %s\n", path);
        displayError(errorMessage);
    }

    errno = 0;
    fscanf(f, "ip=%s\nport=%s", ip, s_port);

    if (errno) {
        snprintf(errorMessage, sizeof(errorMessage), "Error while reading from configuration file: %s\n", path);
        displayError(errorMessage);
    }

    if (fclose(f)) {
        snprintf(errorMessage, sizeof(errorMessage), "Error at closing configuration file: %s\n", path);
        displayError(errorMessage);
    }

    port = atoi(s_port);
    printf("[loaded] ip: %s, port: %d\n", ip, port);
}


void printPaths() {

    for (int i = 0; i < pathsCount; i++)
        printf("%s\n", paths[i]);
}

void getAllFilesPaths(char *directoryPath) {

    getFilesPaths(directoryPath, strlen(directoryPath) + 1);
    strcpy(paths[pathsCount], "");

}

void getFilesPaths(char *directoryPath, int length) {

    DIR *currentDirectory;
    struct dirent *currentEntrance;
    char *entranceName;
    struct stat entranceInfo;
    char path[PATH_MAX];

    char errorMessage[PATH_MAX + 100];

    if (!(currentDirectory = opendir(directoryPath))) {
        snprintf(errorMessage, sizeof(errorMessage), "Error while opening directory: %s\n", directoryPath);
        displayError(errorMessage);
    }

    errno = 0;
    while ((currentEntrance = readdir(currentDirectory)) > 0) {
        entranceName = currentEntrance->d_name;

        if (strcmp(entranceName, ".") == 0 || strcmp(entranceName, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", directoryPath, entranceName);

        if (lstat(path, &entranceInfo) < 0) {
            snprintf(errorMessage, sizeof(errorMessage), "Error at lstat: %s\n", path);
            displayError(errorMessage);
        }

        strcpy(paths[pathsCount++], path + length);

        if (S_ISDIR(entranceInfo.st_mode) && !isEmptyDirectory(path)) {
            getFilesPaths(path, length);
        }
    }

    if (errno) {
        snprintf(errorMessage, sizeof(errorMessage), "Error while reading from directory: %s\n", directoryPath);
        displayError(errorMessage);
    }

    if (closedir(currentDirectory)) {
        snprintf(errorMessage, sizeof(errorMessage), "Error at closing directory: %s\n", directoryPath);
        displayError(errorMessage);
    }
}


void getAllFilesMetadata(char *root) {

    char errorMessage[PATH_MAX + 100];
    ownFiles = (FileMetadata *) malloc(pathsCount * sizeof(FileMetadata));
    //pentru fiecare path din paths, creaza un nou obiect file_metadata,
    //populeaza-l cu date apeland lstat SI adauga-l la own_files_metadata

    if (NULL == ownFiles) {
        displayError("Error at memory allocation for ownFiles");
    }

    struct stat entranceInfo;

    for (int i = 0; i < pathsCount; i++) {
        char *fullPath = (char *) malloc(strlen(root) + strlen(paths[i]) + 10);

        if (NULL == fullPath) {
            snprintf(errorMessage, sizeof(errorMessage), "Error at memory allocation for fullPath:");
            displayError(errorMessage);
        }

        snprintf(fullPath, PATH_MAX, "%s/%s", root, paths[i]);
        strcpy(ownFiles[i].path, paths[i]);

        if (stat(fullPath, &entranceInfo) < 0) {
            snprintf(errorMessage, sizeof(errorMessage), "Error at stat: %s\n", fullPath);
            displayError(errorMessage);
        }

        ownFiles[i].size = entranceInfo.st_size;
        ownFiles[i].timeStamp = entranceInfo.st_mtime;
        ownFiles[i].isRegularFile = S_ISDIR(entranceInfo.st_mode) ? 0 : 1;

        if (fullPath) {
            free(fullPath);
        }
    }
}