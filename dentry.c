#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dcache.h>
#include <linux/kprobes.h>

/* 2017/02/15 23:00 */
/* Tonight, I wanna to show all the dentries in linux */
static struct kprobe kp = {
	.symbol_name = "d_drop",

};

static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	printk("d_drop enter\n");
	return 0;
}

static void handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
	printk("d_drop kprobe exit\n");
}

static int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
	printk("d_drop falut\n");
	return 0;
}
int show_all_dentries(void)
{
	int ret;
	kp.pre_handler = handler_pre;
	kp.post_handler = handler_post;
	kp.fault_handler = handler_fault;

	ret = register_kprobe(&kp);
	if (ret < 0) {
		printk(KERN_INFO "register kprobe failed, return %d\n", ret);
		return ret;
	}
	printk(KERN_INFO "Planted kprobe at 0x%p\n", kp.addr);
	return 0;

}
