#include<stddef.h>
#include <sys/types.h>
#include<unistd.h>
#include<errno.h>

ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);


ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
	size_t nleft=n;
	ssize_t nread;
	char* bufp=reinterpret_cast<char*>(usrbuf);
	
	while(nleft>0)
	{
		if((nread=read(fd, bufp, nleft)) <0)
		{
			if(errno == EINTR)
				nread=0;
			else
				return -1;
		}
		else if(nread==0)
			break;
		nleft -= nread;
		bufp += nread;
	}
	return (n-nleft);
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
	size_t nleft=n;
	ssize_t nwritten;
	char* bufp=reinterpret_cast<char*>(usrbuf);
	
	while(nleft>0)
	{
		if((nwritten=write(fd, bufp, nleft)) <=0)
		{
			if(errno == EINTR)
				nwritten=0;
			else
				return -1;
		}
		nleft -= nwritten;
		bufp += nwritten;
	}
	return n;
}


