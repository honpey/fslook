#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/relay.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include "fslook.h"
#include "include/fslook_types.h"

static struct dentry *dir;
static struct rchan *chan;

static size_t subbuf_size = 262144; /* 256K each buffer */
static size_t n_subbufs = 1;
static int count = 0;

static int subbuf_start_handler(struct rchan_buf *buf,
		void *subbuf,
		void *prev_subbuf,
		size_t prev_padding)
{
	if (relay_buf_full(buf)) {
		printk("cpu:%d %d\n", smp_processor_id(), count++);
		return 0;
	}
	return 1;
}

static struct dentry *create_buf_file_handler(const char *filename,
			struct dentry *parent,
			unsigned short mode,
			struct rchan_buf *buf,
			int *is_global)
{
	struct dentry *buf_file;
	buf_file = debugfs_create_file(filename, mode, parent, buf,
					&relay_file_operations);
	return buf_file;
}

static int remove_buf_file_handler(struct dentry *dentry)
{
	printk("remove dentry\n");
	debugfs_remove(dentry);
	return 0;
}

static struct rchan_callbacks relay_callbacks =
{
	.subbuf_start = subbuf_start_handler,
	.create_buf_file = create_buf_file_handler,
	.remove_buf_file = remove_buf_file_handler,
};

static struct rchan *create_channel(unsigned size, unsigned n)
{
	struct rchan *channel;
	channel = relay_open("cpu", dir, size, n, &relay_callbacks, NULL);
	if (!channel) {
		printk("relay app channel creation failed\n");
		return NULL;
	}
	return channel;
}

/* chan should be reserved in fslook_info */
void fslook_printf(const char *fmt, ...)
{
	char *buff;
	va_list args;
	int len;

	buff = kmalloc(1024, GFP_KERNEL);
	if (!buff) {
		printk("fail to alloc buff\n");
		return;
	}

	va_start(args, fmt);
	len = vscnprintf(buff, 1024, fmt, args);
	va_end(args);

	buff[len] = 0;
	relay_write(chan, buff, len+1);

	return;
}

int init_channel(void)
{
//	struct dentry *buf_file;
	dir = debugfs_create_dir("fslookWW", NULL);
	if (!dir) {
		printk("Could NOT create fslook dir\n");
		return -ENOMEM;
	}

	chan = create_channel(subbuf_size, n_subbufs);

	return 0;
}

void cleanup_channel(void)
{
	printk("remove channel\n");
	if (dir) {
		printk("------\n");
		debugfs_remove_recursive(dir);
	}
}
