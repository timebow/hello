/*
 * 参考： https://blog.csdn.net/aobai219/article/details/1596964
 * 代码实例中的服务器通过socket连接向客户端发送字符串"Hello, you are connected!"。
 * 只要在服务器上运行该服务器软件，在客户端运行客户软件，客户端就会收到该字符串。
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define SERVPORT 3333 /*服务器监听端口号 */
#define BACKLOG 10 /* 最大同时连接请求数 */

int main(int argc, char *argv[])
{
    socklen_t sin_size;
    int sockfd,client_fd; /*sock_fd：监听socket；client_fd：数据传输socket */
    struct sockaddr_in my_addr; /* 本机地址信息 */
    struct sockaddr_in remote_addr; /* 客户端地址信息 */

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket创建出错！"); exit(1);
    }
    
    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(SERVPORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero),8);
    
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind出错！");
        exit(1);
    }
    
    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen出错！");
        exit(1);
    }
    
    while(1)
    {
        sin_size = sizeof(struct sockaddr_in);
        if ((client_fd = accept(sockfd, (struct sockaddr *)&remote_addr, &sin_size)) == -1)
        {
            perror("accept出错");
            continue;
        }
        printf("received a connection from %s/n", inet_ntoa(remote_addr.sin_addr));
        if (!fork())
        { /* 子进程代码段 */
            if (send(client_fd, "Hello, you are connected!/n", 26, 0) == -1)
            perror("send出错！");
            close(client_fd);
            exit(0);
        }
        close(client_fd);
    }
}
