#define FSLOOK_CMD_IOC_RUN ('$' + 1)
typedef int bool;
#define true 1
#define false 0

#define MAX_COMMAND_NAME_LEN 10

struct command {
	char *name;
	int no;
	char *arg;
	
};

#define MAX_COMMANDS 4
