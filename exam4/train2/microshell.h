#ifndef MICROSHELL_H
#define MICROSHELL_H

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include <stdio.h>

typedef enum{
	STRING=0,
	PIPE,
	SEMI
} ttype;

typedef struct s_token{
	char *data;
	ttype type;
}	t_token;

typedef struct s_progam{
	char **argv;
	char *path;
	int count;
	int pipes[2];
	int piped;
	int semicolon;
	pid_t pid;
} 	t_program;

#endif 