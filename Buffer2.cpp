#include"Buffer2.h"

void Buffer::retrieve(const char *buf, size_t len)
{
	
}


void Buffer::append(const char* data, size_t len)
{
	ensureWritable(len);
	std::copy(data, len, beginWrite());
}

void Buffer::ensureWritable(size_t len)
{
	if(readerIndex_ + writableBytes() >len)
	{
		refresh();
	}
	else
	{
		// 已有的空闲空间 readerIndex_+writableBytes()
		// 需要的空闲空间 len
		// 需要增加的空间 len-(readerIndex_+writableBytes())
		// 当前的size size()==writerIndex_+writableBytes()
		// resize到 writerIndex_+writableBytes()+len-(readerIndex_+writableBytes())
		// ==writerIndex_ +len- readerIndex_
		data_.resize(writerIndex_+len-readerIndex_); 
		refresh();// 移动
	}
}

void Buffer::hasRead(size_t n)
{
	int nleft=buf.readableSize()-n;
	assert(readerIndex_+n<=writerIndex_);
	std::copy(beginRead()+n, nleft, data());
	readerIndex_=0;
	writerIndex_=nleft;
}

void Buffer::refresh()
{
	int readable=readableSize();
	std::copy(beginRead(), readableSize(), data());
	readerIndex_=0;
	writerIndex_=readable;
	//assert(readable==readableSize());
}