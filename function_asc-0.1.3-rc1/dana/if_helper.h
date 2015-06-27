#ifndef __IF_HELPER_H__
#include <net/if.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <net/ethernet.h>     /* the L2 protocols */
#include <sys/mman.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <asm/types.h>
#include <linux/sockios.h>

//#define USE_POLL 

#define RING_PAGE_SIZE  4096
#define RING_BLOCK_SIZE (RING_PAGE_SIZE * 1)
#define RING_FRAME_SIZE 1024
#define RING_NUM_BLOCKS 64
#define RING_TOTAL_SIZE (RING_BLOCK_SIZE * RING_NUM_BLOCKS)
#define RING_NUM_FRAMES (RING_TOTAL_SIZE / RING_FRAME_SIZE)

struct if_map_desc {
  int index;
  int mtu;
  int fd;
  int rx_scan_idx;
  int tx_scan_idx;
  size_t rx_cnt;
  void *rxbase;
  void *txbase;
  struct sockaddr sa;
  struct tpacket_req req;
  char name[IF_NAMESIZE + 1];  
};

int openByName(const char *name, int *fd, int *index,
	       struct sockaddr *hwaddr, int *mtu);
int listIf(void);
void mapIfDestroy(struct if_map_desc *);
int mapIfCreate(const char *, struct if_map_desc *);
int scanIfRxRing(struct if_map_desc *, int num);

#endif
