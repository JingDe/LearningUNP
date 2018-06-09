
//非线程安全版readline
// readline即读到\n或\0终止

static int read_cnt;
static char* read_ptr;
static char read_buf[MAXLINE];

static ssize_t myread(int fd, char *ptr)
{
	if(read_cnt<=0)
	{
again:
		if((read_cnt=read(fd, read_buf, sizeof(read_buf))) <0)
		{
			if(errno==EINTR)
				goto again;
			return -1;
		}
		else if(read_cnt==0)
			return 0;
		read_ptr=read_buf;
	}
	
	read_cnt--;
	*ptr=*read_ptr++;
	return 1;
}

//不使用goto语句的写法
static ssize_t myread(int fd, char *ptr)
{
	while(read_cnt<=0)
	{
		if((read_cnt=read(fd, read_buf, sizeof(read_buf))) <0)
		{
			if(errno!=EINTR)
				return -1;
		}
		else if(read_cnt==0)
			return 0;
		read_ptr=read_buf;
	}
	
	read_cnt--;
	*ptr=*read_ptr++;
	return 1;
}

ssize_t readline(int fd, void* vptr, size_t maxlen)
{
	ssize_t n, rc;
	char c, *ptr;
	
	ptr=vptr;
	for(n=0; n<maxlen; n++)
	{
		if((rc=myread(fd, &c))==1)
		{
			*ptr++=c;
			if(c=='\n')
				break;
		}
		else if(rc==0)
		{
			*ptr=0;
			return n;
		}
		else
			return -1;
	}
	*ptr=0;
	return n+1;
}

// 暴露内部缓冲区
char* readlinebuf()
{
	return read_buf;
}