#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/stat.h>

int 
dietlib_ioctl(int fd, int ioctlnum, void *data, unsigned long *size, unsigned long *ioctlrc)
{  
  int rc = -1;

  if (ioctlnum == TCGETS) {
    if (*size < sizeof(struct termios)) return -1;
    struct termios *ts = data;
    *size = sizeof(struct termios);
    *ioctlrc = ioctl(fd, TCGETS, ts);
    rc = 1;
  } 

  return rc;
}

int 
dietlib_fstat(int fd, void *data, unsigned long *size, unsigned long *fstatsc)
{
  int sc = -1;
  
  if (*size < sizeof(struct stat)) return -1;
  struct stat *filestat = data;
  *size = sizeof(struct stat);
  *fstatsc = fstat(fd, filestat);
  sc = 1;

  return sc; 
}
