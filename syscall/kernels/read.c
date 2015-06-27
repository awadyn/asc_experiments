#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h>


int main(int argc, char* argv[]) {

	long counter;

	long num;
	if(argc == 2)
		num = atol(argv[1]);
	else
		num = 0;

	printf("num = %ld \n", num);


	for (counter = 0; counter < 20;) {
		scanf("%ld", &num);
		counter ++;
      		printf("num #%ld = %ld \n", counter, num);
        	scanf("%ld", &num);
		counter ++;
      		printf("num #%ld = %ld \n", counter, num); 
	}

	struct termios ts;
	printf("R: BEFORE: &ts=%p sizeof(ts)=%ld\n", &ts, sizeof(ts));
	int rc = ioctl(0, TCGETS, &ts);
	unsigned long j;
	if (rc == 0) {
		printf("R: AFTER: rc=%d &ts=%p sizeof(ts)=%ld\n", rc, &ts, sizeof(ts));
		for (j = 0; j < sizeof(ts); j ++) {
			printf("R: ts[%ld]=%02x\n", j, ((char *)&ts)[j]);
		}
	}

  printf("\n EXITING... BYE\n");
  return 0;
}
