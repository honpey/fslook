#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/errno.h>

#include "./fslook.h"

#define MAX_BUFLEN 131072
#define PATH_MAX 128

#define handle_error(str) do { perror(str); exit(-1); } while(0)

void sigfunc(int signo)
{
	/* should NOT reach here */
}
static void block_sigint()
{
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);

	pthread_sigmask(SIG_BLOCK, &mask, NULL);
}
static void *reader_thread(void *data)
{
	char buf[MAX_BUFLEN];
	char filename[PATH_MAX];

	const char *output = NULL;
	int failed = 0, fd, out_fd, len;
	int cpu = (int)data;

	block_sigint();

	if (output) {
		out_fd = open(output, O_CREAT | O_WRONLY | O_TRUNC,
			S_IRUSR | S_IWUSR);
		if (out_fd < 0) {
			fprintf(stderr, "Cannot open output file %s\n", output);
			return NULL;
		}
	} else
		out_fd = 1;

	sleep(2);
//	sprintf(filename, "/sys/kernel/debug/fslook/trace_pipe_%d", getpid());
	sprintf(filename, "/sys/kernel/debug/fslookWW/cpu%d", cpu);

open_again:

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		usleep(10000);
		if (failed++ == 10) {
			fprintf(stderr, "Cannot open file %s\n", filename);
			return NULL;
		}
		goto open_again;
	}

	while ((len = read(fd, buf, sizeof(buf))) > 0)
		write(out_fd, buf, len);

	close(fd);
	close(out_fd);

	return NULL;
}

int fslook_create_reader(const char *output)
{
	pthread_t reader[10];

	signal(SIGINT, sigfunc);

	int ncpus, i;

	ncpus = sysconf(_SC_NPROCESSORS_ONLN);
	printf("ncpus:%d\n", ncpus);

	for (i = 0; i < ncpus; i++) {
		if (pthread_create(&reader[i], NULL, reader_thread,
			(void *)i) < 0) {
			printf("i:%d\n", i);
			printf("pthread create reader_thread fail\n");
		}
	}

	return 0;

}

#define FSLOOK_PATH "/sys/kernel/debug/fslookHH/fslookvm"

static int run_fslook()
{
	int fslookvm_fd, fslook_fd;
	int ret;

	fslookvm_fd = open(FSLOOK_PATH, O_RDONLY);
	if (fslookvm_fd < 0) {
		printf("open %s fail:%d\n", FSLOOK_PATH, fslookvm_fd);
		return fslookvm_fd;
	}

	printf("fslookmv_fd:%d\n", fslookvm_fd);

	fslook_fd = ioctl(fslookvm_fd, 0, NULL);
	if (fslook_fd < 0) {
		printf("ioctl ktapvm fail:%d\n", fslook_fd);
		return fslook_fd;
	}

	printf("fslook_fd:%d\n", fslook_fd);

	ret = fslook_create_reader("./out");

	printf("-----------\n");
	ret = ioctl(fslook_fd, FSLOOK_CMD_IOC_RUN, NULL);
	switch (ret) {
		case -EPERM:
		case -EACCES:
			fprintf(stderr, "You may not have permission to run fslook\n");
			break;
	}
	printf("----------111\n");

	sleep(10);
	close(fslook_fd);
	close(fslookvm_fd);

	return ret;

}

/* Get kernel info to userspace */

int main()
{
	printf("\nWelcome the fslook world .. \n\n");

	run_fslook();
	return 0;
}
