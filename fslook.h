#include <linux/dcache.h>
#include <linux/kprobes.h>
#include <linux/vmalloc.h>
#include <linux/anon_inodes.h>
#include <linux/file.h>
#include <linux/time.h>
#include <linux/uaccess.h>

extern int show_all_dentries(void);
extern void show_supers(void);
extern void test_list(void);
struct fslook_info;
int fslook_transport_init(struct fslook_info *, struct dentry *);

/* transport_channel.c */
extern void fslook_printf(const char *fmt, ...);
extern int init_channel(void);
extern void cleanup_channel(void);


#define TRACE_SEQ_LEN(s) ((s)->seq.len)
#define TRACE_SEQ_READPOS(s) ((s)->seq.readpos)
#define TRACE_SEQ_PRINTF(s, ...) ({ trace_seq_printf(s, __VA_ARGS__); !trace_seq_has_overflowed(s); })
