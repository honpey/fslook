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
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "./fslook.h"

int *fds;
int ncpus;
int fslookvm_fd, fslook_fd;
int fslook();
int parse(char *arg, struct command *);
int init_fslook_debugfs_fd();
int fslook_read(const char *output);
struct command commands[MAX_COMMANDS] =  {
	{"ls", 0},
	{"cd", 1},
	{"show", 2},
	{"cls", 3},
};
void sigfunc(int signo)
{
	int i;
	/* close file first, and then rmmod fslookvm */
	close(fslookvm_fd);
	close(fslook_fd);
	for (i=0; i < ncpus; i++) {
		close(fds[i]);
	}
	printf("\n");
	exit(-1);
}
int main()
{
	char shell_prompt[100];
	char *input, *arg;
	struct command cur_cmd;
	int pos, ret;
	int debug_count = 0;
	struct stat filestat;
	int fd;
	char buffer[1024];

	pid_t tid;
	tid = syscall(__NR_gettid);
	printf("tid:%d\n", tid);

	/* load .ko by init_module */
	/* we should firt read the fslook.ko and know length of the file */
	fd = open("../fslook.ko", O_RDONLY, S_IRUSR);
	if (fd < 0) {
		printf("fail to open ../fslook.ko:%d\n", fd);
		return -1;
	}
	ret = fstat(fd, &filestat);
	if (ret < 0) {
		printf("fstat fail:%d\n", ret);
		return -1;
	}
//	ret = finit_module(fd, NULL, 0);
//	printf("__NR_fiit_module:%d\n", __NR_finit_module);
//	ret = syscall(__NR_finit_module, fd, NULL, 3);
//	ret = syscall(350, fd, NULL, 0);
	system("rmmod fslook.ko");
	printf("rmmod fslook.ko OK\n");
	system("insmod ../fslook.ko 2>/dev/null");
	printf("insmod OK\n");
	if (ret < 0) {
		printf("init module:%d\n", ret);
		return -1;
	}

	system("insmod ../fslook.ko");
	signal(SIGINT, sigfunc);
	snprintf(shell_prompt, sizeof(shell_prompt), "(fslook): ");
	init_fslook_debugfs_fd();

	for (;;) {
		debug_count++;
		snprintf(shell_prompt, sizeof(shell_prompt), 
						"(fslook):%d* ", debug_count);
		input = readline(shell_prompt);
		printf("......\n");
		pos = parse(input, &cur_cmd);
#if 1
		printf("command:(%s,%d,%s)\n", 
				cur_cmd.name, cur_cmd.no, cur_cmd.arg);
#endif
		switch (pos) {
			case 0:
				ret = ioctl(fslook_fd, FSLOOK_CMD_IOC_LS, NULL);
				switch (ret) {
					case -EPERM:
					case -EACCES:
						fprintf(stderr, "You may not have permission to run fslook\n");
						break;
					}
				break;
			case 1:
				break;
			case 2:
				ret = ioctl(fslook_fd, FSLOOK_CMD_IOC_SHOW,
								&cur_cmd);
				if (ret < 0) {
					fprintf(stderr, "You may not have permission to run fslook\n");
				}
				break;
			case 3:
				break;
			default:
				break;
		}
		fslook_read(NULL);
		pos = 0;
	}

	return 0;
}

int check_command(char *command);
/*
 * To parse the command in fslook shell
 * How lua parse such info?
 *
 */
int parse(char *command, struct command * cur_cmd)
{
	int whichtok = 0, ret = 0;
	int posInCl = 0;	
	char *tok, *delim = " ";
	char *arg;

	/* char *strtok(char *str, const char *delim) */
	tok = strtok(command, delim);
	while (tok != NULL) {
		if (!(whichtok & 1)) {
			posInCl = check_command(tok);
			if (posInCl < 0) {
				return posInCl;
			}
			cur_cmd->no = posInCl;
			cur_cmd->name = tok;
		} else {
			cur_cmd->arg = tok;
		}
		whichtok ++;
		tok = strtok(NULL, delim);
	}
	return cur_cmd->no;
}

int check_command(char *command)
{
	int i;
	for (i = 0; i < MAX_COMMANDS; i++) {
		if (strcmp(command, commands[i].name) == 0) {
			printf("Found \"%s\"\n", command);
			return i;
		}
	}
	return -1;
	
}
pthread_t reader[10];
/* use poll way to read info from kernel */
int fslook_read(const char *output)
{
	int i, j, ret;
	void *tret;
	char filename[1024];
	char buf[4096];
	bool hasData = true;

	ncpus = sysconf(_SC_NPROCESSORS_ONLN);
//	printf("%d cpus to work\n", ncpus);

	fds = malloc(sizeof(int) * ncpus);
	memset(fds, 0, sizeof(int) * ncpus);

	for (i = 0; i < ncpus; i++) {
		sprintf(filename, "/sys/kernel/debug/fslook/channel/cpu%d", i);
	//	printf("To read:%s\n", filename);
		fds[i] = open(filename, O_RDONLY);
		if (fds[i] < 0) {
			printf("fail to open %s\n", filename);
			exit(-1);
		}
	}

	/* we should NOT use poll will relay*/
read_more:
//	printf("hello world\n");
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

int init_fslook_debugfs_fd()
{
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

	
}
