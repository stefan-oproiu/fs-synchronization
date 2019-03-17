#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"
#include "client.h"

char *ip;
int port;
int sockfd;

struct sockaddr_in server_address;
char *root;

void sendMessage()
{
	char buff[1024];
	sprintf(buff, "test message.");

	write(sockfd, buff, strlen(buff));
	buff[0] = 0;

	int r;
	r = read(sockfd, buff, sizeof(buff));
	buff[r] = 0;

	if (r)
		fprintf(stdout, "%s\n", buff);
}

void handle()
{
	char buff[1024];
	char newpath[PATH_MAX];

	fm *server_files;

	int server_files_count;
	int r = 0, i, size;

	r = read(sockfd, buff, 10);
	buff[r] = 0;

	server_files_count = atoi(buff);

	#if defined DEBUG_MODE
		printf("[debug - client]: server tells that are %d files there\n", server_files_count);
	#endif

	if ((server_files = (fm *) malloc(server_files_count * sizeof(fm))) == NULL)
		displayError("malloc error.");

	for (i = 0; i < server_files_count; i++)
		read(sockfd, &server_files[i], sizeof(fm));

	getFilesToUpdate(server_files, server_files_count);

	#if defined DEBUG_MODE
		printf("[debug - client]: %d files need updated/added\n", ftu_count);
	#endif

	snprintf(buff, 10, "%d", ftu_count);
	write(sockfd, buff, 10);

	for (i = 0; i < ftu_count; i++)
	{
		#if defined DEBUG_MODE
			printf("[debug - client]: sending '%s' to server\n", files_to_update[i].path);
		#endif

		write(sockfd, files_to_update[i].path, PATH_MAX);
	}

	for (i = 0; i < ftu_count; i++)
	{
		int idx;
		unsigned int size;
		unsigned long timestamp;

		snprintf(newpath, PATH_MAX, "%s/%s", root, files_to_update[i].path);

		#if defined DEBUG_MODE
			printf("[debug - client]: file '%s' is being updated.\n", newpath);
		#endif

		r = read(sockfd, buff, 48);
		buff[r] = 0;

		//printf("[debug] buff=%s\n", buff);

		if (!files_to_update[i].is_regular_file) // is a directory
		{
			mkdir(newpath, ACCESSPERMS);
		}
		else // is a file, get data from socket and write into it
		{
			int fd = open(newpath, O_CREAT | O_TRUNC | O_WRONLY, 0755);
			off_t size;

			if (fd < 0)
				displayError("open() error.");

			read(sockfd, buff, 12);
			size = strtol(buff, NULL, 10);

			read(sockfd, buff, size);
			write(fd, buff, size);

			if (close(fd) < 0)
				displayError("close() error.");
		}

		if ((idx = getPathIndex(newpath)) == -1) // file/dir does not exists in the list => add it
		{
			strcpy(own_files[paths_count].path, newpath);
			own_files[paths_count].size = size;
			own_files[paths_count].timestamp = timestamp;
			paths_count ++;
		}
		else // update its stats
		{
			own_files[paths_count].size = size;
			own_files[paths_count].timestamp = timestamp;
		}

		#if defined DEBUG_MODE
			printf("[debug - client]: file '%s' successfully updated.\n", newpath);
		#endif
	}
}

void clientSetup()
{
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		exit(EXIT_FAILURE);
	}

	server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
    	exit(EXIT_FAILURE);
    }

    getAllFilesPaths(root);
    getAllFilesMetadata(root);

    handle();
}

int main(int argc, char *argv[])
{
	if (argc != 3)
    {
        printf("USAGE: %s <config_file_path> <root>.\n", argv[0]);
        exit(1);
    } 

    if (argv[2][strlen(argv[2]) - 1] == '/')
    {
    	printf("[ERROR]: Root shouldn't end with the '/' character.\n");
    	exit(1);
    }

    read_params(argv[1]);

    root = argv[2];
    clientSetup();

    return 0;
}