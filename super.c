#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>

#include "fslook.h"

extern struct fslook_info global_fslook_info;
void show_super(struct super_block *sb, void *arg)
{
	struct fslook_info *fi;
	struct dentry *cur, *tmp;
	struct dentry *s_root;
	int i = 0;
	
	fi = (struct fslook_info *)arg;

	if (strncmp(sb->s_type->name, "f2fs", 4) != 0) {
		return;
	}

	/* how to format my output to the output */
	fslook_printf(fi, "-------------\n");
	s_root = sb->s_root;

	fslook_printf(fi, "--->s_root:%s\n", s_root->d_name.name);
	list_for_each_entry_safe(cur, tmp, &sb->s_root->d_subdirs, d_child)
	{
		i++;
		fslook_printf(fi, "%d:cur:%s\n", i, cur->d_name.name);
	}

	i = 0;
	while(true) {
		i++;
		if (i<100);
		fslook_printf(fi, "----------\n");
	}

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
	f(show_super, fi);
}
