#ifndef CLIENT_H
#define CLIENT_H

#include <stdlib.h>
#include <string.h>
#include "../Common/common.h"


FileMetadata *filesToDelete;
FileMetadata *filesToUpdate;

int filesToUpdateCount;
int fileToDeleteCount;

void getFilesToDelete(FileMetadata *serverFiles, int serverFilesCount)
{
    filesToDelete = (FileMetadata *) malloc(sizeof(FileMetadata));
    
    for (int i = 0; i < pathsCount; i++)
    {
        int toDelete = 1;

        for (int j = 0; j < serverFilesCount; j++)
        {
            if (!strcmp(ownFiles[i].path, serverFiles[j].path))
            {
                toDelete = 0;
                break;
            }
        }

        if (toDelete)
        {
            filesToDelete = (FileMetadata *) realloc(filesToDelete, sizeof(FileMetadata) * (++fileToDeleteCount));
            filesToDelete[fileToDeleteCount - 1] = ownFiles[i];
        }
    }
}

void getFilesToUpdate(FileMetadata *serverFiles, int serverFilesCount)
{
    filesToUpdate = (FileMetadata *) malloc(sizeof(FileMetadata));

    for (int i = 0; i < serverFilesCount; i++)
    {
        int different = 0;
        int found = 0;

        for (int j = 0; j < filesToUpdateCount; j++)
        {
            if (!strcmp(serverFiles[i].path, filesToUpdate[j].path))
            {
                found = 1;

                if (serverFiles[i].size != filesToUpdate[j].size ||
                    serverFiles[i].timeStamp != filesToUpdate[j].timeStamp ||
                    serverFiles[i].isRegularFile != filesToUpdate[j].isRegularFile)
                {
                    different = 1;
                    break;
                }
            }
        }

        if (different || !found)
        {
            filesToUpdate = (FileMetadata *) realloc(filesToUpdate, sizeof(FileMetadata) * (++filesToUpdateCount));
            filesToUpdate[filesToUpdateCount - 1] = serverFiles[i];
        } 
    }
}

#endif //COMMON_H