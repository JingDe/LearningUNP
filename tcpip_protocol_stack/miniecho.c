int mini_connect(int socket)
    该函数用来通知协议软件通过与目标主机三次握手后，建立TCP连接
int mini_recv(int socket, char* buf,int * buflen)
    该函数从指定的SOCKET接收数据
int mini_send(int socket,char*buf,int buflen)
    该函数将指定的数据发送到SOCKET
int mini_close(int socket)
    该函数用来通知协议软件关闭指定的socket连接
使用本协议软件库函数实现ECHO服务的一个简单例程如下：
/***************************************************
* Simple tcp echo client using libnet and libpcap and mini_socket.c
* file:      miniecho.c
* Date:     2003.5.
* Author:   white cpf
* compile:  gcc -Wall -lpcap -lnet miniecho.c mini_socket.c -o miniecho 
* Run:     readhat 8.0 you must be root and run ifconfig to see eth0 OK
*****************************************************/
#include <pcap.h>
#include <libnet.h>
#include "mini_socket.h"
#define uchar unsigned char
#define MAXBUFLEN 2048
char buf[MAXBUFLEN];
int buflen;
char recvbuf[MAXBUFLEN];
int recvedlen;
int main(int argc,char *argv[]){
    int ret;
    int i;
    char sip[40]="169.254.159.112";
    char sport[10]="7777";
    char dip[40]="169.254.159.111";
    char dport[10]="5000";
    struct socket ti;
    int s;
    ti.sip=inet_addr(sip);
    ti.dip=inet_addr(dip);
    ti.sport=(unsigned short)atoi(sport);
    ti.dport=(unsigned short)atoi(dport);
    s=mini_socket(&ti);
    if(s<0){
        printf("mini_socket() error\n");
        return 0;
    }
    ret=mini_connect(s);//connect to tcpip using TCP three time handshaking
    if(ret<0){
        printf("mini_connect() error\n");
        return 0;
    }
    while (1){
        //get input from stdin,  quit when EOF or "myquit!" input
        if(fgets(buf, sizeof(buf), stdin)==0)  break;
        if(strcmp(buf,"myquit!")==0)       break;
        ret=mini_send(s,buf,strlen(buf));
        if(ret<=0){
            printf("mini_send() return %d\n",ret);
            break;
        }
        ret=mini_recv(s,recvbuf,&recvedlen);
        if(ret<=0){
            printf("mini_recv() return %d\n",ret);
            break;
        }
        recvbuf[recvedlen]=0;
        printf("recved[%d bytes]:%s\n",recvedlen,recvbuf);
    }
    mini_close(s);//close tcpip using TCP three time handshaking
}