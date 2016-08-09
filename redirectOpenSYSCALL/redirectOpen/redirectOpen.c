#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <asm/unistd.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <linux/moduleparam.h>

static char __user *redirection = NULL;
static char __user *target = NULL;
static unsigned long **sys_call_table;
static unsigned long le_cr0;
static unsigned long offset;

//module_param(redirection, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
module_param(redirection, charp, 0000 );
MODULE_PARM_DESC(redirection, "Path to redirected open call");

//module_param(target, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
module_param(target, charp, 0000 );
MODULE_PARM_DESC(target, "What shall not be opened");

static asmlinkage long (*system_open) ( const char __user *filename, int flags, umode_t mode);

static asmlinkage long redirect_open( const char __user *filename, int flags, umode_t mode)
{
 int len = strlen(filename);
 int lenTarget = strlen(target);
 
    if(strcmp(filename + len - lenTarget, target)) {
	return (*system_open)(filename, flags, mode);
    } else {
        //NOT SURE HOW TO USE copy_to_user function
        //copy_to_user( (void *) filename, redirection, strlen( redirection ) );

        printk(KERN_ALERT "succesfully compared target\n");
	mm_segment_t old_fs;
	long fd;
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd = (system_open)(redirection, flags, mode);
	set_fs(old_fs);

	return fd;
    }
}

static int __init set_redirect(void) {
  if (redirection == NULL || target == NULL)
  {
    printk(KERN_ALERT "USAGE: insmod redirectOpen.ko [New Destination] [Target]\n");
    return -EINVAL;
  }

  printk(KERN_ALERT "Parameters are fine\n");

  for (offset=PAGE_OFFSET; offset<ULLONG_MAX; offset+=sizeof(void *))
  {
    sys_call_table = (unsigned long **) offset;

    if ( sys_call_table[__NR_close] == (unsigned long *) sys_close)
      break;
  }

  if ( offset == ULLONG_MAX) 
    return -1;

  if (sys_call_table == NULL)
  {
    printk(KERN_ALERT "Table not found\n");
    return -EPERM;
  }

  printk(KERN_ALERT "Found Table\n");

  le_cr0 = read_cr0();
  write_cr0( le_cr0 & (~0x00010000) );
  system_open = (void *) sys_call_table[__NR_open];
  sys_call_table[__NR_open] = (unsigned long *) redirect_open;
  write_cr0( le_cr0 );	
  return 0;
}

static void __exit remove_redirection(void)
{
  write_cr0( le_cr0 & (~0x00010000) );
  sys_call_table[__NR_open] = (unsigned long *) system_open;
  write_cr0( le_cr0 );
  printk(KERN_ALERT "Removed redirect\n");
}

module_init(set_redirect);
module_exit(remove_redirection);

MODULE_AUTHOR("Roy Lara");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ADVANCED UNIX PROGRAMMING PROJECT, Module Intended to hide file through redirection by rewriting open systemcall");
