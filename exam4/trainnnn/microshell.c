#include "microshell.h"

t_token *g_tok = NULL;
t_program *g_prog = NULL;
int g_count = 1;

void *malloc_zero(size_t size)
{
    char *mem = NULL;
    mem = malloc(size);
    if (mem)
    {
    for (size_t i = 0; i < size; i++)
        mem[i] = 0;
    }
    return (mem);
}

void ft_putstr(int fd, char *str)
{
    if (str)
    {
        while (*str)
        {
            write(fd, str, 1);
            str++;
        }
    }
}

int terminate(char *str, int ret)
{
    if (str)
        ft_putstr(1, str);
    if (g_prog)
    {
        for (int i = 0; i < g_count ; i++)
        {
            if (g_prog[i].argv)
                free(g_prog[i].argv);
        }
        free(g_prog);
    }   
    if (g_tok)
        free(g_tok);
    while (1)
    {}
    exit(ret);
    return (ret);
}

int exit_fatal()
{   
    char *str = "error fatal";
    ft_putstr(2, str);
    terminate(NULL, EXIT_FAILURE);
    return(1);
}

int exit_execve(int ret)
{
     char *str = "error execve";
    ft_putstr(2, str);
     terminate(NULL, ret);
     return (ret);
}

int cd1()
{
     char *str = "bad arguments";
    ft_putstr(2, str);
     terminate(NULL, EXIT_FAILURE);
    return(1);
}

int cd2()
{
     char *str = "cd failed";
    ft_putstr(2, str);
     terminate(NULL, EXIT_FAILURE);
    return(1);
}



int main (int argc, char **argv, char **env)
{
    char **g_env = NULL;
    g_env = env;
 //   (void)env;
int ret = 0;
    if (argc < 2)
        return(0);
    
    if (!(g_tok = malloc_zero((argc -1) *sizeof(t_token))))
        return(exit_fatal());

    for (int i = 1; i < argc; i++)
    {
        char *cur = g_tok[i-1].data = argv[i];
        if (strcmp(cur, ";") == 0)
        {
            printf("cur\n");
            g_tok[i-1].type = T_SEMI;
            printf("type : %d\n", g_tok[i -1].type);
        }
        else if (strcmp(cur, "|") == 0)
            g_tok[i-1].type = T_PIPE;
        else
            g_tok[i-1].type = T_STRING;
        if (g_tok[i-1].type != T_STRING)
            g_count++;
    }
    if (!(g_prog = malloc_zero(g_count * (sizeof(t_program)))))
        return (exit_fatal());
    
    int start = 0;
    int j = 0;
//printf("c : %d\n", g_count);

    for (int i = 0; i < g_count; i++)
    {
       t_token *to = NULL;
        start = j;
        while (j < (argc -1))
        {
            
            to = &(g_tok[j]);
            printf("type : %d\n", g_tok[j].type);
            if (to->type != T_STRING)
            {
                printf("no string\n");
                if (g_tok[j].type == T_PIPE)
                    g_prog[i].piped = 1;
                if (g_tok[j].type == T_SEMI)
                    g_prog[i].semicolon = 1;
                //if (g_tok[j].type == PIPE || g_tok[i].type == SEMI)
                
                break;
            }
            j++;
        }
        //int c;
     
       printf("start: %d\n", start);
          printf("j: %d\n", j);
           g_prog[i].c = j - start;
           if ( g_prog[i].c == 0 && (!to || g_tok[j].type == T_SEMI))
        {
            printf("---------ICI\n");
            j++;
            continue;
        }
        // else if (g_prog[i].c == 0 )
        //     j++;
        // else if (g_tok[j].type == STRING)
        //     j++;
        // else if (c == 0)
        //     c++;
        
        if (!(g_prog[i].argv = malloc_zero((g_prog[i].c +1) * (sizeof(char*)))))
            return (exit_fatal());
        printf("c: %d\n", g_prog[i].c);
        for (int k = 0; k < g_prog[i].c; ++k)
        {
            g_prog[i].argv[k] = g_tok[start + k].data;
 //          printf("%s\n", g_prog[i].argv[k]);
        }
        g_prog[i].path = g_prog[i].argv[0];
        j++;
    }
int status;
    for (int i = 0 ; i < g_count; i++)
    {
        if (g_prog[i].argv)
        {
        int ret = 0;
        
        if (strcmp(g_prog[i].path, "cd") == 0)
        {
            if (g_prog[i].c <2)
                return (cd1());
            if (chdir(g_prog[i].argv[1]) == -1)
                return (cd2());
        }
        else
        {
             printf("path1: %s\n", g_prog[i].path);
            int pipe_fd[2] = {0, 0};
            int fd_in = 0;
            
            if (pipe(pipe_fd) < 0)
                return(exit_fatal());
            if ((g_prog[i].pid = fork()) == -1)
                return(exit_fatal());
            else if (g_prog[i].pid == 0)        
            {
                if (dup2(fd_in, 0) == -1)
                    return(exit_fatal());
                if (g_prog[i].piped)
                {
                    if (dup2(pipe_fd[1], 1) == -1)
                       return(exit_fatal());
                }
                close (pipe_fd[0]);
                printf("path: %s\n", g_prog[i].path);
                 printf("nb: %d\n", g_prog[i].c);
                if ((ret = execve(g_prog[i].path, g_prog[i].argv, g_env)) == -1)
                {
               //     printf("%s\n", strerror(errno));
                    return (exit_execve(ret));
                }
                exit(EXIT_FAILURE);
            }
            else
            {
                if (g_prog[i].semicolon || i == g_count -1)
                {
                    waitpid(g_prog[i].pid, &status, 0);
                    if (fd_in)
                        close(fd_in);
                    fd_in = 0;
                    if (WIFEXITED(status))
			            ret = WEXITSTATUS(status);
                }
                else
                {
                    fd_in = pipe_fd[0];
                }
                close (pipe_fd[1]);
               

            }
        }   
        
        }
        else 
            return (terminate(NULL, 1));
        for (int i = 0 ; i < g_count; i++)
        {
            waitpid(g_prog[i].pid, &status, 0);
            // if (WIFEXITED(status))
			//             ret = WEXITSTATUS(status);
        }



    }
    return (terminate(NULL, ret));

}