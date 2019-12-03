/*
 * Ring buffer character device - example for Operating System course
 *
 * Copyright (C) 2016, 2017, 2018  Dawid Mudry & Karol Marciniak <mudri656@gmail.com>
 *
 * Add your copyright here and change MODULE_AUTHOR.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define pr_fmt(fmt) "ringdev: " fmt
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/uaccess.h>

/*
 * mutex used for access synchronization to buffer (ringdev_buf and ringdev_len)
 */
static struct mutex ringdev_lock;

/*
 * buffer and number of written bytes in the buffer
 */
static char ringdev_buf[4096];
static size_t ringdev_len;
static int writePtr;
static int readPtr;
static DECLARE_WAIT_QUEUE_HEAD( headQue );
static _Bool flag = 0;

static int ringdev_open(struct inode *inode, struct file *filp)
{
//	printk( KERN_INFO "Hello , world 4!\n" );
	return 0;
}

static ssize_t ringdev_read(struct file *filp, char __user *buf, size_t count,loff_t* off)
{
	if(readPtr > writePtr)
		return 0;

	ssize_t ret = 0;

	mutex_lock(&ringdev_lock);
check_again:

	if (readPtr >= ringdev_len)
	{
		count = 0;
	}
	else if (writePtr  >=  readPtr)
	{

		count = writePtr - readPtr;
	}

			if(count == 0)
			{
				mutex_unlock(&ringdev_lock);
				ret = wait_event_interruptible( headQue ,flag != 0);
  		   	  //	pr_info("\n return val: %zd \n", ret);
 			  //	pr_info("readPtr: %zd \n", readPtr);
  			  //	pr_info("writePtr: %zd \n", writePtr);
				if(ret != 0)
				{
					 return -ERESTARTSYS;
				}
				mutex_lock(&ringdev_lock);
				flag = 0;
				goto check_again;
			}

	ret = -EFAULT;
//	pr_info("read_Ptr = %d, count: %zd\n", readPtr, count);
	if (copy_to_user(buf,&ringdev_buf[readPtr], count))
	{
		goto out_unlock;
	}

	ret = count;
	readPtr = writePtr;

out_unlock:
	mutex_unlock(&ringdev_lock);
//	pr_info("return val: %zd\n",ret);
	return ret;
}

static ssize_t ringdev_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *off)
{
	ssize_t ret = 0;
	mutex_lock(&ringdev_lock);
	if(*off > ringdev_len)
	{
		count = 0;
	}
	else if(count >= ringdev_len - *off)
	{
		count = ringdev_len - *off;
	}


	if(copy_from_user(&ringdev_buf[writePtr] + *off, buf , count))
	{
		printk(KERN_INFO "copy_from_user not null err\n");
		mutex_unlock(&ringdev_lock);
		return -EFAULT;
	}
	ret = count;
//	*off +=  ret;
	writePtr += count;
	flag = 1;
	wake_up_interruptible(&headQue);

	mutex_unlock(&ringdev_lock);
	return ret; 
}

static int ringdev_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static const struct file_operations ringdev_fops = {
	.owner = THIS_MODULE,
	.open = ringdev_open,
	.read = ringdev_read,
	.write = ringdev_write,
	.release = ringdev_release,
};

static struct miscdevice ringdev_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ringdev",
	.fops = &ringdev_fops
};

static int __init ringdev_init(void)
{
	int ret;

	mutex_init(&ringdev_lock);
//	ringdev_len = snprintf(ringdev_buf, sizeof(ringdev_buf),
//			"Hello world!\n");
	ret = misc_register(&ringdev_miscdevice);
	if (ret < 0) {
		pr_err("can't register miscdevice.\n");
		return ret;
	}
	readPtr = 0;
	writePtr = 0;
	pr_info("minor %d\n", ringdev_miscdevice.minor);
	ringdev_len = 4096;
	return 0;
}

static void __exit ringdev_exit(void)
{
	misc_deregister(&ringdev_miscdevice);
	mutex_destroy(&ringdev_lock);
}

module_init(ringdev_init);
module_exit(ringdev_exit);

MODULE_DESCRIPTION("Ring buffer device");
MODULE_AUTHOR("Dawid Mudry | Karol Marciniak");
MODULE_LICENSE("GPL");