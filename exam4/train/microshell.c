#include "microshell.h"

int g_count = 1;
t_program *g_prog = NULL;
t_token *g_tok = NULL;
int g_ret = 0;
char **g_env = NULL;

void *malloc_zero(size_t size)
{
	char *mem = NULL;
	mem = malloc(size);
	if (mem)
	{
	for (size_t i = 0; i < size; i++)
	{
		mem[i] = 0;
	}
	}
	return (mem);
}

void ft_putstr(int fd, char *str)
{
	while (*str)
	{
		write (fd, str, 1);
		str++;
	}
}

int terminate(int ret)
{
	if (g_prog)
	{
		for (int i = 0 ; i < g_count; i++)
		{
			if (g_prog[i].argv)
			{
				free(g_prog[i].argv);
				g_prog[i].argv = NULL;
			}
		}
		free(g_prog);
		g_prog = NULL;
	}
	if (g_tok)
		free(g_tok);
	while (1)
	{

	}
	exit(ret);
	return (ret);
}

int exit_fatal()
{
	ft_putstr(2, "error:fatal\n");
	terminate(1);
	return (1);
}

int exit_execve(char *path)
{
	ft_putstr(2, "error: execve \n");
	ft_putstr(2, path);
	ft_putstr(2, "\n");

	return (1);
}

int cd1()
{
	ft_putstr(2, "bad arguments\n");
	terminate(1);
	return (1);
}

int cd2(char *str)
{
	ft_putstr(2, "error cd to ");
	ft_putstr(2, str);
	ft_putstr(2, "\n");
	terminate(1);
	return (1);
}

void control_c(int sig)
{
	if (g_prog)
	{
		for (int i = 0; i < g_count; i++)
		{
			kill(g_prog[i].pid, sig);
		}
	}
	g_ret = 127;
}

void control_quit(int sig)
{
	if (g_prog)
	{
		for (int i = 0; i < g_count; i++)
		{
			kill(g_prog[i].pid, sig);
		}
	}
	g_ret = 130;
}
int main (int argc, char **argv, char **env)
{
	g_env = env;
	int status;

	if (signal(SIGINT, &control_c) == SIG_ERR)
	{
		return(exit_fatal());
	}
	if (signal(SIGQUIT, &control_quit) == SIG_ERR)
	{
		return(exit_fatal());
	}

	if (argc == 1)
		return (0);

	if (!(g_tok = malloc_zero((argc - 1) * (sizeof(t_token)))))
		return (exit_fatal());
	
	for (int i = 0; i < argc; i++)
	{
		char *cur = g_tok[i-1].data = argv[i];
		if (strcmp(cur, ";") == 0)
			g_tok[i- 1].type = SEMI;
		else if (strcmp(cur, "|") == 0)
			g_tok[i-1].type = PIPE;
		else
			g_tok[i-1].type = STRING;
		if (g_tok[i-1].type != STRING)
			g_count++;
	}

	if (!(g_prog = malloc_zero(g_count * (sizeof(t_program)))))
		return (exit_fatal());

	int j = 0;
	int start = 0;
	t_token *tok;
	for (int i = 0 ; i < g_count; i++)
	{
		
		start = j;
		tok = NULL;
		while (j < (argc -1))	
		{
			tok = &(g_tok[j]);
		//	printf("type : %u\n", tok->type);
			if (tok->type != STRING)
			{
				g_prog[i].piped = tok->type == PIPE;
				g_prog[i].semicolon = tok->type == SEMI;
				break;
			}
			j++;
		}
		g_prog[i].count = j - start;
	//	printf("count : %d\n", g_prog[i].count);
		if (g_prog[i].count == 0 && (!tok || tok->type == SEMI))
		{
			j++;
			continue;
		}
		if (!(g_prog[i].argv = malloc_zero((g_prog[i].count + 1) * (sizeof(char *)))))
			return (exit_fatal());
		for (int k = 0; k < g_prog[i].count; k++)
		{
			g_prog[i].argv[k] = g_tok[start + k].data;
		//	printf("%s\n", g_prog[i].argv[k]);
		}
		j++;
		g_prog[i].path = g_prog[i].argv[0];
	}

	int fd_in  = 0;

	for (int i =0; i < g_count; i++)
	{
		if (g_prog[i].argv)
		{
			if (strcmp(g_prog[i].path, "cd") == 0)
			{
				if (g_prog[i].count < 2)
					return (cd1());
				if (chdir(g_prog[i].argv[1]) == -1)
					return (exit_fatal());
			}
			else
			{
			
				if (g_prog[i].piped)
				{
					if (pipe(g_prog[i].pipes) == -1)
						exit_fatal();
				}
					
				if ((g_prog[i].pid = fork()) == -1)
					return (exit_fatal());
			
				if (g_prog[i].pid == 0)
				{
						if (fd_in)
					{
						if ((dup2(fd_in, 0) == -1))
						{
							printf("fd in\n");
							exit_fatal();
						}
						close (fd_in);
					}
					if (g_prog[i].piped)
					{
						if (dup2(g_prog[i].pipes[1], 1) == -1)
							return (exit_fatal());
						if (close (g_prog[i].pipes[1]) == -1)
							return (exit_fatal());
						if (close (g_prog[i].pipes[0]) == -1)
							return (exit_fatal());
					}
					if (execve(g_prog[i].path, g_prog[i].argv, g_env) == -1)
					{
						exit_execve(g_prog[i].path);
						exit(1);
					}
				}
				else
				{
					if (g_prog[i].semicolon || i == g_count -1)
					{
						if (g_prog[i].pid == -1)
								;
						else
						{	
						waitpid(g_prog[i].pid, &status, 0);
						if (WIFEXITED(status))
							g_ret = WEXITSTATUS(status);
					//	printf("ret: %d\n", g_ret);
						}
						fd_in = 0;
						close(g_prog[i].pipes[0]);
						
					}
					else
					{
						fd_in = g_prog[i].pipes[0];
					}
					if (g_prog[i].piped && close(g_prog[i].pipes[1]) == -1)
					{
						printf("ici");
						return (exit_fatal());
					}
					if (i> 0 && g_prog[i-1].piped)
					{
						if (close(g_prog[i-1].pipes[0]) == -1)
						{
							printf("close\n");
							return (exit_fatal());
						}
					}
					
				}
				
			}
		}
		else
		{
		return (terminate(1));
			
		}
	}
	
	
	return (terminate(g_ret));
}