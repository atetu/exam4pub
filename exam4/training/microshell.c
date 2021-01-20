#include "microshell.h"
#include <unistd.h>
#include <stdlib.h>
#include<string.h>
#include<stdio.h>

int g_count = 1;
t_token *g_tokens = NULL;
t_program *g_program = NULL;
char **g_env;

void* malloc_zero(size_t size)
{
   // size = 16;
    // printf("size: %zu\n", size);fflush(stdout);
   char *mem = NULL;
//    printf("Puet\n");fflush(stdout);
if (!(mem = malloc(size)))
        printf("Error\n");fflush(stdout);
    // printf("Puet\n");fflush(stdout);
    if (mem)
    {
    for (size_t i = 0 ; i < size; i++)
    {
        mem[i] = 0;
    }
    }
    // printf("pouet\n");
    return ((void *)mem);
}



int main(int argc, char **argv, char **env)
{
    if (argc < 2)
        return(0);
    g_env = env;
    g_tokens = malloc_zero((argc-1) * sizeof(t_token)); // ICIC
    for (int i = 1; i < argc ; ++i)
    {
        char *cur = NULL;
        cur = g_tokens[i-1].data = argv[i];
        printf("%s\n", argv[i]);
        if (strcmp (cur, ";") == 0)
            g_tokens[i-1].type = SEMI;
        else if (strcmp (cur, "|") == 0)
        {
            // printf("ici\n");
            g_tokens[i-1].type = PIPE;
        }
        else
        {
            g_tokens[i-1].type = STRING;
        }
        
        if (g_tokens[i-1].type != STRING)
            g_count++;
        
    }
   
//exit(1);
    g_program = malloc_zero((g_count) * sizeof(t_program)); // ICI

    int start = 0;
    t_token *tok = NULL;
   int j = start;
//    printf("j\n");fflush(stdout);
    for (int i = 0 ; i < g_count; i++)
    {
        // printf("j: %d\n", j);fflush(stdout);
        start = j;
        
        while (j < (argc-1))
        {
            tok = &(g_tokens[j++]);
            if (g_tokens[j].type == SEMI)
                g_program[i].type = SEMI;
            else if (g_tokens[j].type == PIPE)
                g_program[i].type = PIPE;
            else
            {
                g_program[i].type = STRING;
            }
            if (g_program[i].type != STRING)
                break;
        }
        // printf("la\n");
        //  printf("j: %d\n", j);fflush(stdout);
        //  printf("sart: %d\n", start);fflush(stdout);
        //  printf("argc: %d\n",argc);fflush(stdout);
        int arg_count = j - start - (tok != NULL && tok->type != STRING) ;
        printf("count: %d\n", arg_count);fflush(stdout);

        g_program[i].argv = malloc_zero((arg_count +1) * sizeof(char*));
        
        // printf("j: %d\n", j);fflush(stdout);
        //  printf("sart: %d\n", start);fflush(stdout);
         j++;
        for (int x = 0 ; x < arg_count; ++x)
        {
            g_program[i].argv[x] = g_tokens[start+x].data;
            printf("%s\n", g_tokens[start+x].data);
        }
    }
    int pipe_fd[2] ={0, 0};
    int fd_in = 0;

    for (int i = 0; i < g_count; i++)
    {
        dup2(fd_in, 0);
        if ()
        g_program[i]
    }
}