#include <linux/dcache.h>
#include <linux/kprobes.h>
#include <linux/vmalloc.h>
#include <linux/anon_inodes.h>
#include <linux/file.h>
#include <linux/time.h>
#include <linux/uaccess.h>

struct channel_manager
{
	struct dentry *dir;
	struct rchan *chan;
	size_t subbuf_size;
	size_t n_subbufs;
};

extern int show_all_dentries(void);
struct fslook_info;
extern void show_supers(struct fslook_info *);
extern void test_list(void);
struct fslook_info;
int fslook_transport_init(struct fslook_info *, struct dentry *);

/* transport_channel.c */
extern void fslook_printf(struct fslook_info *fi, const char *fmt, ...);
extern int init_channel(struct fslook_info *);
extern void cleanup_channel(void);


#define TRACE_SEQ_LEN(s) ((s)->seq.len)
#define TRACE_SEQ_READPOS(s) ((s)->seq.readpos)
#define TRACE_SEQ_PRINTF(s, ...) ({ trace_seq_printf(s, __VA_ARGS__); !trace_seq_has_overflowed(s); })
