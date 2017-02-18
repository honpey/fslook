#if 0
typedef struct fslook_option {
	char *trunk; /* __user */
	int trunk_len;
	int argc;
	char **argv;
	int verbose;
	int trace_pid;
	int workload;
	int trace_cpu;
	int print_timestamp;
	int quiet;
	int dry_run;
} fslook_option_t;


/* Currently we just copy t*/
typedef struct fslook_global_state {
	void *mempool; /* string memory pool */
	void *mp_freepos; /* free position in memory pool */
	int mp_size; /* memory pool size */
//#define __KERNEL__
	arch_spinlock_t mp_lock;
//#endif
	int strmask;
	int strnum; /* String hash mask (size of hash table -1 )*/
//#define __KERNEL__
	arch_spinlock_t str_lock;
//#endif
	fslook_val_t registry;
	fslook_tab_t *gtab;	/* global table contains cfunction and args*/

	fslook_obj_t *allgc; /* list of all collectable objects */
	fslook_upval_t uvhead; /* head of list of all open upvalues */

	struct fslook_state *mainthread; /* main state */
	int state; /* fslookvm status: FSLOOK_RUNNING, FSLOOK_TRACE_END*/
//#define __KERNEL__
	/* reserved global percpu data */
	void __percpu *percpu_state[PERF_NR_CONTEXTS];
	void __percpu *percpu_print_buffer[PERF_NR_CONTEXTS];
	void __percpu *percpu_temp_buffer[PERF_NR_CONTEXTS];

	/* for recursion tracing check */
	int __percpu *recursion_context[PERF_NR_CONTEXTS];
	fslook_option_t	*parm; /* fslook options */
	pid_t trace_pid;
	struct task_struct *trace_task;
	cpumask_var_t cpumask;
	struct ring_buffer *buffer;
	struct dentry *trace_pipe_dentry;
	struct task_struct *task;
	int trace_enabled;
	int wait_user; /* flag to indicat waiting user consume content */
	struct list_head timers;
	struct fslook_stats __percpu *stats; /* memory allocation stats */
	struct list_head events_head; /* probe event list */

	fslook_func_t *trace_end_closure; /* trace_end closure */
//#endif

} fslook_global_state_t;

typedef struct fslook_state {
	fslook_global_state_t *g; /* global state */

} fslook_state_t;

#endif

struct fslook_info {
	struct dentry *dentry;
	struct dentry *dentry_vm;
	struct dentry *dentry_usr;
	struct task_struct *task;
	struct ring_buffer *buffer;
	struct list_head list;

};

#define FSLOOK_CMD_IOC_RUN	('$' + 1)
