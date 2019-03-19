#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include "common.h"

int sockfd;
struct sockaddr_in server_address, remote_address;
socklen_t len;

char *root;

off_t getFileSize(char *path)
{
    struct stat st;

    if (lstat(path, &st) < 0)
        displayError("lstat() error.");

    return st.st_size;
}

void handleRequest(int connfd)
{
    char buff[4096], newpath[PATH_MAX];
    int r, i;
    int files_to_update_cnt;

    getAllFilesPaths(root);
    getAllFilesMetadata(root);

    fm *files_to_update;

    /*
    * Telling the client how many files are ready to be synchronized.
    * Then sending their files to check which of them needs sync.
    */

    snprintf(buff, 10, "%d", paths_count);
	write(connfd, buff, 10);

    
	for (i = 0; i < paths_count; i++) 
    {
		write(connfd, &own_files[i], sizeof(fm));
        //printf("%s %d\n", own_files[i].path, own_files[i].size);
    }
    
    exit(0);
    /*
    * Receiving the number of files that need updated on clients' computer.
    */

    r = read(connfd, buff, 10);
    buff[r] = 0;

    files_to_update_cnt = atoi(buff);

    #if defined DEBUG_MODE
        printf("[debug - server]: client needs %d files updated.\n", files_to_update_cnt);
    #endif

    /*
    * Allocate enough memory for the files.
    */

    if ((files_to_update = (fm *) malloc(files_to_update_cnt * sizeof(fm))) == NULL)
        displayError("malloc error.");

    /*
    * Store the files into an array.
    */

    for (i = 0; i < files_to_update_cnt; i++)
    {
        r = read(connfd, buff, PATH_MAX);
        buff[r] = 0;

        #if defined DEBUG_MODE
            printf("[debug - server]: client needs file '%s' updated.\n", buff);
        #endif

        files_to_update[i] = own_files[getPathIndex(buff)];
    }

    /*
    * Building absolute path + transfer non-updated files to the client.
    */

    for (i = 0; i < files_to_update_cnt; i++)
    {
        snprintf(newpath, PATH_MAX, "%s/%s", root, files_to_update[i].path);

        #if defined DEBUG_MODE
            printf("[debug - server]: file '%s' is updating to client.\n", newpath);
        #endif

        if (files_to_update[i].is_regular_file) // is not a directory
        {
            int fd;

            if ((fd = open(newpath, O_RDONLY)) == -1)
                displayError("open() error.");

            off_t size = getFileSize(newpath);

            #if defined DEBUG_MODE
                printf("[debug - server]: file '%s', size=%ld\n", newpath, size);
            #endif

            snprintf(buff, 12, "%ld", size);
            write(connfd, buff, 12);

            read(fd, buff, size);
            write(connfd, buff, size);

            if (close(fd) < 0)
                displayError("close() error.");

            #if defined DEBUG_MODE
                printf("[debug - server]: file '%s' successfully updated.\n", files_to_update[i].path);
            #endif
        }
    }

    if (files_to_update)
        free(files_to_update);

    exit(0);
}

void acc()
{
    int connfd;
    pid_t wpid, pid;
    int status;

    len = sizeof(remote_address);

    while ((connfd = accept(sockfd, (struct sockaddr *) &remote_address, &len)) >= 0)
    {
        if ((pid = fork()) == 0)
            handleRequest(connfd);

        close(connfd);

        while ((wpid = wait(&status)) > 0);
    }
}

void serverSetup() 
{
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
        printf("binding failed\n");
        exit(EXIT_FAILURE);
    }

    listen(sockfd, 5);
    acc();
}

int main(int argc, char *args[])
{
    if (argc != 3)
    {
        printf("Usage %s <config_file_path> <root>.\n", args[0]);
        exit(1);
    }   

    // signal(SIGCHLD, SIG_IGN);

    read_params(args[1]);
    root = args[2];
    serverSetup();

    if (own_files)
        free(own_files);

    return 0;
}