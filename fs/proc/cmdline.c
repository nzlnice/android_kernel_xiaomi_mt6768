// SPDX-License-Identifier: GPL-2.0
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#ifdef CONFIG_KSU_SUSFS_SPOOF_CMDLINE_OR_BOOTCONFIG
extern int susfs_spoof_cmdline_or_bootconfig(struct seq_file *m);
#endif

// 如果你同时还想保留普通的 KernelSU 挂钩（做双重保险，如果 SUSFS 没开启的话）
#ifdef CONFIG_KERNELSU
extern char *ksu_handle_cmdline(char *cmdline);
#endif

static int cmdline_proc_show(struct seq_file *m, void *v)
{
	/* 1. 优先走 SUSFS 的命令行欺骗逻辑 */
#ifdef CONFIG_KSU_SUSFS_SPOOF_CMDLINE_OR_BOOTCONFIG
	if (!susfs_spoof_cmdline_or_bootconfig(m)) {
		seq_putc(m, '\n');
		return 0;
	}
#endif

	/* 2. 如果 SUSFS 没起作用或未开启，回退到普通 KernelSU 处理，或者原本的系统逻辑 */
#ifdef CONFIG_KERNELSU
	seq_printf(m, "%s\n", ksu_handle_cmdline(saved_command_line));
#else
	seq_printf(m, "%s\n", saved_command_line);
#endif

	return 0;
}

static int cmdline_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cmdline_proc_show, NULL);
}

static const struct file_operations cmdline_proc_fops = {
	.open		= cmdline_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_cmdline_init(void)
{
	proc_create("cmdline", 0, NULL, &cmdline_proc_fops);
	return 0;
}
fs_initcall(proc_cmdline_init);