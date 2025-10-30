#include "sched.h"
#include "arm/entry.h"
#include "printf.h"
#include "arm/irq.h"
#include "heap.h"
#include "stddef.h"
#include "mm.h"
#include "arm/util.h"


static struct task_struct init_task = INIT_TASK;
struct task_struct *current = &(init_task);
struct task_struct **task;
int nr_tasks = 1;
static int max_tasks = NR_TASKS;
struct isr_wait_struct* wait_list[NR_ISR]; 



void add_to_isr_list(int isr_num, long time, struct task_struct* task_ptr) {
    struct isr_wait_struct* new = kmalloc(sizeof(struct isr_wait_struct));
    new->ticks_to_wait = time;
    new->task_ptr = task_ptr;

    struct isr_wait_struct* isr_ptr = wait_list[isr_num];

    // Check if this is first element
    if (isr_ptr == NULL) {
        wait_list[isr_num] = new;
        new->next = NULL;
        return;
    }

    // Check if we should insert before first
    if (isr_ptr->ticks_to_wait >= time ) {
        new->next = isr_ptr;
        wait_list[isr_num] = new;
        new->next->ticks_to_wait -= time;
        return;
    }

    // Loop until end
    while(isr_ptr->next != NULL ) {
        if (time - isr_ptr->ticks_to_wait < 0) 
            break;
        time -= isr_ptr->ticks_to_wait;
        isr_ptr = isr_ptr->next;
    }
    
    // Now we have place within queue and time left
    if (isr_ptr->next == NULL) {
        // We are last, we can just add remaining time
        isr_ptr->next = new;
        new->next = NULL;
    } else {
        // This means there is another element, and it was bigger than our current time
        new->next = isr_ptr->next;
        isr_ptr->next = new;
        new->next->ticks_to_wait -= time; // Subtract time from next to node so overall time stays same.    
    }
}

void register_for_isr(int isr_num) {
    preempt_disable();
    add_to_isr_list(isr_num, 0, current);
    preempt_enable();
}

// Let the scheduler know the interrupt has fired so it can 
// update internal structures and wake tasks as needed
void handle_isr_wake_up(int isr_num) {
    struct isr_wait_struct* isr_ptr = wait_list[isr_num];

    if ( isr_ptr != NULL && isr_ptr->ticks_to_wait > 0) 
        isr_ptr->ticks_to_wait -= 1;

    while(isr_ptr != NULL && isr_ptr->ticks_to_wait == 0) {
        printf("%x\n", *((int*) 0xffff000000081068));
        isr_ptr->task_ptr->state = TASK_RUNNING;
        printf("Woken task %x\n", isr_ptr->task_ptr);
        wait_list[isr_num] = isr_ptr->next;
        kfree(isr_ptr);
        isr_ptr = wait_list[isr_num];
    }
}


void preempt_disable(void) {
    current->preempt_count++;
}

void preempt_enable(void) {
    current->preempt_count--;
}

void delay_ticks(long ticks) {
    if (ticks == 0) 
        return;
    // Hardcoded number for timer interrupt, stupid but this OS won't grow bigger 
    add_to_isr_list(30, ticks, current);
    printf("Stalled task %x\n", current);
    current->state = TASK_WAITING;
    schedule();
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
		for (int i = 0; i < nr_tasks; i++){
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
		for (int i = 0; i < nr_tasks; i++) {
			p = task[i];
			if (p) {
				p->counter = (p->counter >> 1) + p->priority;
			}
		}
	}
	switch_to(task[next]);
	preempt_enable();
}




void sched_init() {
    // Initialize list of tasks, so it's nicely on heap
    task = kmalloc(sizeof(struct task_struct * ) * NR_TASKS);
    if (!task){ 
        printf("Failed to init scheduler");
        return;
    }
    
    for (int i = 1; i < NR_TASKS; i++) {
        task[i] = NULL;
    }

    task[0] = &init_task;

    // Initialize first pointers for isr waiting
    for (int i = 0; i < NR_ISR; i++) {
        wait_list[i] = NULL;
    }
}

void set_task_prio(int prio) {
    current->priority = prio;
    current->counter = prio;
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

    // Get more space if needed
    if (pid >= max_tasks) {
        max_tasks *= 2;
        struct task_struct ** new_tasks = kmalloc(sizeof(struct task_struct *)* max_tasks);
        if (!new_tasks) {
            printf("Can't get more tasks");
            return -1;
        }
        for (int i = 0; i < max_tasks/2; i++) {
            new_tasks[i] = task[i];
        }
        for (int i = max_tasks/2; i < max_tasks; i++) {
            new_tasks[i] = NULL;
        }
        kfree(task);
        task = new_tasks;
    }

    task[pid] = p;
    printf("New task at: %x\n", task[pid]);
    preempt_enable();
    return pid;
}

struct pt_regs * task_pt_regs(struct task_struct *tsk){ // To my understanding
    // This converts a task to kind of what do we need to load into cpu to emulate it
	unsigned long p = (unsigned long)tsk + THREAD_SIZE - sizeof(struct pt_regs);
	return (struct pt_regs *)p;
}

int move_to_user_mode(unsigned long start, unsigned long size, unsigned long pc) {
    struct pt_regs *regs = task_pt_regs(current);
    regs->pstate = PSR_MODE_EL0t;
    
    extern char user_begin, user_end;
    unsigned long user_start = (unsigned long)&user_begin;
    unsigned long user_end_addr = (unsigned long)&user_end;
    unsigned long user_size = user_end_addr - user_start;
    unsigned long num_pages = (user_size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    // Map all user pages to LOW virtual addresses
    for (unsigned long i = 0; i < num_pages; i++) {
        unsigned long virt_addr = i * PAGE_SIZE;
        unsigned long phys_page = allocate_user_page(current, virt_addr);
        
        if (phys_page == 0) {
            printf("Failed to allocate page %d\n", i);
            return -1;
        }
        
        unsigned long copy_start = start + (i * PAGE_SIZE);
        unsigned long copy_size = PAGE_SIZE;
        
        if (i * PAGE_SIZE >= size) {
            copy_size = 0;
        } else if ((i + 1) * PAGE_SIZE > size) {
            copy_size = size - (i * PAGE_SIZE);
        }
        
        if (copy_size > 0) {
            memcpy(phys_page, copy_start, copy_size);
        }
    }

    // Allocate stack page
    unsigned long stack_vaddr = num_pages * PAGE_SIZE;
    unsigned long stack_page = allocate_user_page(current, stack_vaddr);
    if (stack_page == 0) {
        printf("Failed to allocate stack\n");
        return -1;
    }
    
    // PC is ALREADY an offset, don't subtract again!
    regs->pc = pc;  // Already 0x114
    printf("My pc is 0x%x\n", pc);
    printf("Instr: %x\n", *((int*) 0xffff0000000811f8));
    regs->sp = stack_vaddr + PAGE_SIZE;

set_pgd(current->mm.pgd);
    return 0;
}

void exit_process(){
    preempt_disable();
    for (int i = 0; i < nr_tasks; i++){
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

// int move_to_user_mode(unsigned long start, unsigned long size,unsigned long pc) {
//     struct pt_regs *regs = task_pt_regs(current);
// 	regs->pstate = PSR_MODE_EL0t;
// 	regs->pc = pc;


// 	// Calculate how many pages we need
//     unsigned long num_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
//     regs->sp = num_pages * PAGE_SIZE;  // Stack after code pages
    
//     // Allocate all needed pages
//     for (unsigned long i = 0; i < num_pages; i++) {
//         unsigned long va = i * PAGE_SIZE;
//         unsigned long code_page = allocate_user_page(current, va);
//         if (code_page == 0) {
//             return -1;
//         }
        
//         // Copy this page's worth of data
//         unsigned long copy_size = PAGE_SIZE;
//         if (i == num_pages - 1) {
//             // Last page might be partial
//             copy_size = size - (i * PAGE_SIZE);
//         }
//         memcpy(code_page, start + (i * PAGE_SIZE), copy_size);
//     }
    
// 	set_pgd(current->mm.pgd); // activates translation tables for this process
// 	return 0;
// }