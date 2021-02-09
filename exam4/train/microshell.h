#ifndef MICROSHELL_H
#define MICROSHELL_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <stdio.h>


typedef enum{
	STRING = 0,
	PIPE,
	SEMI
} ttype;

typedef struct s_token{
	ttype type;
	char *data;
}		t_token;

typedef struct s_program{
	char **argv;
	char *path;
	pid_t pid;
	int count;
	int piped;
	int semicolon;
	int pipes[2];
}			t_program;

#endif