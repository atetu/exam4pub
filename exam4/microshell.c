#include "microshell.h"
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/errno.h>
#include <stdio.h>

#define STDIN		0
#define STDOUT		1
#define STDERR		2

char **g_envp = NULL;

size_t g_token_count = 0;
t_token *g_tokens = NULL;

size_t g_program_count = 1;
t_program *g_programs = NULL;

int terminate(char *msg, int ret)
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

	exit(ret);

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

void			control_c(int sig)
{


	(void)sig;
	for (size_t i = 0; i < g_program_count; ++i)
	{
		if(g_programs[i].pid != 0)
		{
			kill(g_programs[i].pid, sig);
			printf("KILL\n");
		}
	}
	// write(1, "\n", 1);
	terminate(NULL, 130);
	
}

void			control_quit(int sig)
{

	(void)sig;
	int is_kill = 0;
	for (size_t i = 0; i < g_program_count; ++i)
	{
		if(g_programs[i].pid != 0)
		{
			kill(g_programs[i].pid, sig);
			is_kill = 1;
		}
	}

	if (is_kill)
	{
		write(1, "\b\b  \b\b", 6);
		terminate(NULL, 127);
	}
	else
	{
		write(1, "Quit\n", 5);
		terminate(NULL, 131);
	}
	
}
int main(int argc, char **argv, char **envp)
{
	g_envp = envp;

	if (signal(SIGINT, &control_c) == SIG_ERR)
	{
		exit_fatal();
		return (EXIT_FAILURE);
	}
	if (signal(SIGQUIT, &control_quit) == SIG_ERR)
	{
		exit_fatal();
		return (EXIT_FAILURE);
	}

	if (argc == 1)
		return (0);

	g_tokens = malloc_zeros((g_token_count = argc - 1) * sizeof(t_token));

//	printf("g_token_count: %zu\n", g_token_count);
	/* Arguments to tokens. */
	for (int i = 1; i < argc; ++i)
	{
		t_token *tok = &(g_tokens[i - 1]);
		char *curr = tok->data = argv[i];
//		printf("i: %d / data : %s\n", i, tok->data);
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
		//	printf("j before : %zu \n", j);
			size_t start = j;
		//	printf("j after : %zu \n", j);
			t_token *tok = NULL;
			while (j < g_token_count)
			{
				tok = &(g_tokens[j]);
		//		printf("j loop : %zu \n", j);
				if (tok->type != TT_STRING)
				{
					pr->piped = tok->type == TT_PIPE;
					pr->semicoloned = tok->type == TT_SEMICOLON;
					break;
				}
				j++;
			}

		//	printf("j : %zu \n", j);
		///	printf("start : %zu \n", start);
			pr->count = j - start;
		//	printf("count : %zu\n", pr->count);
			if (pr->count == 0 && (!tok || tok->type == TT_SEMICOLON))
			{
				j++;
		//		printf("j in the loop : %zu\n", j);
				continue;
			}
		//	printf("count : %zu\n", pr->count);
			pr->args = malloc_zeros((pr->count + 1) * sizeof(char *));

			for (size_t k = 0; k < pr->count; ++k)
			{
				pr->args[k] = g_tokens[start + k].data;
		//		printf("arg: %s\n", pr->args[k]);
			}

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
		if(pr->args)
		{
			int p[2];
			pipe(p);

			pid_t pid = fork();

			if (pid == 0)
			{
				dup2(fd_in, 0);

				if (pr->piped)
					dup2(p[1], 1);

				close(p[0]);
				execve(pr->path, pr->args, envp);
			}
			else
			{
				if (pr->semicoloned || i == g_program_count - 1)
				{
					waitpid(pid, NULL, 0);
					if (fd_in)
						close(fd_in);
					fd_in = 0;
					close(p[0]);
				}
				else
				{
					if (fd_in)
						close(fd_in);
					fd_in = p[0];	
				}

				close(p[1]);
			}
			

			/*if (strcmp("cd", pr->path) == 0)
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
			{*/
				
			/*
				if(pipe(pr->pipes))
					exit_fatal();

				if ((pr->pid = fork()) == -1)
					exit_fatal();
				else if (pr->pid == 0)
				{
					if (dup2(fd_in, FD_IN) < 0)
						exit_fatal();
			//		printf("fd in after close : %d\n", fd_in);
					if (pr->piped)
					{
						if (dup2(pr->pipes[1], FD_OUT) < 0)
							exit_fatal();
					}
					close(pr->pipes[0]);
					printf("count: %zu\n", pr->count);
					if (execve(pr->path, pr->args, g_envp) == -1) 
						exit_execve(pr->path);
					exit(EXIT_SUCCESS);
				}
				else
				{
					if (pr->semicoloned || i == g_program_count - 1)
					{
						//				ft_putstr(FD_ERR, strcat(strcat(strdup("waiting: "), pr->path), "\n"));
						dprintf(2, "----------------===-=-=-=- waiting %d: %s %zu\n", pr->pid, pr->path, i);
						waitpid(pr->pid, NULL, 0);
						dprintf(2, "----------------===-=-=-=- finished waiting %d\n", pr->pid);
						close(pr->pipes[0]);
						if (fd_in)
							close(fd_in);
						fd_in = 0;
					}
					else
					{
						fd_in = pr->pipes[0];
						// printf("fd in : %d\n", fd_in);
					}
					//if (i > 0 && g_programs[i-1].piped)
					//	close(g_programs[i-1].pipes[0]);
					close(pr->pipes[1]);
				}*/
			//}
		}
		else
		{
			break;
		}
		
	}

	if (fd_in != 0)
		close(fd_in);

	/* Just to be sure. */
	//for (size_t i = 0; i < g_program_count; ++i)
	//	waitpid(g_programs[i].pid, NULL, 0);
	// while(1)
	// {}
	return (terminate(NULL, false));
}