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
	if(argc > 0){
		char * last_arg = args[argc - 1];
		while(last_arg[i] != '\0'){
			if(last_arg[i] == '\n'){
				last_arg[i] = '\0';
				break;
			}
			i++;
		}
		//if cmd ends with spaces/tabs, then the last arg is "\n", so we have to decrement argc
		if(strcmp(last_arg, "") == 0){
			argc--;
		}
	}//end of if
	//must be terminated by NULL pointer, as execvp requires
	args[argc] = NULL;
	uc.num_args = argc;
	uc.args = args;
	return uc;
}
bool piro_exec(char * argv[]){
	pid_t pid;
	int status;
	if((pid = fork()) < 0){
		fprintf(stderr, "Error!\n");
		exit(-1);
	}else if (pid == 0){
		//child
		if(execvp(argv[0], argv) < 0){
			//execvp failed
			fprintf(stderr, "Error!\n");
			exit(-1);
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
	char *cmd;
	char *cwd;
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
		//handle exit
		if(strcmp(cmd, "exit\n") == 0){
			free(cmd);
			exit(0);
		}

		UserCommand uc = parse_cmd(cmd);
		if(uc.num_args == 0){
			fprintf(stderr, "Error!\n");
		}
		//cd
		else if(strcmp(uc.args[0], "cd") == 0){
			//fprintf(stdout, "%d\n", uc.num_args);
			//fprintf(stdout, "%s bla", uc.args[1]);
			if(uc.num_args == 1){
				if(chdir(getenv("HOME")) == -1){
					fprintf(stderr, "Error!\n");
				}
			}else if(uc.num_args == 2){
				if(chdir(uc.args[1]) == -1){
					fprintf(stderr, "Error!\n");
				}
			}else{
				fprintf(stderr, "Error!\n");
			}
			
		}
		//pwd
		else if (strcmp(uc.args[0], "pwd") == 0){
			//print current working dir
			cwd = (char *)malloc(sizeof(char) * MAX_CMD_LEN);
			if(cwd == NULL){
				fprintf(stderr, "Error!\n");
			}else if(getcwd(cwd, MAX_CMD_LEN) == NULL){
				fprintf(stderr, "Error!\n");
			}else{
				fprintf(stdout, "%s\n", cwd);
			}

		}else{
			piro_exec(uc.args);
		}
		
		if(cmd){
			free(cmd);
		}
	}

	exit(0);
}