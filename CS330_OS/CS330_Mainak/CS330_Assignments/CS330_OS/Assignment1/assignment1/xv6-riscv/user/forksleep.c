#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include <stddef.h>
#include <stdbool.h>
bool check(char num[])
{
	int i = 0;
	if (num[0] == '-')return false;
		
	for (; num[i] != '\0'; i++)
	{
		if ('0' <= num[i] && '9' >= num[i])
			;
		else
		{
			return false;
		}
	}
	return true;
}
int main(int argc, char **argv)
{
	if (argc != 3 || atoi(argv[1]) <= 0 || (atoi(argv[2]) != 0 && atoi(argv[2]) != 1))
	{
		printf("Arguments not correct || usage: forksleep m n, where m is positive and n can be 0 or 1.\n");
		exit(0);
	}
	if (!check(argv[1]) || !check(argv[2]))
	{
		printf("Arguments not correct || usage: forksleep m n, where m is positive and n can be 0 or 1.\n");
		exit(0);
	}
	int n = atoi(argv[2]);
	int m = atoi(argv[1]);
	int pid=fork();
	if(pid<0){
		printf("error in fork | Aborting...\n");
		exit(0);
	}
	if (pid==0)
	{
		if (n == 0)
		{
			sleep(m);
			printf("%d: Child.\n", getpid());
		}
		if (n == 1)
		{
			printf("%d: Child.\n", getpid());
		}
	}
	else
	{
		if (n == 0)
		{

			printf("%d: Parent.\n", getpid());
		}
		if (n == 1)
		{
			sleep(m);
			printf("%d: Parent.\n", getpid());
		}

		wait(NULL);
	}
	exit(0);
}
