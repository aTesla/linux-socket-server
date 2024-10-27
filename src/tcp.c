#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

/*

u:unsigned
16:16 bit,32:32 bit
h:host
n:net
s:short
l:int

*/
int recv_tcp()
{
    // create socket
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
    {
        perror("create socket error");
        exit(0);
    }

    // bind
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10000);
    addr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(lfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("bind error");
        exit(0);
    }

    // listen
    ret = listen(lfd, 128);
    if (ret == -1)
    {
        perror("listen error");
        exit(0);
    }

    // accept
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int cfd = accept(lfd, (struct sockaddr *)&cliaddr, &clilen);
    if (cfd == -1)
    {
        perror("accept error");
        exit(0);
    }

    // read recv
    char ip[24] = {0};
    printf("client ip:%s,client port:%d\n",
           inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip, sizeof(ip)),
           ntohs(cliaddr.sin_port));

    char buf[1024];
    int number = 0;
    // communication
    while (1)
    {
        // read recv
        memset(buf, 0, sizeof(buf));
        int len = read(cfd, buf, sizeof(buf));
        if (len > 0)
        {
            printf("client say:%s\n", buf);

            memset(buf, 0, sizeof(buf));
            sprintf(buf, "hello,client...%d\n", number++);
            write(cfd, buf, strlen(buf) + 1);
        }
        else if (len == 0)
        {
            printf("client disconnect connection ...\n");
            break;
        }
        else
        {
            perror("read error");
            break;
        }
    }

    close(cfd);
    close(lfd);

    return 0;
}