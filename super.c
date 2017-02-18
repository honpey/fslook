#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>

#include "fslook.h"

void show_super(struct super_block *sb, void *arg)
{
	struct dentry *cur, *tmp;
	struct dentry *s_root;
	int i = 0;

	if (strncmp(sb->s_type->name, "f2fs", 4) != 0) {
		return;
	}

	/* how to format my output to the output */
	fslook_printf("-------------\n");
	s_root = sb->s_root;

	fslook_printf("--->s_root:%s\n", s_root->d_name.name);
	list_for_each_entry_safe(cur, tmp, &sb->s_root->d_subdirs, d_child)
	{
		i++;
		fslook_printf("%d:cur:%s\n", i, cur->d_name.name);
	}

}

/* As function_supers is NOT exported from kernel, we will NOT
	use this function directly, we can get the kernel address,
	can use it by addr */

/* How to declare a function ptr */
/* iterate_supers: 0xffffffff81210000 */

typedef void (*i_supers)(void (*f)(struct super_block *, void *), void *arg);

void show_supers(void)
{
	i_supers f = (i_supers)0xffffffff81210000;
	f(show_super, NULL);
}
