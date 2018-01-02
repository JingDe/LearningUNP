
#include<sys/epoll.h>
#include<string.h>

const static int epollsize=50;
const static int EPOLLEVENTS=50;

// 应用层buffer 
class Buffer{
public:
	Buffer():data_(vector<char>){}
	
	char* data()
	{ return &*data_.begin(); } 
	const char* data() const
	{ return &*data_.begin(); } // iterator begin();
	size_t size()
	{ return data_.size(); }
	
	void retrieve(const ch
	void append(const char* data, size_t len);
	
	void readFd(int fd); // // muduo 的readFd 采用 另外分配足够大的 char buf[], 使用 readv 读取
	// 另一种做法， 调用 ioctl FIONREAD 获得 缓冲区有多少字节
	
private:
	std::vector<char> data_;
	
};

// 实现 Read 和Write 函数
ssize_t Read(int fd, Buffer &buf); // Buffer *buf
ssize_t Write(int fd, Buffer &buf, int n); // Buffer *buf

/*
ssize_t Read(int fd, Buffer &buf)
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
ssize_t Read(int fd, Buffer &buf) 
{
	// muduo 采用 inputBuffer_.readFd(channel_->fd(), &savedErrno)
	
}

/*
ssize_t Write(int fd, Buffer &buf, int nbytes)
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

// 模仿muduo的做法，减少拷贝：
ssize_t Write(int fd, Buffer &buf, int nbytes)
{
	if(nbytes > buf.size())
		return -1;
	char b[512];
	int tw, nleft, nw;
	
	nw=0;
	tw=0; // 一共 write 的字节数
	nleft=nbytes;
	
	while(nleft>0)
	{
		 // Muduo 应用层buffer的做法：从buffer中全部取出来，试图一次性写，将nleft再 放回buffer去
		//nw=write(fd, buf.data(), buf.size()); 
		nw=write(fd, buf.data()+tw, nleft); // 不 拷贝
		if(nw<0)
			break;
		else if(nw<nleft)
		{
			
			
		}
		tw+=nw;
		nleft-=nw;
	}
	// 当源 和目的 重叠， 使用std::copy() ，不能从前面 移到 后面，这里 从后面移到前面
	std::copy(buf.data(), nleft, buf.data()); // 将 剩下的字节 移到开头
	return nbytes-nleft; // return tw;
}


void do_epoll(int listenfd)
{
	int efd;
	int nready;
	struct epoll_event events[EPOLLEVENTS];
	struct sockadd_in addr;
	int addrlen=sizeof(addr);
	struct epoll_event evt;
	//char buf[512];
	Buffer buf;
	
	
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
				
				//ssize_t nr=read(fd, buf, sizeof buf);
				ssize_t nr=Read(fd, buf); // 读到应用层 buffer的策略是：
				// append到 buf末尾
				if(nw==-1) // EPOLLERR 是否排除错误？？
				{
					fprintf(stderr, "read error: %s", strerror(errno));
					// 删除 该 读事件
				}
				// 一种做法，写到stdout，写回fd
				// 一种做法： 将fd添加为 等待写事件, 写回fd
				evt.events=EPOLLOUT;
				evt.data.fd=fd;
				epoll_ctl(efd, EPOLL_CTL_MOD, fd, &evt);
			}
			else if(evts  & EPOLLOUT) // 连接上数据可写
			{
				//ssize_t nw=write(fd, buf, nr);
				ssize_t nw=Write(fd, buf, nr); // 写 应用层buffer 的策略是：
				// 写出 nw 字节到fd，若 出错，buffer不变，若 正常，丢掉 buffer的开头nw字节
				if(nw==-1)
				{
					fprintf(stderr, "write error: %s", strerror(errno));
					// 删除 该 写事件
				}
				else if(nw<nr)
				{
					// 如何处理？ 自定义 应用层buffer
				}
				else
				{
					// 将 写事件 改成读事件
					evt.events=EPOLLIN;
					evt.data.fd=connfd;
					epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &evt);
				}
			}
			else // ？？
			{
				fprintf(stderr, "fd=%d, evts=%d", fd, evts);
			}
		}
	}
}