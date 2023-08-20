#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>

#define SIZE 16 

int main(int argc, char* argv[])
{
	int fd_1 = 0;
	int fd_2 = 0;
	char buf[SIZE];
	int val = 0;
	int ret = 0;
        struct rusage usage;

	if(argc != 2)
	{
		printf("Format: $./executable <sysfs option>\n");
		return -1;
	}


	//write to sysfs file
	fd_1 = open("/sys/kernel/cs614_sysfs/cs614_value", O_WRONLY);
	assert(fd_1 >= 0);

	ret = write(fd_1, argv[1], strlen(argv[1]));
	assert(ret == strlen(argv[1]));


	sleep(30);

	//read from char device
	fd_2 = open("/dev/cs614_device", O_RDONLY);
	assert(fd_2 >= 0);

	memset(buf, '\0', SIZE);

	ret = read(fd_2, &buf, SIZE);
        assert(ret > 0);
        close(fd_1);
        close(fd_2);

        getrusage(RUSAGE_SELF, &usage);

        if((atoi(argv[1]) == 0) && (atoi(buf) == getpid()))
        {
                printf("ret: %d, val read: %d\n", ret, atoi(buf));
                printf("Testcase passed\n");
                return 0;
        }
        else if((atoi(argv[1]) == 1) && (atoi(buf) == 120+getpriority(PRIO_PROCESS, 0)))
        {
                printf("ret: %d, val read: %d\n", ret, atoi(buf));
                printf("Testcase passed\n");
                return 0;
        }
        else if((atoi(argv[1]) == 2) && (!strcmp(argv[0]+2, buf)))
        {
                printf("ret: %d, val read: %s\n", ret, buf);
                printf("Testcase passed\n");
                return 0;
        }
        else if((atoi(argv[1]) == 3) && (atoi(buf) == getppid()))
        {
                printf("ret: %d, val read: %d\n", ret, atoi(buf));
                printf("Testcase passed\n");
                return 0;
        }
        else if((atoi(argv[1]) == 4) && (atoi(buf) == (usage.ru_nvcsw)))
        {
                printf("ret: %d, val read: %lu, usage.ru_nvcsw: %lu\n", ret, atol(buf), usage.ru_nvcsw);
                printf("Testcase passed\n");
                return 0;
        }
        else
        {
                printf("ret: %d, val read: %d\n", ret, atoi(buf));
        }
        printf("Testcase failed\n");

        return 0;

}
