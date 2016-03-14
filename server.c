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
#include<linux/if_ether.h>
#include<linux/sockios.h>
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
    
    for (dev = 0 ; dev < num_devs; ++dev) {
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
    unsigned char rbuf[512];
    char wbuf[512];
    struct ethhdr *hdr = (struct ethhdr *)rbuf;
    struct ethhdr *whdr = (struct ethhdr *)wbuf;
    hdr->h_proto = htons(0x8d8d);
    whdr->h_proto = htons(0x8d8e);

    FD_ZERO(&rfd_set);
    FD_ZERO(&wfd_set);
    create_sockets();
    
    FD_SET(dp_socket[0].fd, &rfd_set);
    //FD_SET(dp_socket[1].fd, &wfd_set);

    while (1) {
        if (select(FD_SETSIZE, &rfd_set, NULL, NULL, NULL) < 0) {
            printf("select error\n");
            exit(-1);
        }

        memset(rbuf + sizeof(*hdr), 0, 512 - sizeof(*hdr));
        memset(wbuf + sizeof(*hdr), 0, 512 - sizeof(*hdr));
        memcpy(wbuf + sizeof(*hdr), "got it", 7);
        if(FD_ISSET(dp_socket[0].fd, &rfd_set)) {
            int ret;
            ret = read(dp_socket[0].fd, rbuf, 511);
            if (ret > 0) {
                //printf("%02x %02x %02x %02x %02x %02x\n",rbuf[0],rbuf[1],rbuf[2],rbuf[3], rbuf[4], rbuf[5]);
                printf("%s\n",rbuf + sizeof(*hdr));
            }
            ret = write(dp_socket[1].fd, wbuf, sizeof(*hdr) + 7);
            if (ret <=0) {
                printf("error write\n");
            }
        }
    }

    return 0;
}