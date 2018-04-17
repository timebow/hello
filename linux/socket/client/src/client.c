/*
 * 参考： https://blog.csdn.net/aobai219/article/details/1596964
 * 客户端程序首先通过服务器域名获得服务器的IP地址，然后创建一个socket，调用connect函数与服务器建立连接，
 * 连接成功之后接收从服务器发送过来的数据，最后关闭socket。
 * 函数gethostbyname()是完成域名转换的。由于IP地址难以记忆和读写，所以为了方便，人们常常用域名来表示主机，
 * 这就需要进行域名和IP地址的转换。
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SERVPORT 3333
#define MAXDATASIZE 100 /*每次最大数据传输量 */

int main(int argc, char *argv[])
{
    int sockfd, recvbytes;
    char buf[MAXDATASIZE];
    struct hostent *host;
    struct sockaddr_in serv_addr;

    if (argc < 2)
    {
        fprintf(stderr,"Please enter the server's hostname!/n");
        exit(1);
    }
    
    if((host=gethostbyname(argv[1]))==NULL)
    {
        herror("gethostbyname出错！");
        exit(1);
    }
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket创建出错！");
        exit(1);
    }
    
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(SERVPORT);
    serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(serv_addr.sin_zero),8);
    
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect出错！");
        exit(1);
    }
    
    if ((recvbytes=recv(sockfd, buf, MAXDATASIZE, 0)) ==-1)
    {
        perror("recv出错！");
        exit(1);
    }
    
    buf[recvbytes] = '\0';
    printf("Received: %s",buf);
    close(sockfd);
}
