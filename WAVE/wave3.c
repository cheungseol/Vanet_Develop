#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>/*for uint16_t*/
#include <stdio.h>
#include <net/if.h>
#include <linux/if_packet.h>


struct lldpdu
{
	char dst[6];
	char src[6];
	uint16_t type;
};



int main(int argc,char* argv[])
{
	struct lldpdu* lpacket = malloc(sizeof(struct lldpdu));
	memset(lpacket,0,sizeof(struct lldpdu));
	struct sockaddr_ll sll;
	memset(&sll,0,sizeof(struct sockaddr_ll));
	int sk = socket(PF_PACKET, SOCK_RAW, htons(0x88CC));/*创建原始套接字*/



	sll.sll_family = PF_PACKET;
	sll.sll_ifindex = if_nametoindex("eth0");/*sll虽然是用来指定目的地址的，但是在这个结构体中sll_ifindex 却指定的是本机发送报文的接口索引*/
	sll.sll_protocol = htons(0x88CC); 
	sll.sll_addr[0] = 0x34;/*sll_addr指定目的MAC地址*/
	sll.sll_addr[1] = 0xb0;
	sll.sll_addr[2] = 0x52;
	sll.sll_addr[3] = 0xda;
	sll.sll_addr[4] = 0xda;
	sll.sll_addr[5] = 0x18;


	lpacket->dst[0] = 0x34;/*自己构造的L2 报文的目的地址*/
	lpacket->dst[1] = 0xb0;
	lpacket->dst[2] = 0x52;
	lpacket->dst[3] = 0xda;
	lpacket->dst[4] = 0xda;
	lpacket->dst[5] = 0x18;



	lpacket->src[0] = 0x00;/*自己构造的L2 报文的源地址*/
	lpacket->src[1] = 0x0c;
	lpacket->src[2] = 0x29;
	lpacket->src[3] = 0x8d;
	lpacket->src[4] = 0xf1;
	lpacket->src[5] = 0x04;	
	lpacket->type = htons(0x88cc);/*报文类型*/

	

	while(1)

	{
       /*只有L2头，数据为空。每隔3秒发送0X88CC报文，即LLDP报文*/

		sendto(sk, lpacket, 14, 0, (struct sockaddr*)&sll, sizeof(struct sockaddr_ll));
		printf("send one lldp packet\n");
		sleep(3);

	}

	return 0;

}