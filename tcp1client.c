#include <sys/types.h> 
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netpacket/packet.h>
#include <netinet/in.h>

#define SERVPORT 3333
#define MAXDATASIZE 100

int main(int argc, char const *argv[])
{
	
	int sockfd,sendbytes;

	char buf[MAXDATASIZE];

	struct hostent * hose;

	struct sockaddr_in serv_addr;

	if (argc<2)
	{
		fprintf(stderr, "Please enter the server's hostname!\n", );
		exit(1);
	}

	/*地址解析函数*/
	if ((host=gethostbyname(argv[1]))==NULL)
	{
		perror("ethostbyname");
		exit(1);
	}

	/*
	建立 socket 
	 */
	if ((sockfd == socket(AF_INET,SOCK_STREAM,0))== -1)
	{
		perror("socket");
		exit(1);
	}

	/*设置sockaddr_in结构体中的相关参数*/
	serv_kaddr.sin_family = AF_INET;
	serv_kaddr.sin_port = htons(SERVPORT);
	serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(serv_sockaddr.sin_zero),8);

	/*调用connect函数，主动发起对服务器端的连接*/
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))==-1)
	{
		perror("connect");
		exit(1);
	}

	/*发送消息给服务器端*/
	if ((sendbytes = send(sockfd,"hello",5,0)) == -1)
	{
		perror("send");
		exit(1);
	}


	close(sockfd);
}
