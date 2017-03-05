#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/fs_struct.h>
#include <linux/mount.h>

#include "fslook.h"

extern struct fslook_info global_fslook_info;
int jishu = 0;

/* How to get mount info? */
void show_super(struct super_block *sb, void *arg)
{
	struct fslook_info *fi;
	struct dentry *cur, *tmp;
	struct dentry *s_root;
	struct dentry *d_parent;
	int i = 0;
	
	fi = (struct fslook_info *)arg;
//	fi = &global_fslook_info;

	if (jishu % 10 == 0)
		fslook_printf(fi, "\n");
	fslook_printf(fi, "%s ", sb->s_type->name);
	jishu++;
	if (strncmp(sb->s_type->name, "rootfs", 6) != 0) {
		return;
	}

	/* how to format my output to the output */
	s_root = sb->s_root;
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

}

/* As function_supers is NOT exported from kernel, we will NOT
	use this function directly, we can get the kernel address,
	can use it by addr */

/* How to declare a function ptr */
/* iterate_supers: 0xffffffff81210000 */

typedef void (*i_supers)(void (*f)(struct super_block *, void *), void *arg);

typedef struct vfsmount *(*lookup_mnt)(struct path *);
/* Transit to a mounted filesystem */
/* lookup_mnt(struct path *path), what the path is? */
//extern struct vfsmount *lookup_mnt(struct path *);

static int follow_managed(struct fslook_info *fi,
							struct path *path)
{
	struct vfsmount *mounted;
	struct dentry *root_dent;

	lookup_mnt lookup_mnt1;
	lookup_mnt1 = (void *)kallsyms_lookup_name("lookup_mnt");

	mounted = lookup_mnt1(path);
	if (!mounted)
		return 0;
	root_dent = mounted->mnt_root;

	fslook_printf(fi, "   mounted:%s\n",
			root_dent->d_sb->s_type->name);
	return 0;
}

/*
 * Show all super_block inhabitied in System
 */
void show_supers(struct fslook_info *fi)
{
	i_supers f;

	f = (void *)kallsyms_lookup_name("iterate_supers");
	fslook_printf(fi, "FileSystems in this system including:\n");
	f(show_super, fi);
	fslook_printf(fi, "\n");

}

struct parm_for_show_fs
{
	struct fslook_info *fi;
	char *fs_name;
	
};
/* Found special fs info */
void show_spec_fs_info(struct super_block *sb, void *arg)
{
	struct parm_for_show_fs * p_fs_info;
	struct fslook_info *fi;
	char *fs_name, *file_name;
	int len;
	struct dentry *s_root, *d_parent;
	struct inode *cur, *tmp;

	p_fs_info = (struct parm_for_show_fs *)arg;
	fi = p_fs_info->fi;
	fs_name = p_fs_info->fs_name;
	len = strlen(fs_name);
	
	if (strncmp(sb->s_type->name, fs_name, len) != 0)
		return;

	s_root = sb->s_root;
	d_parent = s_root->d_parent;

	list_for_each_entry_safe(cur, tmp, &sb->s_inodes, i_sb_list) {
		/* we add a hard file filter here here, only output the special inode */
		/* can lua work here? */
		file_name = get_name_by_inode(cur);
		if (!file_name)
			continue;
		if (strncmp(file_name, "wwww", 4) != 0)
			continue;

		fslook_printf(fi, "ino(%ld,%s) sb(%s)\n", cur->i_ino,
				get_name_by_inode(cur),
				cur->i_sb->s_type->name);
	}
	return;
	

}

void show_fs(struct fslook_info *fi, char *name)
{
	i_supers f;
	struct parm_for_show_fs fs_info;

	fs_info.fi = fi;
	fs_info.fs_name = name;
	f = (void *)kallsyms_lookup_name("iterate_supers");

	f(show_spec_fs_info, &fs_info);
	return;	
}

void show_root_subdirs(struct fslook_info *fi)
{
	struct path path; /* struct vfsmount *mnt; struct dentry *dentry; */
	struct path subpath;
	struct dentry *dentry;
	struct super_block *sb;
	struct dentry *cur, *tmp;
	unsigned managed;

	get_fs_root(current->fs, &path);
	fslook_printf(fi, "path->dentry's name :%s\n",
			path.dentry->d_name.name);

	dentry = path.dentry;
	sb = dentry->d_sb;

	list_for_each_entry_safe(cur, tmp, &dentry->d_subdirs, d_child)
	{
		subpath.dentry = cur;
		subpath.mnt = path.mnt;
		managed = ACCESS_ONCE(cur->d_flags);
		if (managed & DCACHE_MOUNTED) {
			fslook_printf(fi, "--%s, mounted point(%s)\n",
				cur->d_name.name,
				cur->d_sb->s_type->name);
			follow_managed(fi, &subpath);
		}
		/* */
	}
	fslook_printf(fi, "--------\n");

	managed = ACCESS_ONCE(dentry->d_flags);
	fslook_printf(fi, "managed:0x%x\n", managed);
	fslook_printf(fi, "D_MOUNTED:0x%x\n", DCACHE_MOUNTED);
}
