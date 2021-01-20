#ifndef MICROSHELL_H
# define MICROSHELL_H

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
    ttype type;
}               t_program;
#endif