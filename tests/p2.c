#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char *argv[]){
	char str[100];
	if(fgets(str, 99, stdin) == NULL){
		fprintf(stderr, "pipe read failed\n");
		exit(-1);
	}
	printf("blablabla\n");
	fprintf(stdout, "Output: %s\n", str);
	return 0;
}