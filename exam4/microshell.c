#include "microshell.h"
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/errno.h>
#include <stdio.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

char **g_envp = NULL;

size_t g_token_count = 0;
t_token *g_tokens = NULL;

size_t g_program_count = 1;
t_program *g_programs = NULL;

int terminate(int ret)
{
	if (g_programs)
	{
		for (size_t i = 0; i < g_program_count; ++i)
			free(g_programs[i].args);
		free(g_programs);
		g_programs = NULL;
	}
	if (g_tokens)
	{
		free(g_tokens);
		g_tokens = NULL;
	}

	while (1)
	{
	}
	exit(ret);

	return (0);
}

void exit_fatal(void)
{
	write(STDERR, "error: fatal\n", ft_strlen("error: fatal\n"));
	terminate(1);
	exit(EXIT_FAILURE);
}

void exit_execve(char *str)
{
	write(STDERR, "error: cannot execute ", ft_strlen("error: cannot execute "));
	write(STDERR, str, ft_strlen(str));
	write(STDERR, "\n", 1);
	// terminate(1);
	// exit(EXIT_FAILURE);
}

int exit_cd_1()
{
	write(STDERR, "error: cd: bad arguments\n", ft_strlen("error: cd: bad arguments\n"));
	terminate(1);
	return (EXIT_FAILURE);
}

int exit_cd_2(char *str)
{
	write(STDERR, "error: cd: cannot change directory to ", ft_strlen("error: cd: cannot change directory to "));
	write(STDERR, str, ft_strlen(str));
	write(STDERR, "\n", 1);
	terminate(1);
	return (EXIT_FAILURE);
}

void control_c(int sig)
{

	(void)sig;
	if (g_programs)
	{
		for (size_t i = 0; i < g_program_count; ++i)
		{
			if (g_programs[i].pid != 0)
			{
				kill(g_programs[i].pid, sig);
				printf("KILL\n");
			}
		}
	}

}

void control_quit(int sig)
{

	(void)sig;
	if (g_programs)
	{
		for (size_t i = 0; i < g_program_count; ++i)
		{
			if (g_programs[i].pid != 0)
			{
				kill(g_programs[i].pid, sig);
			}
		}
	}
	terminate(131);
}

int main(int argc, char **argv, char **envp)
{
	g_envp = envp;
	int ret = 0;
	int status;
	if (signal(SIGINT, &control_c) == SIG_ERR)
	{
		exit_fatal();
		return (EXIT_FAILURE);
	}
	// if (signal(SIGQUIT, &control_quit) == SIG_ERR)
	// {
	// 	exit_fatal();
	// 	return (EXIT_FAILURE);
	// }

	if (argc == 1)
		return (0);

	g_tokens = malloc_zeros((g_token_count = argc - 1) * sizeof(t_token));

	for (int i = 1; i < argc; ++i)
	{
		t_token *tok = &(g_tokens[i - 1]);
		char *curr = tok->data = argv[i];

		tok->end = i == argc - 1;

		if (strcmp(";", curr) == 0)
			tok->type = TT_SEMICOLON;
		else if (strcmp("|", curr) == 0)
			tok->type = TT_PIPE;
		else
			tok->type = TT_STRING;

		if (tok->type != TT_STRING)
			g_program_count++;
	}

	g_programs = malloc_zeros(g_program_count * sizeof(t_program));

	{
		size_t j = 0;

		for (size_t i = 0; i < g_program_count; ++i)
		{
			t_program *pr = &(g_programs[i]);

			size_t start = j;

			t_token *tok = NULL;
			while (j < g_token_count)
			{
				tok = &(g_tokens[j]);

				if (tok->type != TT_STRING)
				{
					pr->piped = tok->type == TT_PIPE;
					pr->semicoloned = tok->type == TT_SEMICOLON;
					break;
				}
				j++;
			}

			pr->count = j - start;

			if (pr->count == 0 && (!tok || tok->type == TT_SEMICOLON))
			{
				j++;
				continue;
			}

			pr->args = malloc_zeros((pr->count + 1) * sizeof(char *));

			for (size_t k = 0; k < pr->count; ++k)
			{
				pr->args[k] = g_tokens[start + k].data;
			}

			pr->path = pr->args[0];
			j++;
		}
	}

	int fd_in = 0;

	/* Execution. */
	for (size_t i = 0; i < g_program_count; ++i)
	{
		t_program *pr = &(g_programs[i]);
		if (pr->args)
		{

			if (strcmp("cd", pr->path) == 0)
			{
				if (pr->count < 2)
					exit_cd_1();

				ft_putstr(1, pr->args[1]);
				if (chdir(pr->args[1]))
					exit_cd_2(pr->args[1]);
			}

			else
			{
				

				if (pr->piped)
				{
					if(pipe(pr->pipes))
						exit_fatal();
				}
				if ((pr->pid = fork()) == -1)
					exit_fatal();
				else if (pr->pid == 0)
				{
					if (fd_in)
				{
					if (dup2(fd_in, FD_IN) < 0)
					{
						printf("ici\n");
					
						exit_fatal();
					}
					close(fd_in);
				}
					if (pr->piped)
					{
						if (dup2(pr->pipes[1], FD_OUT) < 0)
							exit_fatal();
						close(pr->pipes[1]);
						close(pr->pipes[0]);
					}

					if (execve(pr->path, pr->args, g_envp) == -1)
					{
						exit_execve(pr->path);	
						exit(1);
					}
				
					exit(EXIT_SUCCESS);
				}
				else
				{
				
					
					if (pr->semicoloned || i == g_program_count - 1)
					{
						if (pr->pid == -1)
							;
						else
						{	
							waitpid(pr->pid, &status, 0);
							if (WIFEXITED(status))
								ret = WEXITSTATUS(status);
						}
					 	close(pr->pipes[0]);
						fd_in = 0;
					}
					else
					{
						fd_in = pr->pipes[0];
					}
					if (i > 0 && g_programs[i - 1].piped)
					{
						close(g_programs[i - 1].pipes[0]);
					}
					close(pr->pipes[1]);
				}
			}
		}
		else
		{
			if (i == 0)
				return (terminate(1));
			else
				return (terminate(0));
		}
	}
	
	return (terminate(ret));
}