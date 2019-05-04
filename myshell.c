#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <pwd.h>

#include "makeargv.c"

#define BUFFSIZ 1024
#define TRUE 1
#define FALSE 0

char cwd[BUFFSIZ];
char **commands;
int bg;
int commandNum;

int check_command_exist(char* command);
void getCWD();
int Exec_Command(int commandNum);
int Multi_Pipes(int left, int right);
int with_redirect(int left, int right);
int CD(int commandNum);
void gethelp();

int main() {
	getCWD();
	char argv[BUFFSIZ];
	while (1) {
		printf("[pid: %d]:%s $ ",getpid(), cwd); // display the shell
		//get user input
		fgets(argv, BUFFSIZ, stdin);
		int len = strlen(argv);
		if (len != BUFFSIZ) {
			argv[len-1] = '\0';
		}
	  commandNum = makeargv(argv," \n\r ", &commands);
	if (commandNum != 0) { //check if user has typed command
			if (strcmp(commands[0], "exit") == 0 || strcmp(commands[0], "quit") == 0) { // exit command
					  exit(0);
			} 
				if (strcmp(commands[0], "help") == 0){
           gethelp();
				}
			if (strcmp(commands[0], "cd") == 0) { // cd command
			
			  int check_num = CD(commandNum);
			  if(check_num == -1){
				fprintf(stderr, "Error: Invalid Directory!.\n");
			  }
			  if(check_num == 0){
				getCWD();
			  }
			}
	    else { 
			 char * last = commands[commandNum-1];
			if (commandNum > 1 && strcmp(&last[strlen(last-1)], "&")==0){
				  bg = 1 ;
			 }
				Exec_Command(commandNum);
               bg = 0;	
			}
		}
	}
}

void getCWD() { // get current directory
	getcwd(cwd, BUFFSIZ);
}


int Exec_Command(int commandNum) { 
	pid_t pid = fork();
	if (pid == -1) {
		return -1;
	} 
	if (pid == 0) {
		if(bg == 1){
			setpgid(0,0);
		}
		//get standard input, output file symbol
		int fd_in = dup(STDIN_FILENO);
		int fd_out = dup(STDOUT_FILENO);
        
		int check_num = Multi_Pipes(0, commandNum);
		//restore standard input and output redirection
		dup2(fd_in, STDIN_FILENO);
		dup2(fd_out, STDOUT_FILENO);
		exit(check_num);
	}else {
		int status;
        if(bg != 1){
        waitpid(pid, &status, 0);
        }
		return WEXITSTATUS(status);	
	}
}

int Multi_Pipes(int left, int right) { //command of left, right, might have pipe
	if (left >= right) 
	return 0;
	//if there is pipe
	int pipe_num = -1; 
	for (int i=left; i<right; ++i) {
		if (strcmp(commands[i], "|") == 0) {
			pipe_num = i;
			break;
		}
	}
	if (pipe_num == -1) { // doesn't contain "pipe" in the command
		return with_redirect(left, right);
	} else if (pipe_num+1 == right) {   //has "pipe" but no command after that, miss parameter
		return -1;
	}
	// exec command;
	int fds[2];
	if (pipe(fds) == -1) {
		return -1;
	}
	int check_num = 0;
	pid_t pid = fork();
	
	if (pid == -1) {
		check_num = -1;
	} 
	if (pid == 0) {
        if(bg == 1){
            setpgid(0,0);
        }
        // child runs single command
		close(fds[0]);
		dup2(fds[1], STDOUT_FILENO); // standard output redirect to fds[1]
		close(fds[1]);
		
		check_num = with_redirect(left, pipe_num);
		exit(check_num);
	} else { 
		int status;
		if (pipe_num+1 < right){
			close(fds[1]);
			dup2(fds[0], STDIN_FILENO); // redirect to fds[0]
			close(fds[0]);
			check_num = Multi_Pipes(pipe_num+1, right); // recursively run next commands
		}
        if(bg != 1){
            waitpid(pid, &status, 0);
        }      
	}  
	return check_num;
}

int with_redirect(int left, int right) { 
	// judge if there is redirect
	int num_in = 0, num_out = 0;
	char *input_file = NULL, *output_file = NULL;
	int end = right; 
	for (int i=left; i<right; ++i) {
		if (strcmp(commands[i], "<") == 0) { // input redirect
			++num_in;
			if (i+1 < right)
				input_file = commands[i+1];
			else 
			return -1; // missing file name

			if (end == right) 
			end = i;
		} else if (strcmp(commands[i], ">") == 0) { // output redirect
			++num_out;
			if (i+1 < right)
				output_file = commands[i+1];
			else 
			    return -1; // missing filename after symbol of redirect
			if (end == right) 
			    end = i;
		}
	}
	//deal redirect
	if (num_in == 1) {
		FILE* fp = fopen(input_file, "r");
		if (fp == NULL) // input redirect file not exist
			return -1;
		  fclose(fp);
	}
	int check_num = 0;
	pid_t pid = fork();
	if (pid == -1) {
		check_num = -1;
	} 
	if (pid == 0) {
        if(bg == 1){
            setpgid(0,0);
        }
		//redirect of input&output
		if (num_in == 1)
			freopen(input_file, "r", stdin);
		if (num_out == 1)
			freopen(output_file, "w", stdout);
	
	  if (num_in > 1) { // more than one input redirect symbol
		 return -1;
	 } else if (num_out > 1) { // more than one output redirect symbol command
		 return -1;
	 }
		//exec commands
		char* cmds[BUFFSIZ];
        if(bg == 1){
            end= end -1;
            for (int i= left; i< end; ++i){
            cmds[i] = commands[i];		 
         }
        }
        for (int i=left; i<end; ++i){
		       cmds[i] = commands[i];
           cmds[end] = NULL;     
				}	
	   	  execvp(cmds[left], cmds+left);
	    	exit(errno); 
	}
	 else {
		int status;
         if(bg != 1){
             waitpid(pid, &status, 0);
         }
	}
	return check_num;
}

//handle cd command
int CD(int commandNum) { // cd command
		int ret = chdir(commands[1]);
		if (ret){
		return -1;
		}
	return 0;
}

void gethelp(){
  printf("-------------Custom Shell------------------\n");
	printf("------------Version: 1.0.0-----------------\n");
	printf("JOB_SPEC [&]                       (( expression ))\n");
	printf(". filename [arguments]             :\n");
	printf("case WORD in [PATTERN [| PATTERN]. cd [-L|-P] [dir]\n");
	printf("exec [-cl] [-a name] file [redirec exit [n]\n");
	printf("Backgrounding the shell            & [space before &]\n");
	printf("-------------------------------------\n");
}
