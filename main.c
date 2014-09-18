#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static int MAX_CMD_LEN = 1024;

typedef struct{
	int num_args;
	char ** args;
} UserCommand;

UserCommand parse_cmd(char * cmd){
	UserCommand uc;
	char * args[MAX_CMD_LEN + 1];
	char * token = strtok(cmd, " \t");
	int argc = 0;
	int i = 0;
	while(token != NULL){
		args[argc] = token;
		argc ++;
		token = strtok(NULL, " \t");
	}
	//strip \n off the last command
	char * last_arg = args[argc - 1];
	while(last_arg[i] != '\0'){
		if(last_arg[i] == '\n'){
			last_arg[i] = '\0';
			break;
		}
		i++;
	}
	uc.num_args = argc;
	uc.args = args;
	return uc;
}
bool piro_exec(char * argv[]){
	pid_t pid;
	int status;
	if((pid = fork()) < 0){
		fprintf(stderr, "Error!\n");
		return false;
	}else if (pid == 0){
		//child
		if(execvp(argv[0], argv) < 0){
			//execvp failed
			fprintf(stderr, "Error!\n");
			return false;
		}
	}else{
		//parent
		//wait for child with pid to complete
		waitpid(pid, &status, 0);
	}
	return true;
}
int main(int argc, char *argv[]){
	//main loop
	bool quit = false;
	int i;
	char *cmd;
	//char *ls[1];
	//ls[0] = "ls";
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
		//parse command
		if(strcmp(cmd, "exit\n") == 0){
			free(cmd);
			exit(0);
		}
		//piro_exec(ls);
		UserCommand uc = parse_cmd(cmd);
		printf("%d\n", uc.num_args);
		for(i = 0; i < uc.num_args; i++){
			printf("%s\n", uc.args[i]);
		}
		if(cmd){
			free(cmd);
		}
	}

	exit(0);
}