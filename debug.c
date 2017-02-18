#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

struct debug_node {
	struct list_head dn_list;
	char name[20];
	int No;
};

char *names[10] = {"xiaohong",
					"xiaoming",
					"john",
					"polar",
					"susan",
					"tina",
					"kota",
					"xiu",
					"WangTao",
					"Siyu"
					};

void test_list(void)
{
	struct debug_node *pDN;
	LIST_HEAD(db_list_head);
	struct debug_node *cur, *tmp;
	int i = 0;

	while (i++ < 9) {
		pDN = kmalloc(sizeof(struct debug_node), GFP_NOFS | __GFP_ZERO);
		if (!pDN)
			return;
		pDN->No = i;
		strcpy(pDN->name, names[i]);
		list_add_tail(&pDN->dn_list, &db_list_head);
	}

	list_for_each_entry_safe(cur, tmp, &db_list_head, dn_list) {
		printk("lfees:%s\n", cur->name);
	}
}
