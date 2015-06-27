#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

int main(int argc, char **argv)
{
  int fd = open("/etc/passwd", O_RDONLY);
  fd = open("/etc/passwd", O_RDONLY);
  printf("open successfull \n");
  return fd;
}
