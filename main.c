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

struct node_struct {
  char * line;
  struct node_struct * next;
};

struct history_struct {
  int size;
  struct node_struct * head;
};

int sh_cd(char **args);
int sh_pwd();
int sh_exit();
void intHandler(int sig);
void config();
void history_add(char* line);
void history_print_r(struct node_struct * node, int index);
int history_print();
char *sh_read_line(void);
char **sh_split_line(char *line_org);
int sh_execute(char **args);
int sh_process(char ** args);
int history_select(int index);
void sh_loop();


struct history_struct * history;


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

void intHandler(int sig) {
  printf("I'm sorry Dave, I can't do that\n");
}

void config()
{
  signal(SIGINT, intHandler);

  history = malloc(sizeof(struct history_struct));
  history->size = 0;
  history->head = NULL;
}

void history_add(char* line)
{
  struct node_struct * new = malloc(sizeof(struct node_struct));
  new->line = line;
  new->next = history->head;
  history->head = new;
  history->size++;

}

void history_print_r(struct node_struct * node, int index)
{
  if(node == NULL) {
    return;
  }
  else {
    history_print_r(node->next, index + 1);
    printf("%d\t%s\n", index, node->line);
  }
}

int history_print()
{

  history_print_r(history->head, 1);
  return 1;
}

char *sh_read_line(void)
{
	char *line = NULL;
	size_t buffer_size = 0;
	getline(&line, &buffer_size, stdin);
	return line;
}

char **sh_split_line(char *line_org)
{
  char * line = malloc(sizeof(*line_org));
  strcpy(line, line_org);
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

int sh_process(char ** args) {
  int status = 0;

  if(strcmp(args[0], "cd") == 0){
    status = sh_cd(args);
  }
  else if(strcmp(args[0], "pwd") == 0){
    status = sh_pwd();
  }
  else if(strcmp(args[0], "exit") == 0){
    status = sh_exit();
  }
  else if(strcmp(args[0], "history") == 0) {
    status = history_print();
  }
  else if(args[0][0] == '!') {
    int i = atol(args[1]);

    status = history_select(i);
  }
  else{
    status = sh_execute(args);
  }

  return status;
}

int history_select(int index) {
  if(index >= history->size || index <= 0) {
    printf("\nInvalid history value\n");
  }

  index = index - 1;

  struct node_struct * temp = history->head;

  for(int i = 0; i < index; i++) {
    temp = temp->next;
  }

  char** args = sh_split_line(temp->line);
  sh_process(args);
  free(args);
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

    status = sh_process(args);

    line[strlen(line)-1] = '\0';
    history_add(line);

		free(args);
	} while (status);
}

int main(int argc, char const *argv[])
{
  config();
	sh_loop();
	return 0;
}
