#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>

#include "fslook.h"

extern struct fslook_info global_fslook_info;
int jishu = 0;
/* How to get mount info? */
void show_super(struct super_block *sb, void *arg)
{
	struct fslook_info *fi;
	struct dentry *cur, *tmp;
	struct dentry *s_root;
	int i = 0;
	
//	fi = (struct fslook_info *)arg;
	fi = &global_fslook_info;

	if (jishu%10 == 0)
		fslook_printf(fi, "\n");
	fslook_printf(fi, "%s ", sb->s_type->name);
	jishu++;
	if (strncmp(sb->s_type->name, "rootfs", 6) != 0) {
		return;
	}

	/* how to format my output to the output */
	fslook_printf(fi, "-------------\n");
	s_root = sb->s_root;
	struct dentry *d_parent;
	d_parent = s_root->d_parent;
	if (d_parent) {
		fslook_printf(fi, "has\n");
	} else {
		fslook_printf(fi, "nohas\n");
	}
	list_for_each_entry_safe(cur, tmp, &sb->s_root->d_subdirs, d_child)
	{
		fslook_printf(fi, "%d:%s\n", i, cur->d_name.name);
	}
	fslook_printf(fi, "____________\n");


}

/* As function_supers is NOT exported from kernel, we will NOT
	use this function directly, we can get the kernel address,
	can use it by addr */

/* How to declare a function ptr */
/* iterate_supers: 0xffffffff81210000 */

typedef void (*i_supers)(void (*f)(struct super_block *, void *), void *arg);

void show_supers(struct fslook_info *fi)
{
	i_supers f = (i_supers)0xffffffff81210000;
	fslook_printf(fi, "FileSystems in this system including:\n");
	f(show_super, fi);
	fslook_printf(fi, "\n");
}
