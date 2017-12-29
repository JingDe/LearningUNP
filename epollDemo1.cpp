
#include<sys/epoll.h>
#include<string.h>

const static int epollsize=50;
const static int EPOLLEVENTS=50;

void do_epoll(int listenfd)
{
	int efd;
	int nready;
	struct epoll_event events[EPOLLEVENTS];
	struct sockadd_in addr;
	int addrlen=sizeof(addr);
	struct epoll_event evt;
	char buf[512];
	
	
	efd=epoll_create(epollsize);
	// efd=epoll_create(1);
	if(efd==-1)
	{
		fprintf(strerr, "epoll_create failed: %s", strerror(errno));
		return;
	}
	
	
	evt.events=EPOLLIN;
	evt.data.fd=listenfd;
	epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &evt);
	
	while(true)
	{
		nready=epoll_wait(efd, events, EPOLLEVENTS, -1);
		
		if(nready<0)
		{
			fprintf(stderr, "epoll_wait error: %s", strerror(errno));
			return ;
		}
		
		for (int i=0; i<nready; i++)
		{
			int fd=events[i].data.fd;
			uint32_t evts=events[i].events;
			
			// 判断 错误事件
			if( (evts & EPOLLERR)  ||  (evts & EPOLLHUP) )
			{
				fspritf(stderr, "EPOLLERR  | EPOLLHUP");
				continue;
			}
			
			if((fd==listenfd)  &&  (evts & EPOLLIN)) // 有新的连接请求
			{
				int connfd=accept(listenfd, (struct sockaddr*) &addr, &addrlen);
				if(connfd<0)
				{
					fprintf(stderr, "accept error: %s", strerror(errno));
					perror("accept error");
					return ;
				}
				evt.events=EPOLLIN;
				evt.data.fd=connfd;
				epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &evt);
			}
			else if (evts & EPOLLIN) // 连接上数据可读
			{
				
				ssize_t nr=read(fd, buf, sizeof buf);
				if(nw==-1) // EPOLLERR 是否排除错误？？
				{
					fprintf(stderr, "read error: %s", strerror(errno));
					
				}
				// 一种做法，写到stdout，写回fd
				// 一种做法： 将fd添加为 等待写事件
			}
			else if(evts  & EPOLLOUT) // 连接上数据可写
			{
				ssize_t nw=write(fd, buf, nr);
				if(nw==-1)
				{
					fprintf(stderr, "write error: %s", strerror(errno));
				}
				else if(nw<nr)
				{
					// 如何处理？ 自定义 应用层buffer
				}
			}
			else // ？？
			{
				fprintf(stderr, "fd=%d, evts=%d", fd, evts);
			}
		}
	}
}