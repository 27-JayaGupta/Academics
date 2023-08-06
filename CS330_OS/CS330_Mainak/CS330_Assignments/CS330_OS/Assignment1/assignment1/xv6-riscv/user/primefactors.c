#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include <stddef.h>
#include <stdbool.h>
bool check(char num[])
{
	int i = 0;
	if (num[0] == '-')
		i = 1;
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
	if (argc != 2)	
	{
		printf("There should be one arguments\n");
		exit(0);
	}

	if (!check(argv[1]))
	{
		printf("Argument should be integer in [2, 100]\n");
		exit(0);
	}
	int n = atoi(argv[1]);

	if (n <2 ||n>100)
	{
		printf("First argument should be in range [2, 100]\n");
        exit(0);
	}
    int primes[]={2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97};
    int i=0;
	while (n!=1)
	{
		int arr[2];
		if (pipe(arr) < 0)
		{
			printf("error in pipe\n");
			exit(0);
		};
		int t = fork();
		if (t < 0)
		{
			printf("error in fork\n");
			exit(0);
		}
		// child
		if (t == 0)
		{
            i+=1;
			read(arr[0], &n, sizeof(int));
			close(arr[0]);
			close(arr[1]);
		}
		else
		{ // parent
			int pid = getpid();
            int l=0;
			while(n%primes[i]==0){n/=primes[i];l++; printf("%d, ",primes[i]);}
            if(l!=0)
			{printf("[%d]\n", pid);}
			close(arr[0]);
			write(arr[1], &n, sizeof(int));
			close(arr[1]);
			wait(NULL);
			exit(0);
		}
	}

	exit(0);
}
