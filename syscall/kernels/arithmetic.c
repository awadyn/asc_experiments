#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {

	long num, operand;
	char op;

	if (argc == 2) {
		num = atol(argv[1]);
	}
	else {
		num = 0;
	}

	long ctr;
	for(ctr = 0; ctr < 5; ctr ++) {
		printf("Enter operand: \n");
		scanf("%ld", &operand);
		printf("Enter operator: \n");
		scanf(" %c", &op);
		switch(op) {
			case '+':
				num += operand;
				break;
			case '-':
				num -= operand;
				break;
			case '*':
				num *= operand;
				break;
			case '/':
				if(operand == 0) {
					printf("DIVISION BY ZERO \n");
					return -1;
				} else {
					num /= operand;
					break;
				}
			default:
				printf("invalid operator \n");
		}
		printf("num = %ld \n", num);
	}

	return 0;
}
