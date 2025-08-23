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


int copy_process(unsigned long fn, unsigned long arg) {
    // I'm trying to understand this here
    preempt_disable();
    // We're in some task so no preemption cause that's gonna explode
    struct task_struct *p;

    p = (struct task_struct *) get_free_page(); // Let's get some of that memory for new task
    if (!p)
        return 1;
    p->priority = current->priority;
    p->state = TASK_RUNNING;
    p->counter = p->priority; // We want it to run for prio ticks
    p->preempt_count = 1; // It shouldn't get preempted immediately when it's scheduled

    // We init rest of fields and then return to caller
    p->cpu_context.x19 = fn;
    p->cpu_context.x20 = arg;
    p->cpu_context.pc = (unsigned long)ret_from_fork; 
    p->cpu_context.sp = (unsigned long)p + THREAD_SIZE;
    int pid = nr_tasks++;
    task[pid] = p;
    preempt_enable();
    return 0;
}