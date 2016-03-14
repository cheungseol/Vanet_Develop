#include <stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<linux/if.h>
#include<linux/if_packet.h>
#include<linux/sockios.h>
#include<linux/if_ether.h>
#include<string.h>
 
struct socks {
    int fd;
    struct sockaddr_ll addr;
} dp_socket[2];
 
int create_sockets(void)
{
    struct ifreq       ifr;
    struct sockaddr_ll addr;
    int                retval;
    int                s;
    __u16              type;
    int                dev, num_devs=2;
     
    type = htons(0x8d8d);
     
    for (dev = 0 ; dev <num_devs; ++dev) {
            type = htons(0x8d8d + dev);
            dp_socket[dev].fd = -1;
            s = socket (PF_PACKET, SOCK_RAW, type);
 
            if (s < 0) {
                return (s);
            }
 
            memset ((void*)&ifr, 0, sizeof (ifr));
 
            snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "eth0");
 
            retval = ioctl (s, SIOCGIFINDEX, &ifr);
            if (retval < 0) {
                close (s);
                return (retval);
            }
 
            memset (&addr, 0, sizeof (addr));
            addr.sll_family   = AF_PACKET;
            addr.sll_ifindex  = ifr.ifr_ifindex;
            addr.sll_protocol = type;
 
            retval = bind (s, (struct sockaddr *) &addr, sizeof (addr));
            if (retval < 0) {
                close (s);
                return (retval);
            }
 
            dp_socket[dev].fd = s;
            memset(&dp_socket[dev].addr, 0, sizeof(dp_socket[dev].addr));
 
            dp_socket[dev].addr.sll_family  = AF_PACKET;
            dp_socket[dev].addr.sll_ifindex = ifr.ifr_ifindex;
            dp_socket[dev].addr.sll_protocol = type;
    }
    return 0;
}
 
int main(void)
{
    fd_set rfd_set, wfd_set;
    int i;
    char rbuf[512];
    int ret;
    char wbuf[512];
    struct ethhdr *rhdr = (struct ethhdr *)rbuf;
    struct ethhdr *whdr = (struct ethhdr *)wbuf;
 
    FD_ZERO(&rfd_set);
    FD_ZERO(&wfd_set);
    create_sockets();
     
    FD_SET(dp_socket[1].fd, &rfd_set);
 
    memset(wbuf + sizeof(*whdr), 0, 512 - sizeof(*whdr));
    memcpy(wbuf + sizeof(*whdr), "coming", 7);
 
    whdr->h_dest[0] = 0x00;
    whdr->h_dest[1] = 0x22;
    whdr->h_dest[2] = 0x68;
    whdr->h_dest[3] = 0x14;
    whdr->h_dest[4] = 0xe1;
    whdr->h_dest[5] = 0xb1;
 
    whdr->h_proto = htons(0x8d8d);
    rhdr->h_proto = htons(0x8d8e);
    while (1) {
        ret = write(dp_socket[0].fd, wbuf, 7 + sizeof(*whdr));
        if (select(FD_SETSIZE, &rfd_set, NULL, NULL, NULL) < 0) {
            printf("select error\n");
            exit(-1);
        }
 
        memset(rbuf + sizeof(*rhdr), 0, 512 - sizeof(*rhdr));
        if(FD_ISSET(dp_socket[1].fd, &rfd_set)) {
            ret = read(dp_socket[1].fd, rbuf, 512);
            if (ret > 0) {
                printf("%s\n",rbuf+sizeof(*rhdr));
            }
            ret = write(dp_socket[0].fd, wbuf, 7 + sizeof(*whdr));
            if (ret <=0) {
                printf("error write\n");
            }
            sleep(1);
        }
    }
 
    return 0;
}