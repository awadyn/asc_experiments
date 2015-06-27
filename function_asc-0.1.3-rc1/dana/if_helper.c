#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <inttypes.h>
#include <assert.h>
#include <poll.h>
#include <infiniband/arch.h>

#include "if_helper.h"
#include "danasrv.h"

// While debugging the scanIfRxRing
// I looked at this code http://www.scaramanga.co.uk/code-fu/lincap.c
// You will find a few reminances from its debug code

int 
openIfByName(const char *if_name, int *fd, int *index,
	     struct sockaddr *hwaddr, int *mtu)
{
  struct ifreq ifr;
  int fd_;
  int verbose = globals.opts.verbose;

  memset(&ifr, 0, sizeof(ifr));
  
  *fd=-1;
  strncpy (ifr.ifr_name, if_name, sizeof(ifr.ifr_name) - 1);
  ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';
  
  fd_ = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if ( fd_ == -1) {
    perror(LOC_INFO);
    return -1;
  }
  *fd=fd_;
  
  if (ioctl(fd_, SIOCGIFINDEX, &ifr) == -1) {
    close(fd_);
    perror(LOC_INFO);
    return -1;
  }
  *index = ifr.ifr_ifindex;
  if (verbose) {
    fprintf(stderr, "%s: index:%d", if_name, *index);
  }
  if (ioctl(fd_, SIOCGIFHWADDR, &ifr) == -1) {
      perror(LOC_INFO);
    }
  *hwaddr = ifr.ifr_hwaddr;
  if (verbose) {
    unsigned char *mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
    fprintf(stderr, "mac: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X " , 
	     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  }
    
  if (ioctl(fd_, SIOCGIFMTU, &ifr) == -1) {
    perror(LOC_INFO);
  }
  *mtu = ifr.ifr_mtu;
  if (verbose) fprintf(stderr, " mtu: %d\n", *mtu);
  return 1;
}

int listIf(void)
{
  int             fd;
  struct ifreq    ifr;
  int i;
  
  fd = socket(AF_PACKET, SOCK_RAW, 0);
  if (fd == -1) {
    perror(LOC_INFO);
    return 0;
  }
  
  for (i=1; i<100; i++) {
    ifr.ifr_ifindex = i;
    if (ioctl(fd, SIOCGIFNAME, &ifr) == -1) {
      // perror(LOC_INFO);
      break;
    }
    printf("%d: %s ", ifr.ifr_ifindex, ifr.ifr_name);

    if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
      perror(LOC_INFO);
    }
    {
      unsigned char *mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
      printf("mac: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X " , 
	     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    if (ioctl(fd, SIOCGIFMTU, &ifr) == -1) {
      perror(LOC_INFO);
    }
    printf(" mtu: %d\n", ifr.ifr_mtu);
  }

  close(fd);
  return i;
}

#if 0
int setIfMTU(int fd, int mtu) 
{
  // SIOCSIFMTU
 struct ifreq req;
 req.ifr_mtu=mtu;
}
#endif

void
mapIfDestroy(struct if_map_desc *md) 
{
  if (!md || md->fd<0) return;
  close(md->fd);
  md->fd=-1;
  memset(md->name, 0, sizeof(md->name));
}

#if 0
int getIfHWAddr(int fd, struct sockaddr *sa)
{
  struct ifreq req;
  memset(&req, 0, sizeof(req));

  req.ifr_addr.sa_family = AF_INET;
  if (ioctl(fd, SIOCGIFHWADDR, &req, sizeof(req))<0) {
    perror(LOC_INFO);
    return -1;
  }
  unsigned char *mac = (unsigned char *)req.ifr_hwaddr.sa_data;

  //display mac address
  printf("Mac : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n" , 
	 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  memcpy(sa, &req.ifr_hwaddr, sizeof(struct sockaddr));
  return 1;
}
#endif

int
mapIfCreate(const char *if_name, struct if_map_desc *md)
{
  if (!if_name || !md) return -1;

  memset(md, 0, sizeof(struct if_map_desc));
  md->fd = -1;

  strncpy (md->name, if_name, sizeof(md->name) - 1);
  md->name[sizeof(md->name)-1] = '\0';

  if (openIfByName(md->name, &(md->fd), 
		   &(md->index), &(md->sa), &(md->mtu))<0) return;

  { // SET UP TPACKET VERSION WE WILL USE -- 2
    int v = TPACKET_V2;
    if (setsockopt(md->fd, SOL_PACKET, 
		   PACKET_VERSION, &v, sizeof(v)) < 0) {
      perror(LOC_INFO);
      goto error;
    }
  }

  // FIXME : Can't find PACKET_QDISC_BYPASS
#if 0	     
  { // TURN OF QDISCIPLINE TO BY PASS KERNEL BUFFERING LOGIC ALL TOGETHER
    // ON TRANSIMIT
    int one = 1;
    if (setsockopt(md->fd, SOL_PACKET,
		   PACKET_QDISC_BYPASS, &one,sizeof(one)) < 0) {
      perror(LOC_INFO);
      goto error;
    }
  }
#endif 

  { // CONFIGURE RX AND TX RINGS FOR MAPPING
    assert(sysconf(_SC_PAGESIZE) == RING_PAGE_SIZE);
    memset(&md->req, 0, sizeof(md->req));
    md->req.tp_block_size = RING_BLOCK_SIZE;
    md->req.tp_frame_size = RING_FRAME_SIZE;
    md->req.tp_block_nr = RING_NUM_BLOCKS;
    md->req.tp_frame_nr = RING_NUM_FRAMES;
    
    if (setsockopt(md->fd, 
		   SOL_PACKET, PACKET_RX_RING, 
		   &md->req, sizeof(md->req)) < 0) {
      perror(LOC_INFO);
      goto error;
    }
    assert(md->req.tp_block_size == RING_BLOCK_SIZE);
    assert(md->req.tp_frame_size == RING_FRAME_SIZE);
    assert(md->req.tp_block_nr == RING_NUM_BLOCKS);
    assert(md->req.tp_frame_nr == RING_NUM_FRAMES);
    assert((md->req.tp_block_size * md->req.tp_block_nr) == RING_TOTAL_SIZE);

    if (setsockopt(md->fd, 
		   SOL_PACKET, PACKET_TX_RING, 
		   &md->req, sizeof(md->req)) < 0) {
      perror(LOC_INFO);
      goto error;
    }
    assert(md->req.tp_block_size == RING_BLOCK_SIZE);
    assert(md->req.tp_frame_size == RING_FRAME_SIZE);
    assert(md->req.tp_block_nr == RING_NUM_BLOCKS);
    assert(md->req.tp_frame_nr == RING_NUM_FRAMES);
    assert((md->req.tp_block_size * md->req.tp_block_nr) == RING_TOTAL_SIZE);
  }

  { // MAP THE RX AND TX RINGS 
    md->rxbase = mmap(NULL, 
		      RING_TOTAL_SIZE * 2,
		      PROT_READ | PROT_WRITE | PROT_EXEC,
   		      MAP_SHARED | MAP_LOCKED, 
		      md->fd,
		      0);
    if (md->rxbase == MAP_FAILED) {
      perror(LOC_INFO);
      goto error;
    }
    md->txbase = (uint8_t *)md->rxbase + RING_TOTAL_SIZE; 
  }

  { // bind socket to the specific interface 
    struct sockaddr_ll my_addr;     
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sll_family = AF_PACKET;
    my_addr.sll_protocol = htons(ETH_P_ALL);
    my_addr.sll_ifindex =  md->index;	 
    my_addr.sll_pkttype=0;
    my_addr.sll_halen=0;

    if (bind(md->fd,(struct sockaddr *)&my_addr,
	     sizeof(my_addr))<0) {
      perror(LOC_INFO);
      goto error;
    } 
  }
  return 1;
 error:
  mapIfDestroy(md);
  return -1; 
}

ifPrintStatus(unsigned long status, FILE *f)
{
  fprintf(f, "0x%lx: ", status);
  if (status == TP_STATUS_KERNEL) fprintf(f, "TP_STATUS_KERNEL");
  if (status & TP_STATUS_USER) fprintf(f, "TP_STATUS_USER ");
  if (status & TP_STATUS_COPY) fprintf(f, "TP_STATUS_COPY ");
  if (status & TP_STATUS_LOSING) fprintf(f, "TP_STATUS_LOSING ");
  if (status & TP_STATUS_CSUMNOTREADY) fprintf(f,"TP_STATUS_CSUMNOTREADY "); 
  if (status & TP_STATUS_CSUMNOTREADY) fprintf(f,"TP_STATUS_VLAN_VALID ");
  if (status & TP_STATUS_BLK_TMO) fprintf(f, "TP_STATUS_BLK_TMO ");
  fprintf(f,"\n");
}

char pk_names[]={
	'<', /* incoming */
	'B', /* broadcast */
	'M', /* multicast */
	'P', /* promisc */
	'>', /* outgoing */
};

struct if_ring_frame {
  struct tpacket2_hdr hdr;
  union {
    uint8_t raw_data[RING_FRAME_SIZE-sizeof(struct tpacket2_hdr)];
    struct {
      uint8_t pad0[TPACKET_ALIGN(sizeof(struct tpacket2_hdr)) - 
				 sizeof(struct tpacket2_hdr)]; 
      struct sockaddr_ll sll;
    } data_fields;
  } data;
};

int
scanIfRxRing(struct if_map_desc *md, int num)
{ 
  int i,cnt=0;
  struct pollfd pfd;
  struct if_ring_frame *frames, *f;

  assert(sizeof(struct if_ring_frame)==RING_FRAME_SIZE);
  assert(sizeof(struct if_ring_frame)==md->req.tp_frame_size);

  frames=md->rxbase;
  i = md->rx_scan_idx;
  f=&frames[i];

  do {
    while (f->hdr.tp_status!=TP_STATUS_KERNEL) { 
      cnt++;
      if (globals.opts.trace) {
        fprintf(stderr, "%d:%u.%.6u: if%u %c %u bytes ",
   	    i,
       	    f->hdr.tp_sec, f->hdr.tp_nsec,
   	    f->data.data_fields.sll.sll_ifindex,
  	    pk_names[f->data.data_fields.sll.sll_pkttype],
   	    f->hdr.tp_len);
        ifPrintStatus(f->hdr.tp_status, stderr);
      }
      /* tell the kernel this packet is done with */
      f->hdr.tp_status=TP_STATUS_KERNEL;
      mb(); /* memory barrier */
      i=(i==RING_NUM_FRAMES-1) ? 0 : i+1 ;
      f=&frames[i];
      if (num && cnt == num) break;
    }
    md->rx_scan_idx = i; 
    md->rx_cnt += cnt;
    if (num && cnt>=num) break;
#ifdef USE_POLL 
    /* Sleep when nothings happening */
    pfd.fd=md->fd;
    pfd.events=POLLIN|POLLERR;
    pfd.revents=0;
    poll(&pfd, 1, -1);
#endif
 } while (1);

  return i;
}
