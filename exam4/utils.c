#include "microshell.h"
#include<stdio.h>
void*
malloc_zeros(size_t size)
{
	printf("%zu\n", size);
	char *mem = malloc(size);

	if (mem)
		for (size_t i = 0; i < size; i++)
			mem[i] = 0;

	return (mem);
}

size_t
ft_strlen(char *str)
{
	char *start = str;

	while (*str)
		str++;

	return (str - start);
}

void
ft_putstr(int fd, char *str)
{
	write(fd, str, ft_strlen(str));
}