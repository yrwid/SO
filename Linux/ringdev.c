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
#include <linux/slab.h>
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
static int writePtr;						//Global writePtr
static _Bool backToZero = 0;					//Global variable thats indiacate if  writePtr == readPtr, but  buffor makes one full round
static DECLARE_WAIT_QUEUE_HEAD( headQue );

static int ringdev_open(struct inode *inode, struct file *filp)
{
	int*  zm;
	zm = kmalloc(sizeof(int),GFP_KERNEL);
	if(!zm)  						//Detected kmallock error  
	{
		return -ENOMEM;
	}
	*zm = writePtr;
	filp->private_data = zm;
	return 0;
}

static ssize_t ringdev_read(struct file *filp, char __user *buf, size_t count,loff_t* off)
{ 
	ssize_t ret = 0,wakeUp = 0 ; 				//Return and wakeUp singla  varaibles 
	int *readPtr;

	mutex_lock(&ringdev_lock);
	readPtr = (int*)filp->private_data;

//Check again after wake up
check_again:

 	if(writePtr < *readPtr)
	{
		count = ringdev_len - *readPtr;
		backToZero = 0;
	}
	else if(writePtr  >  *readPtr)
	{
		count = writePtr - *readPtr;
		backToZero = 0;
	}
	else if(writePtr  ==  *readPtr && !backToZero  )
	{
		mutex_unlock(&ringdev_lock);
		wakeUp  = wait_event_interruptible( headQue ,(writePtr != *readPtr) || backToZero );
		if(wakeUp != 0)
		{
			 return -ERESTARTSYS;
		}
		mutex_lock(&ringdev_lock);
		goto check_again;
	}
 	else if(writePtr == *readPtr && backToZero)
	{
		count = ringdev_len;
		backToZero = 0;
	}

	ret = -EFAULT;
	if (copy_to_user(buf,&ringdev_buf[*readPtr], count))
	{
		goto out_unlock;
	}

	ret = count;
	*readPtr += count;
	if(*readPtr >= ringdev_len)				//Looking for buffor  overflow 
	{
		*readPtr = 0;
	}

out_unlock:
	mutex_unlock(&ringdev_lock);
	return ret;
}

static ssize_t ringdev_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *off)
{
	ssize_t ret = 0;					//Return value 
	mutex_lock(&ringdev_lock);

	if(writePtr + count  >= ringdev_len)			//Overflow detected
	{
		count = ringdev_len - writePtr;

	}

	if(copy_from_user(&ringdev_buf[writePtr], buf , count))
	{
		printk(KERN_INFO "copy_from_user  err\n");
		mutex_unlock(&ringdev_lock);
		return -EFAULT;
	}
	ret = count;
	writePtr += count;
	if(writePtr >= ringdev_len)				//Looking for buffor overflow 
	{
		writePtr = 0;
		backToZero = 1 ;
		wake_up_interruptible(&headQue);
		mutex_unlock(&ringdev_lock);
		return -EPIPE;					//Return broken PIPE error
	}
	wake_up_interruptible(&headQue);			//Wake up read function 

	mutex_unlock(&ringdev_lock);
	return ret; 
}

static int ringdev_release(struct inode *inode, struct file *filp)
{
	kfree(filp->private_data);				//Realeas allocated memory 
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
	ret = misc_register(&ringdev_miscdevice);
	if (ret < 0) {
		pr_err("can't register miscdevice.\n");
		return ret;
	}
	writePtr = 0;					//Set writePtr to initial value = 0;
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
