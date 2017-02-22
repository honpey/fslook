#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/debugfs.h>
#include <linux/vmalloc.h>
#include <linux/anon_inodes.h>
#include <linux/file.h>
#include <linux/time.h> /* gettimeofday_ns */
#include <linux/clocksource.h>
#include <linux/uaccess.h> /* copy_from_user */

#include "fslook.h"
#include "include/fslook_types.h"

static long fslook_ioctl(struct file *file,
			unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct fslook_info *fi;

	fi = (struct fslook_info *)file->private_data;
	switch (cmd) {
		case FSLOOK_CMD_IOC_RUN:
			show_supers(fi);
			break;
		default:
			return -EINVAL;
	}
	return ret;
}

static const struct file_operations fslook_fops = {
	.llseek	=	no_llseek,
	.unlocked_ioctl	=	fslook_ioctl,
};

/*
 * How to pass the fslook_info struct to struct inode & file
 *
 * fslookvm_ioctl: 
 *
 */
static long fslookvm_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	int new_fd, err;
	struct file *new_file;
	struct inode *inode;
	void *priv;

	new_fd = get_unused_fd_flags(0);
	if (new_fd < 0) {
		return new_fd;
	}

#if 1 
	inode = file->f_inode;
	priv = inode->i_private;
#endif

	/* Just one anon_inode in kernel? */
	new_file = anon_inode_getfile("[fslook]", &fslook_fops, priv, O_RDWR);
	if (IS_ERR(new_file)) {
		err = PTR_ERR(new_file);
		put_unused_fd(new_fd);
		return err;
	}

	file->private_data = priv;
	fd_install(new_fd, new_file);
	return new_fd;
}

static const struct file_operations fslookvm_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl	= fslookvm_ioctl,
};

/* Fslook Main Entry */
struct fslook_info global_fslook_info;

static int __init fslook_init(void)
{
	struct fslook_info *fi = &global_fslook_info;

	fi->dentry = debugfs_create_dir("fslook", NULL);
	if (!fi->dentry) {
		printk("fslook: debugfs_create_dir failed:%p\n", fi->dentry);
		return -1;
	}

	fi->dentry_vm = debugfs_create_file("fslookvm", 0444,
						fi->dentry, fi/* global control */, &fslookvm_fops);
	if (!fi->dentry_vm) {
		printk("fdlookvm: cannot create fslook file\n");
		debugfs_remove_recursive(fi->dentry);
		return -1;
	}

	init_channel(fi);

	return 0;
}

static void __exit fslook_exit(void)
{
	struct fslook_info *fi = &global_fslook_info;

	debugfs_remove_recursive(fi->dentry);
	cleanup_channel();
	printk("FSLOOK exit\n");

}
module_init(fslook_init)
module_exit(fslook_exit)
MODULE_LICENSE("GPL");


