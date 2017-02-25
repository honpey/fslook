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
typedef int bool;
#define true 1
#define false 0

pthread_t reader[10];
/* use poll way to read info from kernel */
int fslook_read(const char *output)
{
	int ncpus, i, j, ret;
	void *tret;
	int *fds;
	char filename[1024];
	char buf[4096];
	bool hasData = true;

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

	/* we should NOT use poll will relay*/
read_more:
	if (!hasData)
		goto done;		
	hasData = false;
	for (i = 0; i < ncpus; i++) {
		memset(buf, 0, sizeof(buf));
		ret = read(fds[i], buf, 4096);
		if (ret != 0) {
			hasData = true;
			printf("From CPU:%d, %d bytes\n", i, ret);
			for (j = 0; j < ret; j++) {
				printf("%c", buf[j]);
			}
		}
	}
	goto read_more;
done:
	return 0;
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

	ret = ioctl(fslook_fd, FSLOOK_CMD_IOC_RUN, NULL);
	switch (ret) {
		case -EPERM:
		case -EACCES:
			fprintf(stderr, "You may not have permission to run fslook\n");
			break;
	}

#if 0
	ret = fslook_create_reader("./out");
#endif
	char name[1024];
	int pos = 0;
	memset(name, 0, 1024);
more:
	printf("fslook:%s\n", name);
	scanf("%c", &name[pos++]);
	goto more;
	
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
