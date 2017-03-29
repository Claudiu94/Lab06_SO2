/*
 * SO2 - Lab 6 - Deferred Work
 *
 * Exercise #6: kernel thread
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <asm/atomic.h>
#include <linux/kthread.h>

MODULE_DESCRIPTION("Simple kernel thread");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

#define LOG_LEVEL KERN_DEBUG

#define LOCKED		0
#define UNLOCKED	1

wait_queue_head_t wq_stop_thread;
atomic_t flag_stop_thread;
wait_queue_head_t wq_thread_terminated;
atomic_t flag_thread_terminated;
struct task_struct *task;


int my_thread_f(void *data)
{
	printk(LOG_LEVEL "[my_thread_f] Current process id is %d (%s)\n", current->pid, current->comm);

	/* TODO: Wait for command to remove module on wq_stop_thread queue. */
	wait_event_interruptible(wq_stop_thread, atomic_read(&flag_stop_thread) == UNLOCKED);
	/*
	 * TODO: Before completing, notify completion using the
	 * wq_thread_terminated queue and its flag.
	 */
	atomic_set(&flag_thread_terminated, UNLOCKED);
	wake_up_interruptible(&wq_thread_terminated);

	do_exit(0);
}

static int __init kthread_init(void)
{
	printk(LOG_LEVEL "[kthread_init] Init module\n");

	/*
	 * TODO: Initialize the two wait queues and their flags:
	 *   1. wq_stop_thread: used for detecting the remove module
	 *   function being called
	 *   2. wq_thread_terminated: used for detecting the thread
	 *   completing its chore
	 */
	init_waitqueue_head(&wq_stop_thread);
	atomic_set(&flag_stop_thread, UNLOCKED);

	init_waitqueue_head(&wq_thread_terminated);
	atomic_set(&flag_thread_terminated, LOCKED);
	atomic_set(&flag_stop_thread, LOCKED);
	/* TODO: Start a kernel thread that executes my_thread_f(). */
	task = kthread_run(&my_thread_f, NULL, "blabla");

	return 0;
}

static void __exit kthread_exit(void)
{
	/* TODO: Notify kernel thread waiting on wq_stop_thread queue. */
	atomic_set(&flag_stop_thread, UNLOCKED);
	wake_up_interruptible(&wq_stop_thread);
	/*
	 * TODO: Wait for kernel thread to complete on the
	 * wq_thread_terminated queue and its flag.
	 */
	wait_event_interruptible(wq_thread_terminated, atomic_read(&flag_thread_terminated) == UNLOCKED);

	kthread_stop(task);

	printk(LOG_LEVEL "[kthread_exit] Exit module\n");
}

module_init(kthread_init);
module_exit(kthread_exit);
