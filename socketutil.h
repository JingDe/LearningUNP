#ifndef SOCKET_UITL_H
#define SOCKET_UTIL_H

#include<cstdio>
#include<cstdlib>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/epoll.h>
#include<cstring>
#include<sys/ioctl.h>
#include<errno.h>

typedef struct sockaddr SA;

//domain：communication domain（本地通信AF_UNIX、IPv4 AF_INET、IPv6 AF_INET6等）
//        选择用来通信的协议族
// type: 指定 通信semantics（流、数据报、SOCK_SEQPACKET等）
// protocol: 指定socket使用的特定协议，通常一个协议族中只有一个协议支持特定的socket type，可指定为0
//			
int Socket(int domain, int type, int protocol)
{
	int listenfd=socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd<0)
	{ 
		fprintf(stderr, "socket error: %s\n", strerror(errno));
		abort();
	}
	return listenfd;
}

void Bind(int listenfd, struct sockaddr *sa, socklen_t len)
{
	int ret=bind(listenfd, sa, len);
	if(ret<0)
	{
		fprintf(stderr, "bind error:%s\n" , strerror(errno));
		abort();
	}
}

void Listen(int listenfd, int backlog)
{
	int ret=listen(listenfd, backlog);
	if(ret<0)
	{
		fprintf(stderr, "listen error:%s\n", strerror(errno));
		abort();
	}
}

void Accept(int listenfd, struct sockaddr* addr, socklen_t *addrlen)
{
	int connfd=accept(listenfd, (struct sockaddr*) addr, addrlen);
	if(connfd<0)
	{
		//fprintf(stderr, "accept error: %s\n", strerror(errno));
		perror("accept error");
		abort() ;
	}
}

void Connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
	int ret=connect(sockfd, addr, addrlen);
	if(ret<0)
	{
		perror("connect error");
		abort();
	}
}

int Epoll_create(int size)
{
	int efd=epoll_create(size);
	// efd=epoll_create(1);
	if(efd==-1)
	{
		fprintf(stderr, "epoll_create failed: %s\n", strerror(errno));
		abort();
	}
	return efd;
}

void add_event(int efd, int fd, int event)
{
	struct epoll_event evt;
	evt.events=event;
	evt.data.fd=fd;
	epoll_ctl(efd, EPOLL_CTL_ADD, fd, &evt);
}

void delete_event(int efd, int fd, int event)
{
	struct epoll_event evt;
	evt.events=event;
	evt.data.fd=fd;
	epoll_ctl(efd, EPOLL_CTL_DEL, fd, &evt);
}


void modify_event(int efd, int fd, int event)
{
	struct epoll_event evt;
	evt.events=event;
	evt.data.fd=fd;
	epoll_ctl(efd, EPOLL_CTL_MOD, fd, &evt);
}

#endif