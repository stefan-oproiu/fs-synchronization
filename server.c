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

#include "common.h"

int sockfd;
struct sockaddr_in server_address, remote_address;
socklen_t len;

char *root;

void handleRequest(int connfd)
{
    char buff[PATH_MAX], newpath[PATH_MAX];
    int r, i;
    int files_to_update_cnt;

    fm *files_to_update;

    // Tell the client first how many files are on the server

    snprintf(buff, 10, "%d", paths_count);
	write(connfd, buff, 10);

	for (i = 0; i < paths_count; i++)
		write(connfd, &own_files[i], sizeof(fm));

    r = read(connfd, buff, 10);
    buff[r] = 0;

    files_to_update_cnt = atoi(buff);

    #if defined DEBUG_MODE
        printf("[debug - server]: client needs %d files updated.\n", files_to_update_cnt);
    #endif

    if ((files_to_update = (fm *) malloc(files_to_update_cnt * sizeof(fm))) == NULL)
        displayError("malloc error.");

    for (i = 0; i < files_to_update_cnt; i++)
    {
        r = read(connfd, buff, PATH_MAX);
        buff[r] = 0;

        #if defined DEBUG_MODE
            printf("[debug - server]: client needs file '%s' updated.\n", buff);
        #endif

        files_to_update[i] = own_files[getPathIndex(buff)];
    }

    for (i = 0; i < files_to_update_cnt; i++)
    {
        snprintf(newpath, PATH_MAX, "%s/%s", root, files_to_update[i].path);

        #if defined DEBUG_MODE
            printf("[debug - server]: file '%s' is updating to client.\n", newpath);
        #endif

        if (files_to_update[i].is_regular_file) // is not a directory
        {
            int fd = open(newpath, O_RDONLY);

            if (fd == -1)
                displayError("open() error.");

            while ((r = read(fd, buff, 128)))
            {
                buff[r] = 0;
                write(connfd, buff, r);
            }

            close(fd);

            #if defined DEBUG_MODE
                printf("[debug - server]: file '%s' successfully updated.\n", files_to_update[i].path);
            #endif
        }
    }

    close(connfd);
    exit(0);
}

void acc()
{
    len = sizeof(remote_address);

    int connfd = accept(sockfd, (struct sockaddr *) &remote_address, &len);
    pid_t pid;

    if ((pid = fork()) == 0) 
    {
        printf("pid %d\n", getpid());
        handleRequest(connfd);
    }
    else
    {
        acc();
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

    getAllFilesPaths(root);
    getAllFilesMetadata(root);

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

    return 0;
}