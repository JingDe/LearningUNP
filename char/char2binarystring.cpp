// 函数功能：将一个字符如'a'转换成二进制表示的字符串"0110 0001"
// -128 = 1000 0000，一个字节表示范围-128~127
// short至少2个字节
void Char2BinaryString(char c, char* bs)
{
	int i=0;
	short cmp=0x80; // 00000000 10000000
	while(i<8)
	{
		bs[i++]=c & cmp ? '1' : '0';
		cmp = cmp /2;
	}
	bs[i] = '\0';
}

// 函数功能：将二进制表示的字符串"0110 0001"转换成字符如'a'
char BinaryString2Char(char* bs)
{
	char c; // 初始化为0，0000 0000
	int i=0;
	while(i<8)
	{
		c = 2*c + bs[i++]-'0';
	} 
	return c; // c等于字符串表示的十进制值
}

/*
char BinaryString2Char(char *bs)
{
	int c=0;
	int i=0;
	while(i<8)
		c =2*c + bs[i++]-'0';
	return (char)c;
}*/

