#include <stdio.h>
#include <stdlib.h>
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

pthread_t reader[10];
/* use poll way to read info from kernel */
int fslook_read(const char *output)
{
	int ncpus, i, ret;
	void *tret;
	int *fds;
	char filename[1024];
	char buf[4096];

	ncpus = sysconf(_SC_NPROCESSORS_ONLN);
	printf("%d cpus to work\n", ncpus);

	fds = malloc(sizeof(int) * ncpus);
	memset(fds, 0, sizeof(int) * ncpus);

	for (i = 0; i < ncpus; i++) {
		sprintf(filename, "/sys/kernel/debug/fslook/channel/cpu%d", i);
		printf("To read:%s\n", filename);
		fds[i] = open(filename, O_RDONLY);
		if (fds[i] < 0) {
			printf("fail to open %s\n", filename);
		}
	}

	struct pollfd readers[10];
	struct pollfd reader;
	int nready;
	int len;

	memset(&reader,  0, sizeof(struct pollfd));
	memset(readers, 0, sizeof(struct pollfd) * 10);
	for (i = 0; i < ncpus; i++) {
		readers[i].fd = fds[i];
		readers[i].events = POLLIN;
		printf("fd(%d)\n", fds[i]);
	}

	reader.fd = fds[0];
	reader.events = POLLIN;
/*
	for (i = 0; i < ncpus; i++) {
		memset(buf, 0, sizeof(buf));
		len = read(fds[i], buf, sizeof(buf));
		printf(":::::%d\n", i);
		write(1, buf, sizeof(buf));
	}

	return;
	*/

	for (;;) {
poll_more:
		printf("11{--.--PRE--.---\n");
//		nready = poll((struct pollfd *)&readers[0], 4, -1);
		nready = poll((struct pollfd *)&reader, 1, -1);
		printf("{--.--POS--.---\n");
		if (nready < 0) {
			printf("dui----?----\n");
		}

		memset(buf, 0, sizeof(buf));
		len = read(reader.fd, buf, sizeof(buf));
		printf(":::::%d\n", reader.fd);
		write(1, buf, sizeof(buf));

		for (i = 0; i < ncpus; i++) {
			printf("cpu(%d)->revents:%d\n", i, readers[i].revents);

			if (readers[i].revents & POLLERR) {
				printf("error happes in CPU:%d\n", i);
			}

			if (readers[i].revents & POLLOUT) {
				printf("revents happens:%d\n", readers[i].revents);
			}

		}

		sleep(10);
	}
}

#define FSLOOK_PATH "/sys/kernel/debug/fslook/fslookvm"

static int run_fslook()
{
	int fslookvm_fd, fslook_fd;
	int ret;
	void *tret;

	fslookvm_fd = open(FSLOOK_PATH, O_RDONLY);
	if (fslookvm_fd < 0) {
		printf("open %s fail:%d\n", FSLOOK_PATH, fslookvm_fd);
		return fslookvm_fd;
	}

	fslook_fd = ioctl(fslookvm_fd, 0, NULL);
	if (fslook_fd < 0) {
		printf("ioctl ktapvm fail:%d\n", fslook_fd);
		return fslook_fd;
	}

	printf("ioctl begin\n");
	ret = ioctl(fslook_fd, FSLOOK_CMD_IOC_RUN, NULL);
	printf("ioctl end\n");
	switch (ret) {
		case -EPERM:
		case -EACCES:
			fprintf(stderr, "You may not have permission to run fslook\n");
			break;
	}
	printf("----> ioctl down\n");

#if 0
	ret = fslook_create_reader("./out");
#endif
	ret = fslook_read(".out");

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
