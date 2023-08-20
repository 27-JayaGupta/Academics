#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#define SIZE 16
#define N 5

int thread_running[N];

static void *run(void *arg) {
	size_t job = *(size_t*)arg;
	thread_running[job] = 1;
	printf("Job %zu. Sleeping for 600 secs.\n", job);
	sleep(600);
	return NULL;
}

int main(int argc, char *argv[]) {
	size_t jobs[N];
	pthread_t threads[N];
	int fd_1 = 0;
	int fd_2 = 0;
	char buf[SIZE];
	int val = 0;
	int ret = 0;
	char *b = 0;

	if(argc != 2)
        {
                printf("Format: $./executable <sysfs option>\n");
                return -1;
        }

	//initially no thread is running
	for(int i = 0; i < N; i++)
	{
		thread_running[i] = 0;
	}

	printf("Creating threads\n");
	for (size_t i=0; i<N; ++i) {
		jobs[i] = i;
		pthread_create(threads+i, NULL, run, jobs+i);
	}


	//wait until all threads are running
	for(int i = 0; i < N; i++)
	{
		while(!thread_running[i])
		{
			sleep(1);
		}
		printf("Thread %d is running\n", i);
	}

        //write to sysfs file
        fd_1 = open("/sys/kernel/cs614_sysfs/cs614_value", O_WRONLY);
        assert(fd_1 >= 0);

        ret = write(fd_1, argv[1], strlen(argv[1]));
	assert(ret == strlen(argv[1]));


        //read from char device
        fd_2 = open("/dev/cs614_device", O_RDONLY);
        assert(fd_2 >= 0);

        if(atoi(argv[1]) == 5)
        {
                ret = read(fd_2, buf, SIZE);
                printf("ret: %d, buf read: %d\n", ret, atoi(buf));
		assert(ret > 0);

		if(atoi(buf) == N+1)
		{
			printf("Testcase passed\n");
		}
		else
		{
			printf("Testcase failed\n");
		}
        }
        else
        {
                printf("Testing correctness of NUM_THREADS operation only\n");
		return 0;
        }

	//printf("Waiting for threads to join\n");
	for (size_t i=0; i<N; ++i) {
		pthread_join(threads[i], (void**)&b);
	}

        close(fd_1);
        close(fd_2);

	return EXIT_SUCCESS;
}

