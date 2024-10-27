#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

struct SockInfo
{
    int fd;                 // 通信
    pthread_t tid;          // 线程ID
    struct sockaddr_in addr; // 地址信息
};

struct SockInfo infos[128];

void *working(void *arg)
{
    while (1)
    {
        struct SockInfo *info = (struct SockInfo *)arg;
        // 接收数据
        char buf[1024];
        int ret = read(info->fd, buf, sizeof(buf));
        if (ret == 0)
        {
            printf("客户端已经关闭连接...\n");
            info->fd = -1;
            break;
        }
        else if (ret == -1)
        {
            printf("接收数据失败...\n");
            info->fd = -1;
            break;
        }
        else
        {
            write(info->fd, buf, strlen(buf) + 1);
        }
    }

    return NULL;
}

int test_thread()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("create socket error");
        exit(0);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10000);
    addr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("bind error");
        exit(0);
    }

    ret = listen(fd, 100);
    if (ret == -1)
    {
        perror("listen error");
        exit(0);
    }

    socklen_t len = sizeof(struct sockaddr);

    int max = sizeof(infos) / sizeof(infos[0]);
    for (int i = 0; i < max; ++i)
    {
        bzero(&infos[i], sizeof(infos[i]));
        infos[i].fd = -1;
        infos[i].tid = -1;
    }

    while (1)
    {
        struct SockInfo *pinfo;
        for (int i = 0; i < max; ++i)
        {
            if (infos[i].fd == -1)
            {
                pinfo = &infos[i];
                break;
            }
            if (i == max - 1)
            {
                sleep(1);
                i--;
            }
        }

        int connfd = accept(fd, (struct sockaddr *)&pinfo->addr, &len);
        printf("parent thread, connfd: %d\n", connfd);
        if (connfd == -1)
        {
            perror("accept");
            exit(0);
        }
        pinfo->fd = connfd;
        pthread_create(&pinfo->tid, NULL, working, pinfo);
        pthread_detach(pinfo->tid);
    }

    close(fd);

    return 0;
}