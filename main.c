/*
* Authors: Austin Doolittle & Martin Mlinac
* Title; main.c
* Description: The main file for running the simple unix shell
*/
#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>

#define SIZE 64



int sh_cd(char **args)
{
	if(args[1] == NULL){
		perror("cd needs a directory");
	}else{
		if(chdir(args[1]) != 0){
			perror("not a directory");
		}
	}
	return 1;
}

int sh_pwd()
{
	char pwd[1024];
	if(getcwd(pwd, sizeof(pwd)) == NULL){
		perror("there was an error getting the cwd");
	}
	fprintf(stderr, "%s\n", pwd);
	return 1;
}

int sh_exit()
{
	exit(1);
}

char *sh_read_line(void)
{
	char *line = NULL;
	size_t buffer_size = 0;
	getline(&line, &buffer_size, stdin);
	return line;
}

char **sh_split_line(char *line)
{
	int bufsize = SIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if (!tokens) {
		fprintf(stderr, "error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, " \t\r\n\a");
	while (token != NULL) {
		tokens[position] = token;
		position++;

		if (position >= bufsize) {
			bufsize += SIZE;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) {
				fprintf(stderr, "error\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, " \t\r\n\a");
	}
	tokens[position] = NULL;
	return tokens;
}

int sh_execute(char **args)
{
	pid_t pid, wpid;
	int status;

	pid = fork();
	if (pid == 0) {
		if (execvp(args[0], args) == -1) {
			perror("error with exec");
		}
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		perror("error forking");
	} else {
		do {
			if(args[1] != NULL){
				if(strcmp(args[(sizeof(args)/sizeof(char))], "-&") != 0)
				{
					wpid = waitpid(pid, &status, WUNTRACED);
					printf("%d\n", status);
				}	
				else{
					printf("%d\n", pid);
				}
			}else{
				wpid = waitpid(pid, &status, WUNTRACED);
				printf("%d\n", status);
			}
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

void sh_loop()
{
	char *line;
	char **args;
	int status;

	do
	{
		printf(">");
		line = sh_read_line();
		args = sh_split_line(line);

		if(strcmp(args[0], "cd") == 0){
			status = sh_cd(args);
		}
		else if(strcmp(args[0], "pwd") == 0){
			status = sh_pwd();
		}
		else if(strcmp(args[0], "exit") == 0){
			status = sh_exit();
		}
		else{
			status = sh_execute(args);
		}

		free(line);
		free(args);
	} while (status);
}

int main(int argc, char const *argv[])
{
	sh_loop();
	return 0;
}