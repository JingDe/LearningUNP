#include"Buffer2.h"
#include<cassert>

void Buffer::retrieve(const char *buf, std::size_t len)
{
	
}


void Buffer::append(const char* data, std::size_t len)
{
	ensureWritable(len);
	std::copy(data, data+len, beginWrite());
	hasWritten(len);
}

void Buffer::ensureWritable(std::size_t len)
{
	if(readerIndex_ + writableSize() >len)
	{
		refresh();
	}
	else
	{
		// 已有的空闲空间 readerIndex_+writableSize()
		// 需要的空闲空间 len
		// 需要增加的空间 len-(readerIndex_+writableSize())
		// 当前的size size()==writerIndex_+writableSize()
		// resize到 writerIndex_+writableSize()+len-(readerIndex_+writableSize())
		// ==writerIndex_ +len- readerIndex_
		data_.resize(writerIndex_+len-readerIndex_); 
		refresh();// 移动
	}
}

void Buffer::hasRead(std::size_t n) // read  对应 muduo的retrieve
{
	int nleft=readableSize()-n;
	assert(readerIndex_+n<=writerIndex_);
	//std::copy(beginRead()+n, beginRead()+n+nleft, data());
	std::copy(beginRead()+n, beginWrite(), data());
	readerIndex_=0;
	writerIndex_=nleft;
}

void Buffer::hasWritten(std::size_t n)
{
	writerIndex_+=n;
}

void Buffer::refresh()
{
	int readable=readableSize();
	std::copy(beginRead(), beginRead()+readableSize(), data());
	readerIndex_=0;
	writerIndex_=readable;
	//assert(readable==readableSize());
}

void Buffer::reportBuffer()
{
	fprintf(stdout, "BUFFER: readerIndex=%d, writerIndex=%d, writableSize=%d\n", readerIndex_, writerIndex_, readableSize());
}
