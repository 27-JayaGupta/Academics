#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "procstat.h"

struct cpu cpus[NCPU];

struct proc proc[NPROC];

struct proc *initproc;

int nextpid = 1;
struct spinlock pid_lock;

extern void forkret(void);
static void freeproc(struct proc *p);

extern char trampoline[]; // trampoline.S

// helps ensure that wakeups of wait()ing
// parents are not lost. helps obey the
// memory model when using p->parent.
// must be acquired before any p->lock.
struct spinlock wait_lock;

extern int current_sched_policy;

// Global variables for printing statistics of a batch
int BATCH_SIZE = 0;
int CURRENT_BATCH_SIZE = 0; // batch size maintains the number of active processes from a batch

struct batch_stat stats = {
  .batch_exec_time = 0,
  .first_proc_in_batch = 0,
  .avg_turnaround_time = 0,
  .avg_waiting_time = 0,

  .max_completion_time = 0,
  .min_completion_time = 1000000000,
  .avg_completion_time = 0,

  .count_cpu_burst = 0,

  .avg_cpu_burst = 0,
  .max_cpu_burst = 0,
  .min_cpu_burst = 1000000000,

  .count_est_cpu_burst = 0,
  .avg_cpu_burst_est = 0,
  .max_cpu_burst_est = 0,
  .min_cpu_burst_est = 1000000000,

  .est_error_count = 0,
  .est_error_avg = 0,
};

// Allocate a page for each process's kernel stack.
// Map it high in memory, followed by an invalid
// guard page.
void
proc_mapstacks(pagetable_t kpgtbl) {
  struct proc *p;
  
  for(p = proc; p < &proc[NPROC]; p++) {
    char *pa = kalloc();
    if(pa == 0)
      panic("kalloc");
    uint64 va = KSTACK((int) (p - proc));
    kvmmap(kpgtbl, va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
  }
}

// initialize the proc table at boot time.
void
procinit(void)
{
  struct proc *p;
  
  initlock(&pid_lock, "nextpid");
  initlock(&wait_lock, "wait_lock");
  for(p = proc; p < &proc[NPROC]; p++) {
      initlock(&p->lock, "proc");
      p->kstack = KSTACK((int) (p - proc));
  }
}

// Must be called with interrupts disabled,
// to prevent race with process being moved
// to a different CPU.
int
cpuid()
{
  int id = r_tp();
  return id;
}

// Return this CPU's cpu struct.
// Interrupts must be disabled.
struct cpu*
mycpu(void) {
  int id = cpuid();
  struct cpu *c = &cpus[id];
  return c;
}

// Return the current struct proc *, or zero if none.
struct proc*
myproc(void) {
  push_off();
  struct cpu *c = mycpu();
  struct proc *p = c->proc;
  pop_off();
  return p;
}

int
allocpid() {
  int pid;
  
  acquire(&pid_lock);
  pid = nextpid;
  nextpid = nextpid + 1;
  release(&pid_lock);

  return pid;
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  uint xticks;

  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if(p->state == UNUSED) {
      goto found;
    } else {
      release(&p->lock);
    }
  }
  return 0;

found:
  p->pid = allocpid();
  p->state = USED;

  // Allocate a trapframe page.
  if((p->trapframe = (struct trapframe *)kalloc()) == 0){
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // An empty user page table.
  p->pagetable = proc_pagetable(p);
  if(p->pagetable == 0){
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // Set up new context to start executing at forkret,
  // which returns to user space.
  memset(&p->context, 0, sizeof(p->context));
  p->context.ra = (uint64)forkret;
  p->context.sp = p->kstack + PGSIZE;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);

  p->ctime = xticks;
  p->stime = -1;
  p->endtime = -1;

  return p;
}

// free a proc structure and the data hanging from it,
// including user pages.
// p->lock must be held.
static void
freeproc(struct proc *p)
{
  if(p->trapframe)
    kfree((void*)p->trapframe);
  p->trapframe = 0;
  if(p->pagetable)
    proc_freepagetable(p->pagetable, p->sz);
  p->pagetable = 0;
  p->sz = 0;
  p->pid = 0;
  p->parent = 0;
  p->name[0] = 0;
  p->chan = 0;
  p->killed = 0;
  p->xstate = 0;
  p->state = UNUSED;
}

// Create a user page table for a given process,
// with no user memory, but with trampoline pages.
pagetable_t
proc_pagetable(struct proc *p)
{
  pagetable_t pagetable;

  // An empty page table.
  pagetable = uvmcreate();
  if(pagetable == 0)
    return 0;

  // map the trampoline code (for system call return)
  // at the highest user virtual address.
  // only the supervisor uses it, on the way
  // to/from user space, so not PTE_U.
  if(mappages(pagetable, TRAMPOLINE, PGSIZE,
              (uint64)trampoline, PTE_R | PTE_X) < 0){
    uvmfree(pagetable, 0);
    return 0;
  }

  // map the trapframe just below TRAMPOLINE, for trampoline.S.
  if(mappages(pagetable, TRAPFRAME, PGSIZE,
              (uint64)(p->trapframe), PTE_R | PTE_W) < 0){
    uvmunmap(pagetable, TRAMPOLINE, 1, 0);
    uvmfree(pagetable, 0);
    return 0;
  }

  return pagetable;
}

// Free a process's page table, and free the
// physical memory it refers to.
void
proc_freepagetable(pagetable_t pagetable, uint64 sz)
{
  uvmunmap(pagetable, TRAMPOLINE, 1, 0);
  uvmunmap(pagetable, TRAPFRAME, 1, 0);
  uvmfree(pagetable, sz);
}

// a user program that calls exec("/init")
// od -t xC initcode
uchar initcode[] = {
  0x17, 0x05, 0x00, 0x00, 0x13, 0x05, 0x45, 0x02,
  0x97, 0x05, 0x00, 0x00, 0x93, 0x85, 0x35, 0x02,
  0x93, 0x08, 0x70, 0x00, 0x73, 0x00, 0x00, 0x00,
  0x93, 0x08, 0x20, 0x00, 0x73, 0x00, 0x00, 0x00,
  0xef, 0xf0, 0x9f, 0xff, 0x2f, 0x69, 0x6e, 0x69,
  0x74, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

// Set up first user process.
void
userinit(void)
{
  struct proc *p;

  p = allocproc();
  initproc = p;
  
  // allocate one user page and copy init's instructions
  // and data into it.
  uvminit(p->pagetable, initcode, sizeof(initcode));
  p->sz = PGSIZE;

  // prepare for the very first "return" from kernel to user.
  p->trapframe->epc = 0;      // user program counter
  p->trapframe->sp = PGSIZE;  // user stack pointer

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;

  release(&p->lock);
}

// Grow or shrink user memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *p = myproc();

  sz = p->sz;
  if(n > 0){
    if((sz = uvmalloc(p->pagetable, sz, sz + n)) == 0) {
      return -1;
    }
  } else if(n < 0){
    sz = uvmdealloc(p->pagetable, sz, sz + n);
  }
  p->sz = sz;
  return 0;
}

// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *p = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy user memory from parent to child.
  if(uvmcopy(p->pagetable, np->pagetable, p->sz) < 0){
    freeproc(np);
    release(&np->lock);
    return -1;
  }
  np->sz = p->sz;

  // copy saved user registers.
  *(np->trapframe) = *(p->trapframe);
  np->special = 0;
  // Cause fork to return 0 in the child.
  np->trapframe->a0 = 0;

  // increment reference counts on open file descriptors.
  for(i = 0; i < NOFILE; i++)
    if(p->ofile[i])
      np->ofile[i] = filedup(p->ofile[i]);
  np->cwd = idup(p->cwd);

  safestrcpy(np->name, p->name, sizeof(p->name));

  pid = np->pid;

  release(&np->lock);

  acquire(&wait_lock);
  np->parent = p;
  release(&wait_lock);

  acquire(&np->lock);
  np->state = RUNNABLE;
  release(&np->lock);

  return pid;
}

int
forkf(uint64 faddr)
{
  int i, pid;
  struct proc *np;
  struct proc *p = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy user memory from parent to child.
  if(uvmcopy(p->pagetable, np->pagetable, p->sz) < 0){
    freeproc(np);
    release(&np->lock);
    return -1;
  }
  np->sz = p->sz;

  // copy saved user registers.
  *(np->trapframe) = *(p->trapframe);
  np->special = 0;
  // Cause fork to return 0 in the child.
  np->trapframe->a0 = 0;
  // Make child to jump to function
  np->trapframe->epc = faddr;

  // increment reference counts on open file descriptors.
  for(i = 0; i < NOFILE; i++)
    if(p->ofile[i])
      np->ofile[i] = filedup(p->ofile[i]);
  np->cwd = idup(p->cwd);

  safestrcpy(np->name, p->name, sizeof(p->name));

  pid = np->pid;

  release(&np->lock);

  acquire(&wait_lock);
  np->parent = p;
  release(&wait_lock);

  acquire(&np->lock);
  np->state = RUNNABLE;
  release(&np->lock);

  return pid;
}

// Pass p's abandoned children to init.
// Caller must hold wait_lock.
void
reparent(struct proc *p)
{
  struct proc *pp;

  for(pp = proc; pp < &proc[NPROC]; pp++){
    if(pp->parent == p){
      pp->parent = initproc;
      wakeup(initproc);
    }
  }
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
void
exit(int status)
{
  struct proc *p = myproc();


  if(p == initproc)
    panic("init exiting");

  // Close all open files.
  for(int fd = 0; fd < NOFILE; fd++){
    if(p->ofile[fd]){
      struct file *f = p->ofile[fd];
      fileclose(f);
      p->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(p->cwd);
  end_op();
  p->cwd = 0;

  acquire(&wait_lock);

  // Give any children to init.
  reparent(p);

  // Parent might be sleeping in wait().
  wakeup(p->parent);
  
  acquire(&p->lock);

  p->xstate = status;
  p->state = ZOMBIE;

  uint xticks;
  release(&wait_lock);

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);

  p->endtime = xticks;

  // calculate average turnaround time
  if(p->special){
    CURRENT_BATCH_SIZE -=1;
  
    stats.avg_turnaround_time += (p->endtime - p->ctime);
    stats.avg_completion_time += p->endtime;

    if(p->endtime < stats.min_completion_time) {
      stats.min_completion_time = p->endtime;
    } else if(p->endtime > stats.max_completion_time) {
      stats.max_completion_time = p->endtime;
    }

    // Calculate batch execution time if the process is the last process from the batch
    if((CURRENT_BATCH_SIZE == 0) && (BATCH_SIZE != 0)){
      stats.batch_exec_time = p->endtime - stats.batch_exec_time;
    }

    // Calculate estimate of cpu burst for sjf
    if(current_sched_policy == SCHED_NPREEMPT_SJF)
      calculate_estimate(p, xticks);
    
  }

  // Jump into the scheduler, never to return.
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(uint64 addr)
{
  struct proc *np;
  int havekids, pid;
  struct proc *p = myproc();

  acquire(&wait_lock);

  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(np = proc; np < &proc[NPROC]; np++){
      if(np->parent == p){
        // make sure the child isn't still in exit() or swtch().
        acquire(&np->lock);

        havekids = 1;
        if(np->state == ZOMBIE){
          // Found one.
          pid = np->pid;
          if(addr != 0 && copyout(p->pagetable, addr, (char *)&np->xstate,
                                  sizeof(np->xstate)) < 0) {
            release(&np->lock);
            release(&wait_lock);
            return -1;
          }
          freeproc(np);
          release(&np->lock);
          release(&wait_lock);
          return pid;
        }
        release(&np->lock);
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || p->killed){
      release(&wait_lock);
      return -1;
    }
    
    // Wait for a child to exit.
    sleep(p, &wait_lock);  //DOC: wait-sleep
  }
}

int
waitpid(int pid, uint64 addr)
{
  struct proc *np;
  struct proc *p = myproc();
  int found=0;

  acquire(&wait_lock);

  for(;;){
    // Scan through table looking for child with pid
    for(np = proc; np < &proc[NPROC]; np++){
      if((np->parent == p) && (np->pid == pid)){
	found = 1;
        // make sure the child isn't still in exit() or swtch().
        acquire(&np->lock);

        if(np->state == ZOMBIE){
           if(addr != 0 && copyout(p->pagetable, addr, (char *)&np->xstate,
                                  sizeof(np->xstate)) < 0) {
             release(&np->lock);
             release(&wait_lock);
             return -1;
           }
           freeproc(np);
           release(&np->lock);
           release(&wait_lock);
           return pid;
	}

        release(&np->lock);
      }
    }

    // No point waiting if we don't have any children.
    if(!found || p->killed){
      release(&wait_lock);
      return -1;
    }

    // Wait for a child to exit.
    sleep(p, &wait_lock);  //DOC: wait-sleep
  }
}

struct proc*
sjf()
{
  struct proc* p, *sched_proc;
  sched_proc = 0;
  for(;;) {
    for(p = proc; p < &proc[NPROC]; p++) {
      acquire(&p->lock);
      if(p->state != RUNNABLE){
        release(&p->lock);
        continue;
      }
      if((!p->special)){ // If we come across RUNNABLE non batch process, schedule it.
        release(&p->lock);
        return p;
      }
      if(p->special) {
        if(sched_proc == 0){
          sched_proc = p;
        }
        else if((p->nextest <= sched_proc->nextest)){
          if((p->nextest  < sched_proc->nextest ) || (p->proc_wait_time > sched_proc->proc_wait_time)){
            sched_proc = p;
          }   
        }
      }    
      release(&p->lock);
    }

    if(sched_proc != 0)
      break;
  }
  
  return sched_proc;
}


struct proc*
unix_scheduling()
{
  struct proc* p, *sched_proc;
  sched_proc = 0;
  p=0;
  int flag = 0;
  for(;;) {

    for(p = proc; p < &proc[NPROC]; p++) {
      acquire(&p->lock);
      if(p->state != RUNNABLE){
        release(&p->lock);
        continue;
      }

      if(!(p->special) && !flag){ // If we come across RUNNABLE non batch process, schedule it.
        sched_proc = p;
        flag = 1;
      }

      else if(p->special){
        
        p->cpu_usage /= 2;
        p->priority = p->base_priority + (p->cpu_usage/2);

        if(sched_proc == 0){
          sched_proc = p;
        }else if(!flag && (p->priority <= sched_proc->priority)){
          if((p->priority < sched_proc->priority) || (p->proc_wait_time > sched_proc->proc_wait_time)){
            sched_proc = p;
          }   
        }
      }    
      release(&p->lock);
    }

    if(sched_proc != 0)
      break;
  }
  
  return sched_proc;
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p, *selected_proc ;
  struct cpu *c = mycpu();
  
  c->proc = 0;

  for(;;){
    // Avoid deadlock by ensuring that devices can interrupt.
    intr_on();

    for(p = proc; p < &proc[NPROC]; p++) {

      // Print stats if all batch processes are finished
      if(CURRENT_BATCH_SIZE == 0 && BATCH_SIZE !=0) 
      { 
        // Once the entire batch is scheduled set the default policy to Round Robin again
        schedpolicy(SCHED_PREEMPT_RR);
        print_batch_stats();
      }

      selected_proc = 0;
      if(current_sched_policy == SCHED_PREEMPT_UNIX)
      {
          selected_proc = unix_scheduling();
          p = proc; // set p to first proc if policy changes to RR in between
      } else if(current_sched_policy == SCHED_NPREEMPT_SJF){
          p = proc; // set p to first proc 
          selected_proc = sjf();
      } else {
           selected_proc = p;
      }

      acquire(&selected_proc->lock);

      if(selected_proc->state == RUNNABLE ) {
        
        // Switch to chosen process.  It is the process's job
        // to release its lock and then reacquire it
        // before jumping back to us.
        selected_proc->state = RUNNING;
        c->proc = selected_proc;

        if(selected_proc->special){
          uint xticks;
          if (!holding(&tickslock)) {
            acquire(&tickslock);
            xticks = ticks;
            release(&tickslock);
          }
          else xticks = ticks;

          // Update the average waiting time.
          selected_proc->proc_wait_time += xticks - selected_proc->wtime;
          stats.avg_waiting_time += xticks - selected_proc->wtime;

          if(current_sched_policy == SCHED_NPREEMPT_SJF)
            selected_proc->sticks = xticks;
        }

        swtch(&c->context, &selected_proc->context);

        // Process is done running for now.
        // It should have changed its p->state before coming back.
        c->proc = 0;
      }
      release(&selected_proc->lock);
    }
  }
}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&p->lock))
    panic("sched p->lock");
  if(mycpu()->noff != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(intr_get())
    panic("sched interruptible");

  intena = mycpu()->intena;
  swtch(&p->context, &mycpu()->context);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  uint xticks;
  struct proc *p = myproc();
  acquire(&p->lock);
  p->state = RUNNABLE;

  if(p->special){

    if(!holding(&tickslock)) {
      acquire(&tickslock);
      xticks = ticks;
      release(&tickslock);
    }
    else xticks = ticks;

    p->wtime = xticks;
    if(current_sched_policy == SCHED_NPREEMPT_SJF){
      calculate_estimate(p, xticks);
    } else if(current_sched_policy == SCHED_PREEMPT_UNIX){
      p->cpu_usage += SCHED_PARAM_CPU_USAGE;
    }
  } 

  sched();
  release(&p->lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch to forkret.
void
forkret(void)
{
  static int first = 1;
  uint xticks;

  // Still holding p->lock from scheduler.
  release(&myproc()->lock);

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);

  myproc()->stime = xticks;

  // If the process is the first process to be scheduled in the batch for the first time
  // batch_exec_time is set as its stime.
  if((myproc()->special == 1) && stats.first_proc_in_batch == 0){
    stats.first_proc_in_batch = 1;
    stats.batch_exec_time = xticks;
  }

  if (first) {
    // File system initialization must be run in the context of a
    // regular process (e.g., because it calls sleep), and thus cannot
    // be run from main().
    first = 0;
    fsinit(ROOTDEV);
  }

  usertrapret();
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  // Must acquire p->lock in order to
  // change p->state and then call sched.
  // Once we hold p->lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup locks p->lock),
  // so it's okay to release lk.

  acquire(&p->lock);  //DOC: sleeplock1
  release(lk);

  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  if(p->special)
  { 
    if(current_sched_policy == SCHED_NPREEMPT_SJF){
      calculate_estimate(p, 0);
    } else if(current_sched_policy == SCHED_PREEMPT_UNIX){
      p->cpu_usage += SCHED_PARAM_CPU_USAGE/2;
    }
  }
  
  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  release(&p->lock);
  acquire(lk);
}

// Wake up all processes sleeping on chan.
// Must be called without any p->lock.
void
wakeup(void *chan)
{
  struct proc *p;

  for(p = proc; p < &proc[NPROC]; p++) {
    if(p != myproc()){
      acquire(&p->lock);
      if(p->state == SLEEPING && p->chan == chan) {
        p->state = RUNNABLE;
        if(p->special){
            if (!holding(&tickslock)) {
              acquire(&tickslock);
              p->wtime = ticks;
              release(&tickslock);
            }
            else p->wtime = ticks;
        }
        
      }
      release(&p->lock);
    }
  }
}

// Kill the process with the given pid.
// The victim won't exit until it tries to return
// to user space (see usertrap() in trap.c).
int
kill(int pid)
{
  struct proc *p;

  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->pid == pid){
      p->killed = 1;
      if(p->state == SLEEPING){
        // Wake process from sleep().
        p->state = RUNNABLE;
        if(p->special){
            if (!holding(&tickslock)) {
              acquire(&tickslock);
              p->wtime = ticks;
              release(&tickslock);
            }
            else p->wtime = ticks;
        }
      }
      release(&p->lock);
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}

// Copy to either a user address, or kernel address,
// depending on usr_dst.
// Returns 0 on success, -1 on error.
int
either_copyout(int user_dst, uint64 dst, void *src, uint64 len)
{
  struct proc *p = myproc();
  if(user_dst){
    return copyout(p->pagetable, dst, src, len);
  } else {
    memmove((char *)dst, src, len);
    return 0;
  }
}

// Copy from either a user address, or kernel address,
// depending on usr_src.
// Returns 0 on success, -1 on error.
int
either_copyin(void *dst, int user_src, uint64 src, uint64 len)
{
  struct proc *p = myproc();
  if(user_src){
    return copyin(p->pagetable, dst, src, len);
  } else {
    memmove(dst, (char*)src, len);
    return 0;
  }
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  struct proc *p;
  char *state;

  printf("\n");
  for(p = proc; p < &proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    printf("%d %s %s", p->pid, state, p->name);
    printf("\n");
  }
}

// Print a process listing to console with proper locks held.
// Caution: don't invoke too often; can slow down the machine.
int
ps(void)
{
   static char *states[] = {
  [UNUSED]    "unused",
  [SLEEPING]  "sleep",
  [RUNNABLE]  "runble",
  [RUNNING]   "run",
  [ZOMBIE]    "zombie"
  };
  struct proc *p;
  char *state;
  int ppid, pid;
  uint xticks;

  printf("\n");
  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->state == UNUSED) {
      release(&p->lock);
      continue;
    }
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";

    pid = p->pid;
    release(&p->lock);
    acquire(&wait_lock);
    if (p->parent) {
       acquire(&p->parent->lock);
       ppid = p->parent->pid;
       release(&p->parent->lock);
    }
    else ppid = -1;
    release(&wait_lock);

    acquire(&tickslock);
    xticks = ticks;
    release(&tickslock);

    printf("pid=%d, ppid=%d, state=%s, cmd=%s, ctime=%d, stime=%d, etime=%d, size=%p", pid, ppid, state, p->name, p->ctime, p->stime, (p->endtime == -1) ? xticks-p->stime : p->endtime-p->stime, p->sz);
    printf("\n");
  }
  return 0;
}

int
pinfo(int pid, uint64 addr)
{
   struct procstat pstat;

   static char *states[] = {
  [UNUSED]    "unused",
  [SLEEPING]  "sleep",
  [RUNNABLE]  "runble",
  [RUNNING]   "run",
  [ZOMBIE]    "zombie"
  };
  struct proc *p;
  char *state;
  uint xticks;
  int found=0;

  if (pid == -1) {
     p = myproc();
     acquire(&p->lock);
     found=1;
  }
  else {
     for(p = proc; p < &proc[NPROC]; p++){
       acquire(&p->lock);
       if((p->state == UNUSED) || (p->pid != pid)) {
         release(&p->lock);
         continue;
       }
       else {
         found=1;
         break;
       }
     }
  }
  if (found) {
     if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
         state = states[p->state];
     else
         state = "???";

     pstat.pid = p->pid;
     release(&p->lock);
     acquire(&wait_lock);
     if (p->parent) {
        acquire(&p->parent->lock);
        pstat.ppid = p->parent->pid;
        release(&p->parent->lock);
     }
     else pstat.ppid = -1;
     release(&wait_lock);

     acquire(&tickslock);
     xticks = ticks;
     release(&tickslock);

     safestrcpy(&pstat.state[0], state, strlen(state)+1);
     safestrcpy(&pstat.command[0], &p->name[0], sizeof(p->name));
     pstat.ctime = p->ctime;
     pstat.stime = p->stime;
     pstat.etime = (p->endtime == -1) ? xticks-p->stime : p->endtime-p->stime;
     pstat.size = p->sz;
     if(copyout(myproc()->pagetable, addr, (char *)&pstat, sizeof(pstat)) < 0) return -1;
     return 0;
  }
  else return -1;
}

int
schedpolicy(int new_policy)
{
  int old_policy;

  old_policy = current_sched_policy;
  current_sched_policy = new_policy;
  return old_policy;
}


int
forkp(int base_priority)
{
  int i, pid;
  struct proc *np;
  struct proc *p = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy user memory from parent to child.
  if(uvmcopy(p->pagetable, np->pagetable, p->sz) < 0){
    freeproc(np);
    release(&np->lock);
    return -1;
  }
  np->sz = p->sz;

  // copy saved user registers.
  *(np->trapframe) = *(p->trapframe);

  // Cause fork to return 0 in the child.
  np->trapframe->a0 = 0;
  np->base_priority = base_priority;
  np->special = 1;
  np->nextest = 0;
  np->cpu_usage = 0;
  np->priority = base_priority;
  np->proc_wait_time = 0;
  // increment reference counts on open file descriptors.
  for(i = 0; i < NOFILE; i++)
    if(p->ofile[i])
      np->ofile[i] = filedup(p->ofile[i]);
  np->cwd = idup(p->cwd);

  safestrcpy(np->name, p->name, sizeof(p->name));

  pid = np->pid;

  release(&np->lock);

  acquire(&wait_lock);
  np->parent = p;
  release(&wait_lock);

  acquire(&np->lock);
  np->state = RUNNABLE;
  if (!holding(&tickslock)) {
    acquire(&tickslock);
    np->wtime = ticks;
    release(&tickslock);
  }
  else np->wtime = ticks;
  release(&np->lock);

  BATCH_SIZE += 1;
  CURRENT_BATCH_SIZE += 1;

  return pid;
}

void
calculate_estimate(struct proc* p, uint xticks) {

  if(xticks == 0){
    if (!holding(&tickslock)) {
      acquire(&tickslock);
      xticks = ticks;
      release(&tickslock);
    }
    else xticks = ticks;
  }

  if(p->nextest > 0) {
    stats.count_est_cpu_burst += 1;
    stats.avg_cpu_burst_est += p->nextest;

    if(p->nextest > stats.max_cpu_burst_est) {
      stats.max_cpu_burst_est = p->nextest;
    }

    if(p->nextest < stats.min_cpu_burst_est) {
      stats.min_cpu_burst_est = p->nextest;
    }
  }

  int t = xticks - p->sticks;

  if(p->nextest > 0 && t > 0){
    stats.est_error_count += 1;

    if(p->nextest > t) {
      stats.est_error_avg +=  (p->nextest - t);
    } else {
      stats.est_error_avg +=  (t - p->nextest);
    }
  }

  if(t > 0){
    
      stats.count_cpu_burst += 1;
      stats.avg_cpu_burst += t;
      if(t > stats.max_cpu_burst) {
        stats.max_cpu_burst = t;
      }

      if(t < stats.min_cpu_burst) {
        stats.min_cpu_burst = t;
      }

      p->nextest = t - (SCHED_PARAM_SJF_A_NUMER*t)/SCHED_PARAM_SJF_A_DENOM + (SCHED_PARAM_SJF_A_NUMER*(p->nextest))/(SCHED_PARAM_SJF_A_DENOM);
  }
  return;

}

void 
print_batch_stats()
{ 
  printf("\n");
  printf("Batch execution time: %d\n", stats.batch_exec_time);
  //printf("Current Batch Size: %d, Batch Size: %d\n", CURRENT_BATCH_SIZE, BATCH_SIZE);
  printf("Average turn-around time: %d\n", stats.avg_turnaround_time/BATCH_SIZE);
  printf("Average waiting time: %d\n", stats.avg_waiting_time/BATCH_SIZE);
  printf("Completion time: avg: %d, max: %d, min: %d \n", stats.avg_completion_time/BATCH_SIZE
                                                        , stats.max_completion_time
                                                        , stats.min_completion_time);
 
 if(current_sched_policy == SCHED_NPREEMPT_SJF){
    printf("CPU bursts: count: %d, avg: %d, max: %d, min: %d\n", stats.count_cpu_burst
                                                             , stats.avg_cpu_burst/stats.count_cpu_burst
                                                             , stats.max_cpu_burst
                                                             , stats.min_cpu_burst);
    printf("CPU burst estimates: count: %d, avg: %d, max: %d, min: %d\n", stats.count_est_cpu_burst
                                                                      , stats.avg_cpu_burst_est/stats.count_est_cpu_burst
                                                                      , stats.max_cpu_burst_est
                                                                      , stats.min_cpu_burst_est);
    printf("CPU burst estimation error: count: %d, avg: %d\n", stats.est_error_count
                                                           , stats.est_error_avg/stats.est_error_count);

 }
 
  // reset all the stats variable for the next batch
  stats.batch_exec_time = 0;
  stats.first_proc_in_batch = 0;
  stats.avg_turnaround_time = 0;
  stats.avg_waiting_time = 0;

  stats.max_completion_time = 0;
  stats.min_completion_time = 1000000000;
  stats.avg_completion_time = 0;

  stats.count_cpu_burst = 0;

  stats.avg_cpu_burst = 0;
  stats.max_cpu_burst = 0;
  stats.min_cpu_burst = 1000000000;

  stats.count_est_cpu_burst = 0;
  stats.avg_cpu_burst_est = 0;
  stats.max_cpu_burst_est = 0;
  stats.min_cpu_burst_est = 1000000000;

  stats.est_error_count = 0;
  stats.est_error_avg = 0;
  BATCH_SIZE = 0;
  CURRENT_BATCH_SIZE = 0;
}