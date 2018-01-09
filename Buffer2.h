#ifndef MY_BUFFER2_H
#define MY_BUFFER2_H

#include<cstdio>
#include<vector>
#include<sys/types.h>
#include<sys/socket.h>

/// +-------------------+------------------+------------------+
/// |    empty bytes    |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size


class Buffer{
public:
	static const std::size_t kInitialSize = 1024;

	// vector构造函数 explicit vector( size_type count );
	Buffer():data_(kInitialSize), readerIndex_(0),writerIndex_(0)
	{} 
	Buffer(int size):data_(size), readerIndex_(0),writerIndex_(0)
	{}
	
	char* data()
	{ return &*data_.begin(); } 
	const char* data() const
	{ return &*data_.begin(); } // iterator vector::begin();
	
	char* beginRead()
	{ return &*data_.begin()+readerIndex_; }
	char* beginWrite()
	{ return &*data_.begin()+writerIndex_; }
	
	std::size_t readableSize() const
	{ return writerIndex_ - readerIndex_; }
	std::size_t writableSize() const
	{ return data_.size()-writerIndex_; } // vector.size() 不是capacity()
	
	void retrieve(const char *buf, std::size_t len);
	void append(const char* data, std::size_t len);
	
	// void readFd(int fd); 
	// muduo 的readFd 采用 另外分配足够大的 char extrabuf[65535], 使用 readv 读取
	// muduo 做法的好处：1，使用 readv结合栈上空间而不为data_初始分配过大内存  减少内存开销
	// 2, 减少 多次调用read write 系统调用的开销
	// 3，将 readFd置于Buffer内而不作为free function的Read，使得每个IO线程一个buffer，只需要extrabuf的64KB的栈上空间，与连接数目无关？？
	//    若栈上空间紧张，可使用 thread local的extrabuf？？
	// 另一种做法， 先调用 ioctl FIONREAD 获得 缓冲区有多少字节，决定是否需要extrabuf
	
	void hasRead(std::size_t n);
	void hasWritten(std::size_t n);
	
	void reportBuffer();
		
private:
	void ensureWritable(std::size_t len);
	void refresh();
	
	
	
	std::vector<char> data_;
	int readerIndex_;
	int writerIndex_;
};


/*
Buffer 的一种实现：保持 包含的数据 在data_开头，调用size()获得实际长度，每次操作扩大或收缩 data_
muduo的实现： 初始分配 足够大内存，使用 读和写 两个指针，从buffer的读指针往后读，写到buffer的写指针之后
*/


#endif