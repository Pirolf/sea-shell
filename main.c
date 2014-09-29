#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static int MAX_CMD_LEN = 1024;
static int OVERWRITE = 1;
static int APPEND = 2;
static int PIPE = 3;

typedef struct{
	int num_args;
	int pipe_idx;
	char ** args;
} UserCommand;

UserCommand parse_cmd(char * cmd){
	UserCommand uc;
	uc.pipe_idx = -1;
	char * args[MAX_CMD_LEN + 1];
	char * token = strtok(cmd, " \t");
	int argc = 0;
	int i = 0;

	while(token != NULL){
		if(strcmp(token, "|") == 0){
			uc.pipe_idx = argc;
		}
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
	}//end of 
	//must be terminated by NULL pointer, as execvp requires
	args[argc] = NULL;
	uc.num_args = argc;
	uc.args = args;
	return uc;
}

// args1 | args2
void exec_pipe(UserCommand uc, char * args1[], char * args2[]){
	int fd[2];
	pid_t pid;

	if(pipe(fd) < 0){
		fprintf(stderr, "Error!\n");
		exit(-1);
	}

	if((pid = fork()) < 0  ){
		fprintf(stderr, "Error!\n");
		exit(-1);
	}else if (pid == 0){
		close(1);       //close normal stdout 
		// make stdout same as fd[1] 
		if (dup2(fd[1], 1) < 0) {
			fprintf(stderr, "Error!\n");
			exit(-1);
		}  
    		close(fd[0]); // we don't need this

    		if(execvp(uc.args[0], args1) < 0){
			//execvp failed
    			fprintf(stderr, "Error!\n");
			exit(-1);
    		}
    	}
	else {
		close(0);    
		if (dup2(fd[0], 0) < 0) {
			fprintf(stderr, "Error!\n");
			exit(-1);
		}  
		close(fd[1]);

    		if(execvp(uc.args[uc.pipe_idx + 1], args2) < 0){
    			fprintf(stderr, "Error!\n");
    			exit(-1);
    		}
    	}
}
//returns a copy of arr[start..end]
void subarr (char * arr[], int start, int end, char ** sub){
	int i;
	for(i = start; i <= end; i++){
		sub[i-start] = arr[i]; 
	}
}
bool piro_exec(char * argv[], int argv_len, int flag, int pipe_idx){
	pid_t pid;
	int status;

	if((pid = fork()) < 0){
		fprintf(stderr, "Error!\n");
		exit(-1);
	}else if (pid == 0){
		//child
		//if < or <<
		if(flag == OVERWRITE || flag == APPEND){
			int fw;
			if (flag == OVERWRITE){
				fw = open(argv[argv_len - 1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			}else{
				fw = open(argv[argv_len - 1], O_WRONLY | O_APPEND  | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			}

			if(fw < 0){
				fprintf(stderr, "fw Error!\n");
				exit(-1);
			}
			if(dup2(fw, 1) < 0){
				fprintf(stderr, "dup Error!\n");
				exit(-1);
			}
			//close(fw);
			//argv[0..-3]
			char *args[argv_len - 1];
			subarr(argv, 0, argv_len - 3, args);
			args[argv_len - 2] = NULL;

			if(execvp(argv[0], args) < 0){
				//execvp failed
				fprintf(stderr, "exec 1 Error!\n");
				exit(-1);
			}
		}
		else if(execvp(argv[0], argv) < 0){
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

		}
		else{
			//check for >, >>, |
			int flag = 0;
			if(uc.num_args >= 3){
				char * last_but_two = uc.args[uc.num_args - 2];
				if(strcmp(last_but_two, "<") == 0){
					flag = OVERWRITE;
				}else if (strcmp(last_but_two, "<<") == 0){
					flag = APPEND;
				}else if (uc.pipe_idx > 0){
					flag = PIPE;
				}
			}
			
			if(flag == PIPE){
				char *args1[uc.pipe_idx + 1];
				char *args2[uc.num_args - uc.pipe_idx];

				subarr(uc.args, 0, uc.pipe_idx - 1, args1);
				args1[uc.pipe_idx] = NULL;

				subarr(uc.args, uc.pipe_idx + 1,  uc.num_args - 1, args2);
				args2[uc.num_args - uc.pipe_idx - 1] = NULL;

				//set up pipe
				int pid, status;
				if((pid = fork()) == 0){
					exec_pipe(uc, args1, args2);
				}else{
					wait(&status);
				}
				

			}//end if
			else{
				piro_exec(uc.args, uc.num_args, flag, uc.pipe_idx);
			}

		}
		
		if(cmd){
			free(cmd);
		}
	}

	exit(0);
}