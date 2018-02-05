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
#include<arpa/inet.h>
#include"socketutil.h"

extern int Socket(int domain, int type, int protocol);
extern void Connect(int sockfd, const struct sockaddr* servaddr, socklen_t addrlen);
extern int Epoll_create(int size);

extern void add_event(int efd, int fd, int event);
extern void delete_event(int efd, int fd, int event);
extern void modify_event(int efd, int fd, int event);

const static int EPOLLEVENTS=5;
const static int SERV_PORT=2018;
const static char* SERV_ADDR="127.0.0.1";

int main()
{
	int clifd, epfd;
	struct sockaddr_in servaddr;
	socklen_t addrlen;
	struct epoll_event evts[EPOLLEVENTS];
	struct epoll_event evt;
	char buf[125];

	clifd=Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof servaddr);
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(SERV_PORT);
	inet_pton(AF_INET, SERV_ADDR, &servaddr.sin_addr);

	addrlen=sizeof(servaddr);
	Connect(clifd, (SA*) &servaddr, addrlen);

	epfd=Epoll_create(EPOLLEVENTS);
	
	evt.events=EPOLLIN;
	evt.data.fd=STDIN_FILENO; 
	epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &evt);
	

	while(true)
	{
		int ready=epoll_wait(epfd, evts, EPOLLEVENTS, -1);
		if(ready<0)
			break;

		for(int i=0; i<ready; i++)
		{
			int fd=evts[i].data.fd;
			uint32_t events=evts[i].events;

			if(events  &  EPOLLIN)
			{
				//fscanf(stdin, "%s", buf);
				int nr=read(fd, buf, sizeof buf);
				if(nr<=0)
				{
					delete_event(epfd, fd, EPOLLIN);
				}
				else
				{	
					if(fd==STDIN_FILENO)
					{
						buf[nr-1]='\0'; // 从标准输入读的时候在用户输入字符串后多添加一个换行符
						fprintf(stdout, "read stdin: [%s]\n", buf);
						add_event(epfd, clifd, EPOLLOUT);
					}
					else if(fd==clifd)
					{
						buf[nr]='\0'; // 从套接字读入server发送的数据没有
						fprintf(stdout, "read socket: [%s]\n", buf);
						delete_event(epfd, clifd, EPOLLIN);
						add_event(epfd, STDOUT_FILENO, EPOLLOUT);
					}
				}
			}
			else if(events  &  EPOLLOUT)
			{
				write(fd, buf, strlen(buf));
				

				if(fd==STDOUT_FILENO)
				{
					fprintf(stdout, "write stdout: [%s]\n", buf);
					delete_event(epfd, STDOUT_FILENO, EPOLLOUT);
				}
				else if(fd==clifd)
				{
					fprintf(stdout, "write socket: [%s]\n", buf);
					modify_event(epfd, clifd, EPOLLIN);
				}
			}
			else
			{
				fprintf(stderr, "events=%d, fd=%d\n", events, fd);
			}
		}
	}

	close(clifd);
	close(epfd);
	return 0;
}

