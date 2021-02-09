#include "microshell.h"

int g_count = 1;
t_program *g_prog = NULL;
t_token *g_tok = NULL;
int g_ret;
char **g_env = NULL;

void *malloc_zero(size_t size)
{
	char *mem = NULL;
	mem = malloc(size);
//	printf("size : %zu\n", size);
	if (mem)
	{
		for (size_t i =0 ; i < size ; i++)
		{
			mem[i] =0 ;
//			printf("i : %zu\n", i);
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
			write (fd, str, 1);
			str++;
		}
	}
}

int terminate(int ret)
{
	if (g_prog)
	{
		for (int i =0 ; i < g_count; i++)
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
	// while (1)
	// {

	// }
//	printf("ret: %d\n", ret);
	exit (ret);
	return (ret);
}

int exit_fatal()
{
	ft_putstr(2, "error fatal\n");
	terminate(1);
	return (1);
}

int exit_execve(char *path)
{
	ft_putstr(2, "error execve : ");
	ft_putstr(2, path);
	ft_putstr(2, "\n");
	// exit(1);
	return (1);
}

int cd1()
{
	ft_putstr(2, "bad arguments\n");
	return (1);
}

int cd2(char *path)
{
	ft_putstr(2, "cd : error ");
	ft_putstr(2, path);
	ft_putstr(2, "\n");
	return (1);
}


void control_c(int sig)
{
	if (g_prog)
	{
		for (int i =0 ; i < g_count; i++)
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
		for (int i =0 ; i < g_count; i++)
		{
			if (g_prog[i].pid != 0 && g_prog[i].pid != -1)
				kill(g_prog[i].pid, sig);
		}
	}
	g_ret = 131;
}


void close_one_fd(int fd)
{
	close(fd);
}

void close_two_fd(int i)
{
	if (g_prog[i].piped)
	{
		close(g_prog[i].pipes[0]);
		close(g_prog[i].pipes[1]);
	}
}

void close_previous_fd(int i)
{
	if (i > 0 && g_prog[i-1].piped)
		close(close(g_prog[i-1].pipes[0]));
}

void close_fd_in(int fd)
{
	if (fd)
		close(fd);
}

int main(int argc, char **argv, char **env)
{

	if (signal(SIGINT, &control_c) == SIG_ERR)
		return (exit_fatal());
	if (signal(SIGQUIT, &control_quit) == SIG_ERR)
		return (exit_fatal());
	g_env = env;
	
	int status;

	if (argc == 1)
		return (0);

	if (!(g_tok = malloc_zero((argc-1) * (sizeof(t_token)))))
		return (exit_fatal());

	for (int i = 1; i < argc; i++)
	{
		char *cur = g_tok[i -1].data = argv[i];
		if (strcmp(cur, ";") == 0)
			g_tok[i-1].type = SEMI;
		else if (strcmp(cur, "|") == 0)
			g_tok[i-1].type = PIPE;
		else
			g_tok[i-1].type = STRING;

		if(g_tok[i - 1].type != STRING)
			g_count++;
	}

	if (!(g_prog = malloc_zero(g_count * (sizeof(t_program)))))
		return (exit_fatal());
	
	int j = 0;
	int start= 0;
	t_token *tok = NULL;

	for (int i = 0; i < g_count; i++)
	{
		start = j;

		while (j < argc -1)
		{
			tok = &(g_tok[j]);
			if (tok->type != STRING)
			{
				g_prog[i].piped = g_tok[j].type == PIPE;
				g_prog[i].semicolon = g_tok[j].type == SEMI;
				break;
			}
			j++;
		}

		g_prog[i].count = j - start;
	//printf("count : %d\n", g_prog[i].count);
		if (g_prog[i].count == 0 && (!tok || tok->type == SEMI))
		{
			j++;
			continue;
		}
		if (!(g_prog[i].argv = malloc_zero((g_prog[i].count + 1) * (sizeof(char *)))))
			return (exit_fatal());
		for (int k = 0 ; k < g_prog[i].count; k++)
		{
			g_prog[i].argv[k] = g_tok[start +k].data;
	//		printf("%s\n", g_prog[i].argv[k]);
		}

		g_prog[i].path = g_prog[i].argv[0];
		j++;
	}

	int fd_in =0;

	for (int i = 0; i < g_count; i++)
	{
		t_program *pr = &(g_prog[i]);
		if (g_prog[i].argv)
		{
			if (strcmp(pr->path, "cd") == 0)
			{
				g_ret = 0;
				if (pr->count <2)
				{
					g_ret = 1;
					cd1();
				}
				
				if (chdir(pr->argv[1]) == -1)
				{
					g_ret = 1;
					cd2(pr->argv[1]);
				}
			}
			else
			{
				if (pr->piped)
				{
					if (pipe(pr->pipes) == -1)
					{
						close_fd_in(fd_in);
						close_previous_fd(i);
						return (exit_fatal());	
					}
				}
				if ((pr->pid = fork()) == -1)
				{
					close_previous_fd(i);
					close_two_fd(i);
					close_fd_in(fd_in);
					return (exit_fatal());
				}
				else if (pr->pid == 0)
				{
					if (fd_in)
					{
						if (dup2(fd_in, 0) == -1)
						{
							close_previous_fd(i);
							close_two_fd(i);
							close_fd_in(fd_in);
							return (exit_fatal());
						}
						close(fd_in); // ICI
					}
					
					if (pr->piped)
					{
						if (dup2(g_prog[i].pipes[1], 1) == -1)
						{
							close_previous_fd(i);
							close_two_fd(i);
							return (exit_fatal());
						}
						if (close(pr->pipes[1]) == -1)
						{
							close_previous_fd(i);
							close_two_fd(i);
							return (exit_fatal());
						}
						if (close(pr->pipes[0]) == -1)
						{
							close_previous_fd(i);
							close_two_fd(i);
							return (exit_fatal());
						}
					}
					if (execve(pr->path, pr->argv, g_env) == -1)
					{
						close(0);
						close(1);
						exit_execve(pr->path);
						exit(1);
					}
				}
				else
				{
					if (pr->pid == -1) // ICI
						;
					else
					{
						waitpid(pr->pid, &status, 0);
						if (WIFEXITED(status))
							g_ret = WEXITSTATUS(status);
					}
					if (pr->semicolon || i == g_count -1)
					{
						
						if (pr->piped && close(pr->pipes[0]) == -1)
						{
							close_previous_fd(i);
							close_two_fd(i);
							return (exit_fatal());
						}
						fd_in = 0;
					}
					else
					{
						fd_in = pr->pipes[0];
					}
					if (pr->piped && close(pr->pipes[1]) == -1)
					{
						close_previous_fd(i);
						close_two_fd(i);
						close_fd_in(fd_in);
						return (exit_fatal());
					}
					if (i> 0 && g_prog[i-1].piped)
					{
						if (close(g_prog[i-1].pipes[0]) == -1)
						{
							close_two_fd(i);
							close_fd_in(fd_in);
							return (exit_fatal());
						}
					}
				}
				
			}
			
		}
		else
		{
			if (i == g_count -1)
				return(terminate(g_ret));
			else
			{
				return (terminate(1));
			}
			
		}
	}
//printf("ret outide : %d\n", g_ret);
	return (terminate(g_ret));
}