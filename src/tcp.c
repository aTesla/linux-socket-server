#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

// 信号处理函数
void callback(int num)
{
    while (1)
    {
        pid_t pid = waitpid(-1, NULL, WNOHANG);
        if (pid <= 0)
        {
            printf("sub process runing,or recyled\n");
            break;
        }
        printf("child die,pid = %d\n", pid);
    }
}

int childWork(int cfd);
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

    // 注册信号的捕捉
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = callback;
    sigemptyset(&act.sa_mask);
    sigaction(SIGCHLD, &act, NULL);

    // 接受多个客户端连接, 对需要循环调用 accept
    while (1)
    {
        // 4. 阻塞等待并接受客户端连接
        struct sockaddr_in cliaddr;
        socklen_t cliaddr_len = sizeof(cliaddr);
        int cfd = accept(lfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
        if (cfd == -1)
        {
            if (errno == EINTR)
            {
                // accept调用被信号中断了, 解除阻塞, 返回了-1
                // 重新调用一次accept
                continue;
            }
            perror("accept error");
            exit(0);
        }

        // 打印客户端的地址信息
        char ip[24] = {0};
        printf("client ip:%s,client port:%d\n",
               inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip, sizeof(ip)),
               ntohs(cliaddr.sin_port));

        // 新的连接已经建立了, 创建子进程, 让子进程和这个客户端通信
        pid_t pid = fork();
        printf("%i\n",pid);
        printf("a=%i\n",getpid());
        if (pid == 0)
        {
            // 子进程 -> 和客户端通信
            // 通信的文件描述符cfd被拷贝到子进程中
            // 子进程不负责监听
            close(lfd);
            while (1)
            {
                int ret = childWork(cfd);
                if (ret <= 0)
                {
                    break;
                }
            }
            // 退出子进程
            close(cfd);
            exit(0);
        }
        else if (pid > 0)
        {
            // 父进程不和客户端通信
            close(cfd);
        }
    }

    return 0;
}

// 5. 和客户端通信
int childWork(int cfd)
{
    char buf[1024];
    int number = 0;
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
    }
    else
    {
        perror("read error");
    }

    return len;
}