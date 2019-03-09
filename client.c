#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char *ip;
int port;
int sockfd;

struct sockaddr_in server_address;

#include "common.h"

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

    sendMessage();
}

int main(int argc, char *argv[])
{
	if (argc != 3)
    {
        printf("Usage %s <config_file_path> <root>.\n", argv[0]);
        exit(1);
    }    
    read_params(argv[1]);
    clientSetup();

    return 0;
}