#include "microshell.h"
int g_count = 1;
t_program *g_prog = NULL;
t_token *g_tok = NULL;
char **g_env = NULL;
int g_ret = 0;

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
	if (str)
	{
		while (*str)
		{
			write(fd, str, 1);
			str++;
		}
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
	{
		free(g_tok);
		g_tok = NULL;
	}
	// while(1)
	// {}
	exit(ret);
	return (ret);
}

int exit_fatal()
{
	ft_putstr(2, "error : exit fatal\n");
	terminate(1);
	return (1);
}

int exit_execve(char *path)
{
	ft_putstr(2, "error : execve ");
	ft_putstr(2, path);
	ft_putstr(2, "\n");
	return (1);
}

int cd1()
{
	ft_putstr(2, "error : cd: bad arguments\n");
	return (1);
}

int cd2(char *path)
{
	ft_putstr(2, "error : cd: cannot change directory to ");
	ft_putstr(2, path);
	ft_putstr(2, "\n");
	return (1);
}

void control_c(int sig)
{
	if (g_prog)
	{
		for (int i = 0; i < g_count; i++)
		{
			if (g_prog[i].pid != 0 && g_prog[i].pid != -1)
				kill(g_prog[i].pid, sig);
		}
	}
	g_ret = 130;
}

void control_quit(int sig)
{
	if (g_prog)
	{
		for (int i = 0; i < g_count; i++)
		{
			if (g_prog[i].pid != 0 && g_prog[i].pid != -1)
				kill(g_prog[i].pid, sig);
		}
	}
	g_ret = 131;
}

int main(int argc, char **argv, char **env)
{
	g_env = env;
	
	int status;

	if (signal(SIGINT, &control_c) == SIG_ERR)
		return (exit_fatal());
	if (signal(SIGQUIT, &control_quit) == SIG_ERR)
		return (exit_fatal());
	if (argc == 1)
		return (0);

	if (!(g_tok = malloc_zero((argc-1) * (sizeof(t_token)))))
		return (exit_fatal());
	
	for (int i = 1; i < argc; i++)
	{
		char *cur = g_tok[i-1].data = argv[i];
		if (strcmp(cur, ";")== 0)
			g_tok[i-1].type = SEMI;
		else if (strcmp(cur, "|") == 0)
			g_tok[i-1].type = PIPE;
		else
			g_tok[i-1].type = STRING;
		if (g_tok[i-1].type != STRING)
			g_count++;
	}

	if (!(g_prog = malloc_zero(g_count * (sizeof(t_program)))))
		return(exit_fatal());

	int start = 0;
	int j = 0;
	t_token *tok = NULL;
	for (int i = 0; i < g_count; i++)
	{
		start = j;
		tok = NULL;
		while (j < argc -1)
		{
			tok = &(g_tok[j]);
			if (tok->type != STRING)
			{
				g_prog[i].piped = tok->type == PIPE;
				g_prog[i].semicolon = tok->type == SEMI;
				break;
			}
			j++;
		}
		
		int c = g_prog[i].count = j - start;
		
		if (c == 0 && (!tok || tok->type == PIPE || tok->type == SEMI)) // verifier avec un arg string == 0
		{
			j++;
			continue;
		}
		
		if(!(g_prog[i].argv = malloc_zero((c + 1) * (sizeof( char *)))))
			return (exit_fatal());
	
		for (int k = 0; k < c; k++)
		{
			g_prog[i].argv[k] = g_tok[start + k].data;
			//printf("%s\n", g_prog[i].argv[k]);
		}
		g_prog[i].path = g_prog[i].argv[0];
		j++;
	}

	int fd_in =0;
	t_program *pr = NULL;
	int pstart = 0;
	int pipe_tube = 0;
	for (int i =0 ; i < g_count; i++)
	{
		pr = &(g_prog[i]);
		if (pr->argv)
		{
			if (strcmp(pr->path, "cd")== 0)
			{
				g_ret = 0;
				if (pr->count != 2)
				{
					cd1();
					g_ret = 1;
					continue;
				}
				if (chdir(pr->argv[1]) == -1)
				{
					cd2(pr->argv[1]);
					g_ret = 1;
					continue;
				}
			}
			else
			{
				if (pr->piped)
				{
					if (!pstart)
					{
						pipe_tube = 1;
						pstart = i;
					}
					if (pipe(pr->pipes) == -1)
						return (exit_fatal());
					// printf("pipe 0: %d\n", pr->pipes[0]);
					// printf("pipe 1: %d\n", pr->pipes[1]);
				}
					if ((pr->pid = fork()) == -1)
						return (exit_fatal());
					else if (pr->pid == 0)
					{
						if (fd_in)
						{
							if (dup2(fd_in, 0) == -1)
								return (exit_fatal());
							if (close(fd_in) == -1)
								return (exit_fatal());
						}
						if (pr->piped)
						{
							if (dup2(pr->pipes[1], 1) == -1)
								return (exit_fatal());
							if (close(pr->pipes[1]) == -1)
								return (exit_fatal());
							if (close(pr->pipes[0]) == -1)
								return (exit_fatal());
						}
						if (execve(pr->path, pr->argv, g_env) == -1)
						{
							exit_execve(pr->path);
							close(0);
							close(1);
							exit (1);
						}
						exit(0);
					}
					else
					{
						if (pr->semicolon || i == g_count -1)
						{
							if (pipe_tube)
							{
								while(pstart != i)
								{
									if (g_prog[pstart].pid == 0 || g_prog[pstart].pid == -1)
										waitpid(g_prog[pstart].pid, &status, 0);
									g_prog[pstart].pid = 0;
									if (WIFEXITED(status))
										g_ret = WEXITSTATUS(status);
									if (pstart > 0  && g_prog[start-1].piped)
									{
										//printf("ICI\n");
										close(g_prog[pstart-1].pipes[0]);
										close(g_prog[pstart-1].pipes[1]);
									}
									pstart++;
								}
								if (pstart > 0  && g_prog[start-1].piped)
								{
								//	printf("ICI\n");
									close(g_prog[pstart-1].pipes[0]);
									close(g_prog[pstart-1].pipes[1]);
								}
								pstart = 0;
								pipe_tube = 0;
							}
							waitpid(pr->pid, &status, 0);
							if (WIFEXITED(status))
								g_ret = WEXITSTATUS(status);
							if (pr->piped && close(pr->pipes[0] == -1))
								return (exit_fatal());
							fd_in = 0;
						}
						else
						{
							fd_in = pr->pipes[0];
						}
						if (pr->piped && close(pr->pipes[1]) == -1)
							return (exit_fatal());
					
					}
					
				}
			}
			
		}

	return (terminate(g_ret));
}