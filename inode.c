#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/fs_struct.h>
#include <linux/mount.h>

#include "fslook.h"

extern struct fslook_info global_fslook_info;

/* How to get mount info? */
char *get_name_by_inode(struct inode *inode)
{
	struct dentry *dent;
	char *name = NULL;

	hlist_for_each_entry(dent, &inode->i_dentry, d_u.d_alias) {
		spin_lock(&dent->d_lock);
		name = dent->d_name.name;	
		spin_unlock(&dent->d_lock);
	}
	return name;
}
