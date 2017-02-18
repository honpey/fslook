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

/* fslook kernel part */

/*TODO: kill this function in future */

#if 0
static int __init init_dummy_kernel_functions(void)
{
	/*
	unsigned long *addr;

	 * fslook need symbol ftrace_profile_set_filter to set event filter,
	 *
	 */
	 return 0;
}

#endif

#if 0
static int fslook_main(struct file *file, fslook_option_t *parm)
{
	/*
	unsigned long *buff = NULL;
	fslook_state_t *fs;
	fslook_proto_t *fp;
	*/
	int ret = 0;

	/*
	start_time = gettimeofday_ns();

	delta_time = (gettimeofday_ns() - start_time) / NSEC_PER_USEC;
	*/
	return ret;
}

#endif
static long fslook_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
/*	fslook_option_t parm; */
	int ret = 0;

	switch (cmd) {
		/*
		case FSLOOK_CMD_IOC_VERSION:
			print_version();
			return 0;
		*/
		case FSLOOK_CMD_IOC_RUN:
			printk("hFSLOOK_CMD_IOC_RUN\n");
			show_supers();
#if 0
			if (capable(CAP_SYS_ADMIN))
				return -EACCES;

			ret = copy_from_user(&parm, (void __user *)arg,
							sizeof(fslook_option_t));
			if (ret < 0)
				return -EFAULT;
		/*	return fslook_main(file, &parm); */
#endif
		default:
			printk("HERE?\n");
			return -EINVAL;
	}
	return ret;
}

static const struct file_operations fslook_fops = {
	.llseek	=	no_llseek,
	.unlocked_ioctl	=	fslook_ioctl,
};

static long fslookvm_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	int new_fd, err;
	struct file *new_file;
	new_fd = get_unused_fd_flags(0);
	if (new_fd < 0) {
		printk("fslookvm_ioctl:1\n");
		return new_fd;
	}

	new_file = anon_inode_getfile("[fslook]", &fslook_fops, NULL, O_RDWR);
	if (IS_ERR(new_file)) {
		err = PTR_ERR(new_file);
		put_unused_fd(new_fd);
		printk("fslookvm_ioctl:2\n");
		return err;
	}

	file->private_data = NULL;
	fd_install(new_fd, new_file);
	printk("fslookvm_ioctl:3\n");
	return new_fd;
}

static const struct file_operations fslookvm_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl	= fslookvm_ioctl,
};

/* Fslook Main Entry */
/* fslook_info_t is defined in include/fslook_types.h */
/* where there are some important global structure for */

struct fslook_info global_fslook_info;

/* We wanna to get all the dentries of the system */
static int __init fslook_init(void)
{
	struct fslook_info *fi = &global_fslook_info;

	/*
	if (init_dummy_kernel_functions())
		return -1;
	*/

	fi->dentry = debugfs_create_dir("fslookHH", NULL);
	if (!fi->dentry) {
		printk("fslook: debugfs_create_dir failed:%p\n", fi->dentry);
		return -1;
	}

	fi->dentry_vm = debugfs_create_file("fslookvm", 0444, fi->dentry, NULL, &fslookvm_fops);

	if (!fi->dentry_vm) {
		printk("fdlookvm: cannot create fslook file\n");
		debugfs_remove_recursive(fi->dentry);
		return -1;
	}

	/* init the buffer */
//	fslook_transport_init(fi, fi->dentry);
	init_channel();

	return 0;
}

static void __exit fslook_exit(void)
{
	struct fslook_info *fi = &global_fslook_info;

	debugfs_remove_recursive(fi->dentry);
	cleanup_channel();
	printk("honpey exit\n");

}
module_init(fslook_init)
module_exit(fslook_exit)
MODULE_LICENSE("GPL");


