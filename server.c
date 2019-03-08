#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

char *ip;
int port;

struct sockaddr_in server_address, remote_address;
int sockfd;
socklen_t len;

void read_params(char *path)
{
    FILE *f = fopen(path, "r");
    ip = (char *) malloc(20);

    char *s_port = (char *)malloc(20);
    fscanf(f, "ip=%s\nport=%s", ip, s_port);

    if (f)
        fclose(f);
    
    port = atoi(s_port);
    printf("%s %d\n", ip, port);
}

void handleRequest(int connfd)
{
    char buff[1024];
    int r;

    r = read(connfd, buff, sizeof(buff));
    
    buff[r] = '1';
    buff[r + 1] = 0;

    write(connfd, buff, r + 1);
    close(connfd);
    exit(0);
}

void acc()
{
    printf("works\n");

    len = sizeof(remote_address);
    int connfd = accept(sockfd, (struct sockaddr *) &remote_address, &len);
    pid_t pid;

    if ((pid = fork()) == 0) {
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
    acc();
}

int main(int argc, char *args[])
{
    if (argc != 3)
    {
        printf("Usage %s <config_file_path> <root>.\n", args[0]);
        exit(1);
    }   

    signal(SIGCHLD, SIG_IGN);

    read_params(args[1]);
    serverSetup();

    return 0;
}