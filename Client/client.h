#ifndef CLIENT_H
#define CLIENT_H

#include <stdlib.h>
#include <string.h>
#include "../Common/common.h"


FileMetadata *filesToDelete;
FileMetadata *filesToUpdate;

int filesToUpdateCount;
int fileToDeleteCount;

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

#endif //COMMON_H