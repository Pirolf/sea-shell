#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

static int MAX_CMD_LEN = 1024;
int main(int argc, char *argv[]){
	//main loop
	bool quit = false;
	char *cmd;
	cmd = (char *)malloc(sizeof(char) * (MAX_CMD_LEN + 1));
	if(cmd == NULL){
		exit(1);
	}
	while(!quit){
		//print prompt
		printf("myshell>");

		//handle user input, +1 for null teminator
		cmd = malloc(sizeof(char) * (MAX_CMD_LEN + 1));
		if(cmd == NULL){
			exit(1);
		}
		cmd = fgets(cmd, MAX_CMD_LEN + 1, stdin);
		if(cmd == NULL){
			exit(1);
		}
		printf("%s", cmd);
		//parse command
		if(strcmp(cmd, "exit\n") == 0){
			exit(0);
		}
		if(cmd){
			free(cmd);
		}
	}

	exit(0);
}