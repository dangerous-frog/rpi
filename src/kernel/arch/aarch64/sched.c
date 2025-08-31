#include "sched.h"
#include "arm/entry.h"
#include "printf.h"
#include "arm/irq.h"
#include "mm.h"
#include "arm/util.h"


static struct task_struct init_task = INIT_TASK;
struct task_struct *current = &(init_task);
struct task_struct *task[NR_TASKS] = {&(init_task), };
int nr_tasks = 1;


void preempt_disable(void) {
    current->preempt_count++;
}

void preempt_enable(void) {
    current->preempt_count--;
}


void _schedule(void) {
    preempt_disable();
    // Taken from first Linux release
	int next,c;
	struct task_struct * p;
    // This loops because if no task can run, then we just gotta wait for that to happen
	while (1) {
		c = -1;
		next = 0;
        // Here we find the task with maximum counter (so highest prio)
        // That is also actually running
		for (int i = 0; i < NR_TASKS; i++){
			p = task[i];
			if (p && p->state == TASK_RUNNING && p->counter > c) {
				c = p->counter;
				next = i;
			}
		}
        // If we found something then we go well do it
		if (c) {
			break;
		}
        // Otherwise go through all tasks and just refresh their counters 
		for (int i = 0; i < NR_TASKS; i++) {
			p = task[i];
			if (p) {
				p->counter = (p->counter >> 1) + p->priority;
			}
		}
	}
	switch_to(task[next]);
	preempt_enable();
}


void schedule(void) {
    // The task called schedule so this ought to get zero'd
    current->counter = 0;
    _schedule();
}

void switch_to(struct task_struct * next) {
    if (current == next) 
        return;    
    struct task_struct * prev = current;
    current = next;
    cpu_switch_to(prev, next);
}


void schedule_tail(void) {
    preempt_enable();
}


void timer_tick() {
    // Decrement counter
    --current->counter;
    // If it's not time yet or we can't preempt, then ain't a thing we can do
    if (current->counter > 0 || current->preempt_count > 0) {
        return;
    }
    // If we stayed here, ensure it is 0
    current->counter = 0;
    enable_irq();
    _schedule();
    disable_irq();
    
}


int copy_process(unsigned long clone_flags, unsigned long fn, unsigned long arg, unsigned long stack) {
    // I'm trying to understand this here
    preempt_disable();
    // We're in some task so no preemption cause that's gonna explode
    struct task_struct *p;

    unsigned long page = allocate_kernel_page();
    p = (struct task_struct * )page;
    if (!p)
        return 1;
    struct pt_regs *childregs = task_pt_regs(p);
	memzero((unsigned long)childregs, sizeof(struct pt_regs));

    if (clone_flags & PF_KTHREAD) {
        // It is kernel thread, we do same
        p->cpu_context.x19 = fn;
        p->cpu_context.x20 = arg;
    } else {
        // 'Tis user thread
        struct pt_regs * cur_regs = task_pt_regs(current);
		*childregs = *cur_regs;
		childregs->regs[0] = 0;
		childregs->sp = stack + PAGE_SIZE;
		p->stack = stack;
        copy_virt_memory(p); // We need to copy virt mem in user space
    }

    p->flags = clone_flags;
    p->priority = current->priority;
    p->state = TASK_RUNNING;
    p->counter = p->priority; // We want it to run for prio ticks
    p->preempt_count = 1; // It shouldn't get preempted immediately when it's scheduled

    // We init rest of fields and then return to caller
    p->cpu_context.pc = (unsigned long)ret_from_fork; 
    p->cpu_context.sp = (unsigned long)childregs;
    int pid = nr_tasks++;
    task[pid] = p;
    preempt_enable();
    return pid;
}

struct pt_regs * task_pt_regs(struct task_struct *tsk){ // To my understanding
    // This converts a task to kind of what do we need to load into cpu to emulate it
	unsigned long p = (unsigned long)tsk + THREAD_SIZE - sizeof(struct pt_regs);
	return (struct pt_regs *)p;
}

int move_to_user_mode(unsigned long start, unsigned long size,unsigned long pc) {
    struct pt_regs *regs = task_pt_regs(current);
	regs->pstate = PSR_MODE_EL0t;
	regs->pc = pc;
	regs->sp = 2 *  PAGE_SIZE;  // For now, no more than 1 page text and 1 page stack
	unsigned long code_page = allocate_user_page(current, 0);
	if (code_page == 0)	{
		return -1;
	}
	memcpy(code_page, start, size); // Copies to virt memory we allocated
	set_pgd(current->mm.pgd); // activates translation tables for this process
	return 0;
}

void exit_process(){
    preempt_disable();
    for (int i = 0; i < NR_TASKS; i++){
        if (task[i] == current) {
            task[i]->state = TASK_ZOMBIE;
            break;
        }
    }
    if (current->stack) {
        free_page(current->stack);
    }
    preempt_enable();
    schedule();
}

