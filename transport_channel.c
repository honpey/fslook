#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/relay.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include "fslook.h"
#include "include/fslook_types.h"

static int subbuf_start_handler(struct rchan_buf *buf,
		void *subbuf,
		void *prev_subbuf,
		size_t prev_padding)
{
	if (relay_buf_full(buf)) {
		printk("cpu:%d %d\n", smp_processor_id(), 1); 
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

static struct rchan *create_channel(struct dentry *dir,
			unsigned size, unsigned n)
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
void fslook_printf(struct fslook_info *fi, const char *fmt, ...)
{
	char *buff;
	va_list args;
	int len;
	struct rchan *chan;

	chan = fi->flcm->chan;

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

int init_channel(struct fslook_info *fi)
{
	struct channel_manager *flcm;

	flcm = kmalloc(sizeof(struct channel_manager), GFP_KERNEL);
	if (!flcm) {
		printk("NO mem to create channel_manager\n");
		return -ENOMEM;
	}

	flcm->dir = debugfs_create_dir("channel", fi->dentry);
	if (!flcm->dir) {
		printk("Could NOT create fslook dir\n");
		return -ENOMEM;
	}

	flcm->subbuf_size = 262144; /* 256K each buffer */
	flcm->n_subbufs = 2;

	flcm->chan = create_channel(flcm->dir, 262144, 2);

	fi->flcm = flcm;

	return 0;
}

void cleanup_channel(void)
{
	/*
	if (dir) {
		printk("------\n");
		debugfs_remove_recursive(dir);
	}
	*/
}
