#include "microshell.h"
#include <unistd.h>
#include <stdio.h>

#define STDIN		0
#define STDOUT		1
#define STDERR		2

char **g_envp = NULL;

size_t g_token_count = 0;
t_token *g_tokens = NULL;

size_t g_program_count = 1;
t_program *g_programs = NULL;

int terminate(char *msg, bool asErr)
{
	// if (msg)
	// {
	// 	ft_putstr(FD_ERR, msg);
	// 	ft_putstr(FD_ERR, "\n");
	// }

	for (size_t i = 0; i < g_program_count; ++i)
		free(g_programs[i].args);

	free(msg);
	free(g_tokens);
	free(g_programs);

	exit(asErr);

	return (0);
}


void exit_fatal(void)
{
	write(STDERR, "error: fatal\n", ft_strlen("error: fatal\n"));
	terminate(NULL, 1);
	exit(EXIT_FAILURE);
}

void exit_execve(char *str)
{
	write(STDERR, "error: cannot execute ", ft_strlen("error: cannot execute "));
	write(STDERR, str, ft_strlen(str));
	write(STDERR, "\n", 1);
	exit(EXIT_FAILURE);
}

int exit_cd_1()
{
	write(STDERR, "error: cd: bad arguments\n", ft_strlen("error: cd: bad arguments\n"));
	//return (EXIT_FAILURE);
	terminate(NULL, 1);
	return(EXIT_FAILURE);
}

int exit_cd_2(char *str)
{
	write(STDERR, "error: cd: cannot change directory to ", ft_strlen("error: cd: cannot change directory to "));
	write(STDERR, str, ft_strlen(str));
	write(STDERR, "\n", 1);
	return (EXIT_FAILURE);
}


int terminate_errno(char *origin)
{
	ft_putstr(FD_ERR, "microshell: ");
	if (origin)
	{
		ft_putstr(FD_ERR, origin);
		ft_putstr(FD_ERR, ": ");
	}
	ft_putstr(FD_ERR, strerror(errno));
	ft_putstr(FD_ERR, "\n");

	return (terminate(NULL, true));
}

int main(int argc, char **argv, char **envp)
{
	g_envp = envp;

	if (argc == 1)
		return (0);

	g_tokens = malloc_zeros((g_token_count = argc - 1) * sizeof(t_token));

	printf("g_token_count: %zu\n", g_token_count);
	/* Arguments to tokens. */
	for (int i = 1; i < argc; ++i)
	{
		t_token *tok = &(g_tokens[i - 1]);
		char *curr = tok->data = strdup(argv[i]);
		printf("i: %d / data : %s\n", i, tok->data);
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

	/* Tokens to 'program' struct. */
	{
		size_t j = 0;

		for (size_t i = 0; i < g_program_count; ++i)
		{
			t_program *pr = &(g_programs[i]);
			printf("j before : %zu \n", j);
			size_t start = j;
			printf("j after : %zu \n", j);
			t_token *tok = NULL;
			while (j < g_token_count)
			{
				tok = &(g_tokens[j]);
				printf("j loop : %zu \n", j);
				if (tok->type != TT_STRING)
				{
					pr->piped = tok->type == TT_PIPE;
					pr->semicoloned = tok->type == TT_SEMICOLON;
					break;
				}
				j++;
			}

			printf("j : %zu \n", j);
			printf("start : %zu - %s\n", start, g_tokens[start].data);
			pr->count = j - start;
			if (pr->count == 0 && tok->type == TT_SEMICOLON)
				continue;
			printf("count : %zu\n", pr->count);
			pr->args = malloc_zeros((pr->count+1) * sizeof(char *));

			for (size_t k = 0; k < pr->count; ++k)
				pr->args[k] = g_tokens[start + k].data;

			pr->path = pr->args[0];
			 j++;
		}
	}

	//	/* Dump. */
	//	for (size_t i = 0; i < g_program_count; ++i)
	//	{
	//		t_program *pr = &(g_programs[i]);
	//
	//		printf("\n\npath = %s\n", pr->path);
	//		printf("piped = %s\n", pr->piped ? "true" : "false");
	//		printf("semicoloned = %s\n", pr->semicoloned ? "true" : "false");
	//		for (size_t j = 0; pr->args[j]; ++j)
	//			printf("arg[%zu] = %s\n", j, pr->args[j]);
	//	}

	int fd_in = 0;

	/* Execution. */
	for (size_t i = 0; i < g_program_count; ++i)
	{
		t_program *pr = &(g_programs[i]);
		if(pr && pr->args)
		{
			if (strcmp("cd", pr->path) == 0)
			{
				if (pr->count < 2)
					exit_cd_1();
				
				ft_putstr(1, pr->args[1]);
				if (chdir(pr->args[1]))
					exit_cd_2(pr->args[1]);
				// else
				// {
				// 	char buf[500];
				// 	getcwd(buf, 500);
				// 	printf("buf:%s\n", buf);
				// }
			}
			
			else
			{
				
			
				if(pipe(pr->pipes))
					exit_fatal();

				if ((pr->pid = fork()) == -1)
					exit_fatal();
				else if (pr->pid == 0)
				{
					if (dup2(fd_in, FD_IN) < 0)
						exit_fatal();
					printf("fd in after close : %d\n", fd_in);
					if (pr->piped)
					{
						if (dup2(pr->pipes[1], FD_OUT) < 0)
							exit_fatal();
					}
				
					if (execve(pr->path, pr->args, g_envp) == -1) 
						exit_execve(pr->path);
					exit(EXIT_SUCCESS);
				}
				else
				{
					if (pr->semicoloned || i == g_program_count - 1)
					{
						//				ft_putstr(FD_ERR, strcat(strcat(strdup("waiting: "), pr->path), "\n"));
						waitpid(pr->pid, NULL, 0);
						close(pr->pipes[0]);
						fd_in = 0;
					}
					else
					{
						fd_in = pr->pipes[0];
						// printf("fd in : %d\n", fd_in);
					}
					if (i > 0 && g_programs[i-1].piped)
						close(g_programs[i-1].pipes[0]);
					close(pr->pipes[1]);
				}
			}
		}
		else
		{
			break;
		}
		
	}

	/* Just to be sure. */
	for (size_t i = 0; i < g_program_count; ++i)
		waitpid(g_programs[i].pid, NULL, 0);
	// while(1)
	// {}
	return (terminate(NULL, false));
}