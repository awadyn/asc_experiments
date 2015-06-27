#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <inttypes.h>

#include "danasrv.h"
#include "uio_helper.h"
#include "if_helper.h"

struct Globals globals;

#define DANA_DEVICE_NAME "dana"

struct dana_desc {
  uint64_t *reg;
  struct uio_info_t uio_info;
};

int
mapDana(struct dana_desc *dd)
{
  struct uio_info_t  *uio_list, *uio;

  uio = uio_find_devices(-1);
  if (!uio) {
    fprintf(stderr, "ERROR: %s: no uio devices found\n", LOC_INFO);
    return -1;
  }

  uio=uio_list;
  while (uio) {
    uio_get_all_info(uio);
    if (strncmp(uio->name, DANA_DEVICE_NAME, strlen(DANA_DEVICE_NAME))==0) {
      memcpy(&(dd->uio_info), uio, sizeof(dd->uio_info));
      dd->uio_info.next=NULL;

      uio_get_device_attributes(&dd->uio_info);
      uio_mmap_all(&dd->uio_info);
      if (globals.opts.verbose) {
	uio_show_device(&dd->uio_info,stderr);
	uio_show_dev_attrs(&dd->uio_info,stderr);
	uio_show_maps(&dd->uio_info,stderr);
      }
      break;
    }
    uio = uio->next;
  }
  uio_free_info(uio_list); 

  if (dd->uio_info.maps[0].mmap_status != UIO_MMAP_OK) {
    fprintf(stderr, "ERROR: %s: failed to map dana device memory\n", LOC_INFO);
    return -1;
  }

  return 1;
}

static void
mapDanaDestroy(struct dana_desc *dd)
{
  if (dd) uio_single_release_dev(&(dd->uio_info));
}

void
usage()
{
  fprintf(stderr, "USAGE: danasrv [-h] [-v] [-c cnt] [ethdev]\n"
          "  -c <cnt> : count for packet to process (0 continous is default)\n" 
	  "  -h       : print usage\n"
	  "  -v       : be verbose\n"
	  "  ethdev   : ethernet device to monitor\n");
}

static int
process_args (int argc, char **argv)
{
  int opt;

  bzero(&globals.opts,sizeof(globals.opts));
  globals.opts.ifname="eth0";

  while ((opt = getopt(argc, argv, "c:ltvh")) != -1) {
    if (opt == EOF) break;
    switch (opt) {
    case 'c':
      globals.opts.count = atoi(optarg);
      break;
    case 'l': 
      {
	int n;
	n=listIf();
	printf("There are %d interfaces\n", n);
	exit(0);
      }
      break;
    case 't':
      globals.opts.trace = 1;
      break;
    case 'v' : 
      globals.opts.verbose = 1;
      break;
    case 'h' : 
      usage();
      exit(0);
      break;
    default:
      usage();
      exit(0);
    }
  }
  if (argc-optind > 0) globals.opts.ifname=argv[optind];
  if (globals.opts.verbose)
    fprintf(stderr, "opts.verbose=%d opts.ifname=%s\n", 
	    globals.opts.verbose,
	    globals.opts.ifname);
  return 0;
}

int
main(int argc, char **argv)
{
  struct if_map_desc if_map;
  struct dana_desc dana;
  char *ifname = "eth0";

  process_args(argc, argv);

  if (mapIfCreate(globals.opts.ifname, &if_map) < 0) {
    fprintf(stderr, "ERROR: %s: unable to map ethernet interface",
            LOC_INFO);
    return -1;
  } 

#ifdef MAP_DANA
  if (mapDana(&dana) < 0) {
    fprintf(stderr, "ERROR: %s: unable to map dana interface\n",
	    LOC_INFO);
    return -1;
  }
#endif

  scanIfRxRing(&if_map, globals.opts.count);

#ifdef MAP_DANA
  mapDanaDestroy(&dana);
#endif

  mapIfDestroy(&if_map);

  return 0;
}
 
