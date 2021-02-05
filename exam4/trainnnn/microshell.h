#ifndef MICROSHELL_H
# define MICROSHELL_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>

typedef enum
{
    T_SEMI = 0,
    T_STRING,
    T_PIPE
} ttype;


typedef struct s_token{
    char *data;
    ttype type;
}                  t_token;

typedef struct s_program{
    char **argv;
    int piped;
    int semicolon;
    char *path;
    int c;
    pid_t pid;
}               t_program;
#endif