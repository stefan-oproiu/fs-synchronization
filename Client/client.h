#ifndef CLIENT_H
#define CLIENT_H

#include "../Common/common.h"

#ifndef ACCESSPERMS
#define ACCESSPERMS(S_IRWXU | S_IRWXG | S_IRWXO
#endif

FileMetadata *filesToDelete;
FileMetadata *filesToUpdate;

int filesToUpdateCount;
int fileToDeleteCount;

char *formatdate(time_t val);

int adjustTimestamp(char *filepath, char *timestamp);

int getIndexFromServerFiles(FileMetadata *serverFiles, int count, char *path);

int removeDirectory(char *directoryPath);

int removeFile(char *path);

void getFilesToDelete(FileMetadata *serverFiles, int serverFilesCount;

void getFilesToUpdate(FileMetadata *serverFiles, int serverFilesCount);

#endif //COMMON_H