#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdbool.h> //type bool
#define MAX_LENGTH	80 // The maximum length command
#define MAX_ARGS	MAX_LENGTH/2 //The maximum number of arguments
char temp[MAX_LENGTH];

//split string command to arguments
char** parse_arguments(char *command, bool* isAnd) {
	char delim[] = " \n\t\r\a";
	unsigned int args_size = MAX_ARGS;
	char** args = (char**)malloc(args_size * sizeof(char*));
	strcpy(temp, command);
    char* param;
    param = strtok(temp, delim);
    int size = 0;
    while (param != NULL) {
       	args[size] = param;
		if (++size >= args_size) {
			args_size += MAX_ARGS;
			args = (char**)realloc(args, args_size * sizeof(char*));
			if (!args) {
				perror("Allocaion error");
				exit(EXIT_FAILURE);
			}
		}
        param = strtok(NULL, delim);
    }
    args[size] = NULL;
	//check if the last agrugemt is "&"
    if (isAnd && strcmp("&", args[size-1]) == 0){
    	*isAnd = true;
    	args[size-1] = NULL; // delete the last "&" argument
    }
    return args;
}

void exec_command(char* command) {
	if (strcmp(command, "") == 0) {
		return;
	}
	//check if the last argurment is "&" or not (by using the "isAnd" variable)
	//if the last argurment is "&" then the parent and child processes will run concurrently
	bool isAnd = false;
	char** args = parse_arguments(command, &isAnd);
	//Fork a child process 
	pid_t child_pid = fork();
	
	if (child_pid < 0) {
		//Parent process, but child couldn't be created
		perror("Fork failed");
		free(args); // free up memory for arguments
		exit(EXIT_FAILURE);
	}

	if (child_pid == 0) { 
		//Child process
		if (execvp(args[0], args) < 0) { // catch error
			perror(args[0]);
			free(args); // free up memory for arguments
			exit(EXIT_FAILURE);
		}
	} 
	else if (child_pid > 0) { 
		int status = 0;
		pid_t wpid;
		//Parent process
		//Process wait if command have &
		if (!isAnd){
			while((wpid = wait(&status)) > 0);
		}
		//printf("parent finished\n");
	}
	free(args);
}

int main() {
	char command[MAX_LENGTH];
	int should_run = 1; /* flag to determine when to exit program */
	char empty_history[] = "No commands in history";
	char* history = NULL; // History buffer
	while (should_run) {
		printf("ssh> ");
		fflush(stdout);
		fgets(command, MAX_LENGTH, stdin);
		char** args = parse_arguments(command, NULL);
		if (!args[0]) { // if command is empty
			free(args);
			continue;
		}
		if (strcmp("!!", args[0]) != 0) {
			if (history) 
				free(history);
			history = (char*)malloc((strlen(command) + 1) * sizeof(char));
			strcpy(history, command);
		}
		if (strcmp("exit", args[0]) == 0){
			free(args);
			should_run = 0;
			break;
		}
		if (strcmp("!!", args[0]) == 0) { // Command from history buffer
			if (!history) {
				printf("%s\n", empty_history);
				fflush(stdout);
				free(args);
				continue;
			}
			else {
				printf("Command from history: %s\n", history);
				fflush(stdout);
				strcpy(command, history);
			}
		}
		free(args);
		exec_command(command);
	}
	return 0;
} 
