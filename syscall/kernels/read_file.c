#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {

	FILE *numbers;
	numbers = fopen("numbers", "r");

	printf("fopen \n");
/*
	long num = 0;
	long temp;
	int ctr = 0;


	while (fscanf(numbers, "%ld ", &temp) == 1) {
		num += temp;
		if (ctr = 0) 
			ctr = 1;
		else
			ctr = 0;
		printf("%ld \n", num);

	}
*/
	fclose(numbers);
	return 0;
}
