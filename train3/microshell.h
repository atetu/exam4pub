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

typedef struct s_token
{
    char *data;
    ttype type;
}   t_token;

typedef struct s_program
{
    char **argv;
    char *path;
    int count;
    pid_t pid;
    int piped;
    int semicolon;
    int pipes[2];
}   t_program;