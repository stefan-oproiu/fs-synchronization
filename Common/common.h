#ifndef COMMON_H
#define COMMON_H

#define DEBUG_MODE

#include <limits.h>

typedef struct FileMetadata {

    char path[PATH_MAX];
    unsigned int size;
    unsigned long timeStamp;
    short isRegularFile; // 1 if regular file; 0 if directory

} FileMetadata;

extern int pathsCount ;
char paths[1024][PATH_MAX];

FileMetadata *ownFiles;

char *ip;
int port;

void displayError(char *msg);

int isEmptyDirectory(char *directoryPath);

int getPathIndex(char *path);

void readConfigurationParameters(char *path);

void printPaths();

void getAllFilesPaths(char *directoryPath);

void getFilesPaths(char *directoryPath, int length);

void getAllFilesMetadata(char *root);

#endif //COMMON_H