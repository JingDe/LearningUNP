
#include"Buffer2.h"
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


const static int EPOLLEVENTS=50;
const static int SERV_PORT=2018;


ssize_t Recv(int fd, Buffer &buf); // Buffer *buf
ssize_t Send(int fd, Buffer &buf, int n); // Buffer *buf
int Socket(int domain, int type, int protocol);
void Bind(int listenfd, struct sockaddr *sa, socklen_t len);
void Listen(int listenfd, int backlog);
int Epoll_create(int size);

void do_epoll(int listenfd);
void handle_newConnection(int listenfd, const struct sockaddr_in &addr, socklen_t addrlen, int efd);
void handle_read(int fd, Buffer& buf, int efd);
void handle_send(int fd, Buffer& buf, int efd);

typedef struct sockaddr SA;

int main()
{
	int listenfd;
	struct sockaddr_in serveraddr;
	int LISTENQ=20;
	
	listenfd=Socket(AF_INET, SOCK_STREAM, 0);

	int optval=1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
	
	bzero(&serveraddr, sizeof serveraddr);
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serveraddr.sin_port=htons(SERV_PORT);
	
	Bind(listenfd, (SA*)&serveraddr, sizeof serveraddr);
	
	
	Listen(listenfd, LISTENQ);
	
	fprintf(stdout, "do_epoll...\n");
	do_epoll(listenfd);
	
	return 0;
}


/*
ssize_t Recv(int fd, Buffer &buf)
{
	char b[512];
	int nr, tr;
	tr=0;
	while(true)
	{
		nr=read(fd, b, sizeof b);
		if(nr<0)
		{
			if(tr==0)
				tr=-1;
			break;
		}
		tr+=nr;
		buf.append(b, nr); // buf.append(b);
	}
	return tr;
}*/

// 减少 拷贝
ssize_t Recv(int fd, Buffer &buf) // 此函数 可以 用 Buffer.readFd() 替代，main函数中 调用Read() 或 readFd() 
{
	buf.reportBuffer();
	// muduo 采用 inputBuffer_.readFd(channel_->fd(), &savedErrno)
	// muduo 的readFd 采用 另外分配足够大的 char extrabuf[65535], 使用 readv 读取
	// 另一种做法， 先调用 ioctl FIONREAD读取硬件状态寄存器的数值, 获得 缓冲区有多少字节，决定是否需要extrabuf
	int nbytes;
	int ret=ioctl(fd, FIONREAD, &nbytes);
	char *extrabuf=NULL;
	int ivcnt;
	if(ret<0) // ??ioctl何时失败
	{
		// 读到extrabuf 再append到buf
		// 采用 muduo做法
		extrabuf=new char[65535];
		ivcnt=2;
	}
	else if(nbytes<buf.writableSize())
	{
		ivcnt=1;
	}
	else
	{
		extrabuf=new char[nbytes];
		ivcnt=2;
	}
	
	fprintf(stdout, "nbytes=%d, ivcnt=%d\n", nbytes, ivcnt);
	
	struct iovec ivec[2];
	ivec[0].iov_base=buf.beginWrite();
	ivec[0].iov_len=buf.writableSize();
	ivec[1].iov_base=extrabuf;
	ivec[1].iov_len=sizeof extrabuf;
	ssize_t nr=readv(fd, ivec, ivcnt);
	
	fprintf(stdout, "readv: %d\n", nr);
	
	if(nr<0)
	{
		fprintf(stderr, "readv error:%s\n", strerror(errno));
	}
	else if(nr>buf.writableSize())
	{
		buf.hasWritten(buf.writableSize());
		buf.append(extrabuf, nr-buf.writableSize());
	}
	else
	{
		buf.hasWritten(nr);
	}
	
	
	if(extrabuf)
		delete extrabuf;
	
	buf.reportBuffer();
	return nr;
}

/*
ssize_t Send(int fd, Buffer &buf, int nbytes)
{
	if(nbytes > buf.size())
		return -1;
	char b[512];
	int n, nleft, nw;
	
	nleft=nbytes;
	
	while(nleft>0)
	{
		n=buf.retrieve(b, sizeof b); // 从buffer中 最多取出 b的大小 字节， 拷贝性能低！
		nw=write(fd, b, n); 
		if(nw<0)
			break;
		else if(nw<n)
		{
			//  b中剩下字节放回buffer中？
			buf.append(b+nw, n-nw);
		}
		nleft-=nw;
	}
	return nbytes-nleft;
}*/

// 客户调用写函数，将buf的nbytes字节写到fd，从buf的readerIndex_开始读出来写到fd中
// 模仿muduo的做法，减少拷贝：
ssize_t Send(int fd, Buffer &buf, int nbytes)
{
	if(nbytes > buf.readableSize())
		nbytes = buf.readableSize();
	
	//char b[512];
	int tw, nleft, nw;
	
	nw=0;
	tw=0; // 一共 write 的字节数
	nleft=nbytes;
	
	while(nleft>0)
	{
		 // Muduo 应用层buffer的做法：从buffer中全部取出来，试图一次性写，将nleft再 放回buffer去
		nw=write(fd, buf.beginRead()+tw, nleft); // 不 拷贝
		if(nw<0)
		{
			//break; 
			buf.hasRead(tw);
			return -1;
		}
		// write出错的处理？？
		// 若 已经 write成功了 一部分，返回出错，更新buffer
		// 若第一次write出错，返回出错，更新buffer不变
		else if(nw<nleft)
		{
			
			
		}
		tw+=nw;
		nleft-=nw;
	}
	// 当源 和目的 重叠， 由于std::copy()实现是从前往后依序拷贝的 ，不能从前面copy到后面，这里 从后面copy到前面
	//std::copy(buf.data(), nleft, buf.data()); // 将 剩下的字节 移到开头
	//更新readerIndex_的位置：
	buf.hasRead(tw);
	return tw;
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

void do_epoll(int listenfd)
{
	int efd;
	int nready;
	struct epoll_event events[EPOLLEVENTS];
	struct sockaddr_in addr;
	socklen_t addrlen=sizeof(addr);
	struct epoll_event evt;
	//char buf[512];
	Buffer buf(512);
		
	efd=Epoll_create(EPOLLEVENTS);
	
	evt.events=EPOLLIN;
	evt.data.fd=listenfd;
	epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &evt);
	
	
	while(true)
	{
		nready=epoll_wait(efd, events, EPOLLEVENTS, -1);
		fprintf(stdout, "\n\nnready=%d\n", nready);
		
		if(nready<0)
		{
			fprintf(stderr, "epoll_wait error: %s\n", strerror(errno));
			return ;
		}
		
		for (int i=0; i<nready; i++)
		{
			fprintf(stdout, "No.%d\n", i);
			int fd=events[i].data.fd;
			uint32_t evts=events[i].events;
			
			fprintf(stdout, "fd=%d, evts=%d\n", fd, evts);
			
			
			if( (evts & EPOLLERR)  ||  (evts & EPOLLHUP) )
			{
				fprintf(stderr, "EPOLLERR  | EPOLLHUP\n");
				continue;
			}
			
			if((fd==listenfd)  &&  (evts & EPOLLIN)) // 有新的连接请求
			{
				handle_newConnection(listenfd, addr, addrlen, efd);
			}
			else if (evts & EPOLLIN) // 连接上数据可读
			{
				handle_read(fd, buf, efd);				
			}
			else if(evts  & EPOLLOUT) // 连接上数据可写
			{
				handle_send(fd, buf, efd);
			}
			else 
			{
				fprintf(stdout, "unknown situation\n");
				//fprintf(stderr, "fd=%d, evts=%d\n", fd, evts);
			}
		}
	}
}


void handle_newConnection(int listenfd, const struct sockaddr_in &addr, socklen_t addrlen, int efd)
{
	fprintf(stdout, "new connection\n");
	int connfd=accept(listenfd, (struct sockaddr*) &addr, &addrlen);
	if(connfd<0)
	{
		//fprintf(stderr, "accept error: %s\n", strerror(errno));
		perror("accept error");
		return ;
	}
	struct epoll_event evt;
	evt.events=EPOLLIN;
	evt.data.fd=connfd;
	epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &evt);
}

void handle_read(int fd, Buffer& buf, int efd)
{
	fprintf(stdout, "EPOLLIN\n");
	//ssize_t nr=read(fd, buf, sizeof buf);
	ssize_t nr=Recv(fd, buf); // 读到应用层 buffer的策略是：append到 buf末尾
	fprintf(stdout, "nr=Recv : %d\n", nr);
	if(nr==-1) // EPOLLERR 是否排除错误？？
	{
		fprintf(stderr, "Recv error: %s\n", strerror(errno));
		// 删除 该 读事件
	}
	// 一种做法，写到stdout，写回fd
	// 一种做法： 将fd添加为 等待写事件, 写回fd
	struct epoll_event evt;
	evt.events=EPOLLOUT;
	evt.data.fd=fd;
	epoll_ctl(efd, EPOLL_CTL_MOD, fd, &evt);
}

void handle_send(int fd, Buffer& buf, int efd)
{
	fprintf(stdout, "EPOLLOUT\n");
	//ssize_t nw=write(fd, buf, nr);
	ssize_t nw=Send(fd, buf, buf.readableSize()); // 写 应用层buffer 的策略是：
	// 写出 nw 字节到fd，若 出错，buffer不变，若 正常，丢掉 buffer的开头nw字节
	if(nw==-1)
	{
		fprintf(stderr, "Send error: %s\n", strerror(errno));
		// 删除 该 写事件
	}
	else if(nw<buf.readableSize())
	{
		//仍然等待 EPOLLOUT
	}
	else
	{
		// 将 写事件 改成 读事件
		struct epoll_event evt;
		evt.events=EPOLLIN;
		evt.data.fd=fd;
		epoll_ctl(efd, EPOLL_CTL_MOD, fd, &evt);
	}
}

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
