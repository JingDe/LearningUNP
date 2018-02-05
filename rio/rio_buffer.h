#ifndef CSAPP_RIO_BUFFER_H_
#define CSAPP_RIO_BUFFER_H_

#include <sys/types.h>
#include<stddef.h>
#include<unistd.h>

#define RIO_BUFSIZE 8192 // 作用于当前源文件 

typedef struct{
	int rio_fd;
	int rio_cnt;
	char* rio_bufptr;
	char rio_buf[RIO_BUFSIZE];
}rio_t;

void rio_readinitb(rio_t *rp, int fd);

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);

#endif 
