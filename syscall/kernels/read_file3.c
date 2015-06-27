#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc, char **argv)
{

  int fd = open("/etc/passwd", O_RDONLY);
  fd = open("/etc/passwd", O_RDONLY);
  printf("fd = %d \n", fd);
  printf("open successfull \n");

  void* map = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  printf("map successful \n");


  struct stat filestat;
  struct stat filestat2;
  int stat = fstat(2, &filestat2);
  printf("stat = %d \n", stat);
  printf("fstat successful \n");

  return 0;
}
