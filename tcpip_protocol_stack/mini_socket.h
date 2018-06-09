struct mini_sock{ 
	u8 type;//=TCP or UDP
	u32 dip;//destination ip
	u32 dport;//destination port
	u16 sip;//source ip
	u16 sport;//source port
	……
}