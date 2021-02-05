#ifndef MICROSHELL_H
# define MICROSHELL_H

#include <unistd.h>
#include <stdlib.h>
#include<string.h>
#include<stdio.h>
#include <sys/types.h>
typedef enum
{
    SEMI=0,
    PIPE,
    STRING

}ttype;

typedef struct  s_token{
    char *data;
    ttype type;
}               t_token;

typedef struct  s_program{
    char **argv;
    char *path;
    ttype type;
    pid_t pid;
}               t_program;
#endif