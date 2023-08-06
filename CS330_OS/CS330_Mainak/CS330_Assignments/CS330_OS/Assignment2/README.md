In this assignment, you will experiment with a few scheduling algorithms in xv6. Before you
start the assignment, you will incorporate a couple of system calls that will help you submit
a batch of jobs to xv6.

Download ```assignment2.zip``` and unzip it in your home directory. This will create a new directory
named assignment2/ and place the xv6 code for this assignment under it. This code includes the
solutions of the first assignment and many more things. Since we will be experimenting with
single-CPU schedulers only, please replace ```CPUS := 3``` by ```CPUS := 1``` in assignment2/xv6-riscv/Makefile.

-------------------------
BATCH SUBMISSION OF JOBS
-------------------------

I have included a user program submitjobs.c in the assignment2/xv6-riscv/user/ directory. You will be
using this program to submit a batch of jobs. The program is to be run as submitjobs < batchfilename.txt
from the xv6 shell prompt. The batch file has a specific format: the first line specifies the scheduling
algorithm as an integer code and each subsequent line represents a process. Each process line starts with a
positive integer specifying its base priority value and next it specifies the command to run the process. See
user/batch.txt as an example. All batch files must be listed in assignment2/xv6-riscv/Makefile under
JOBFILES. Currently, you will see a few batch files already listed. Any new batch file that you add must
be added to this list. We will be experimenting with four scheduling algorithms in this assignment:
(i) non-preemptive FCFS (code 0), (ii) non-preemptive SJF (code 1), (iii) preemptive round-robin (code 2),
and (iv) preemptive UNIX scheduler (code 3). The base priority value of a process is used only in the last
scheduler.

Go over the program submitjobs.c and understand what it is doing. Currently, the program will not compile
when you run make qemu because the program uses two wrapper functions that are not implemented yet:
schedpolicy() and forkp(). The schedpolicy() function is used to pass the scheduling policy to the xv6 kernel.
This should be implemented as a new system call named schedpolicy. For convenience, you can make this system
call return the current scheduling policy back to the program. This return value can be used to change the
scheduling policy back to the original later. The submitjobs.c program does not make use of the return value of
schedpolicy(). The forkp() function is same as the fork() function except that the former passes the base
priority value of the process being created. This should also be implemented as a new system call named forkp
to create a new process and store its base priority value in its process table entry. Your first task is to
implement these two system calls and make submitjobs work. This is the prerequisite for starting the assignment.

--------------------------------
UNDERSTANDING THE xv6 SCHEDULER
--------------------------------

The xv6 machine boots up using a preemptive round-robin scheduler (the scheduler with code 2 above). The xv6
scheduler is implemented through two functions scheduler() and sched() both implemented in
assignment2/xv6-riscv/kernel/proc.c file. The main function of xv6 in assignment2/xv6-riscv/kernel/main.c
calls scheduler() to get the ball rolling. When this function is called for the first time, the CPU context
contains the context of the execution up to that point. This context will be referred to as the "FIRST" context.
The scheduler() function picks a process round-robin from among all the processes in the RUNNABLE state
and switches its context with the current CPU context. So, the first call to scheduler will save the FIRST
context and load the context of the first selected process. From then on, a process may undergo a context
switch whenever it switches from RUNNING state to RUNNABLE, SLEEPING, or ZOMBIE. This happens in different
functions of the assignment2/xv6-riscv/kernel/proc.c file and it will be helpful to find out the places where
such a state transition takes place. When such a transition happens, the sched() function is called. For
example, a process switches from RUNNING to RUNNABLE state in the yield() function of assignment2/xv6-riscv/kernel/proc.c
file. The yield() function is called either by the yield system call from the user program or by the timer
interrupt from the usertrap() and kerneltrap() functions in assignment2/xv6-riscv/kernel/trap.c. Similarly,
a process switches from RUNNING to SLEEPING state in the sleep() function of assignment2/xv6-riscv/kernel/proc.c.
In all these cases, the sched() function is called. The sched() function simply switches the context between the
currently running process and the saved FIRST context. This makes the FIRST context return from the previous switch
call of the scheduler function and it continues to execute the process selection loop of the scheduler() function.
That's how the next process to run gets selected and its context is switched with the FIRST context. Notice
that the selected process must hold the process lock when it reaches the switch call. Few things to note:
(i) The scheduler() function always runs on the FIRST context. (ii) A process is switched out in the switch
call of the sched() function and therefore, the next time it is scheduled, it will start running from there and
the first thing it does after returning from sched() is to release its lock. That's how the process lock gets released
before it starts running and it will re-acquire the lock before it calls sched() next time. Notice that before
every call to sched(), the process lock is acquired and the state is changed to RUNNABLE making the currently
running process also eligible for scheduling again. (iii) The process lock is acquired from the time its state is
compared against RUNNABLE all the way to the time it is scheduled. The lock cannot be released in-between
because then its state may change to something other than RUNNABLE.

----------------------
SCHEDULING ALGORITHMS
----------------------

As already mentioned, you will be experimenting with four scheduling algorithms. It will be a good idea
to give some names to these algorithms and define their corresponding integer codes using #define in
assignment2/xv6-riscv/kernel/param.h so that you don't have to hardcode the integer codes in your xv6
code. For example, you can include the following lines in assignment2/xv6-riscv/kernel/param.h.

#define SCHED_NPREEMPT_FCFS 0
#define SCHED_NPREEMPT_SJF 1
#define SCHED_PREEMPT_RR 2
#define SCHED_PREEMPT_UNIX 3

I discuss each one of these algorithms in the following.

SCHED_NPREEMPT_FCFS: You get this algorithm in xv6 by not calling yield() on a timer interrupt in both
usertrap() and kerneltrap() functions. So, timer interrupt induced preemption will be disabled. However,
note that if the process goes to SLEEPING state or calls yield() through the yield system call, it will
be switched out. It will become eligible for scheduling again when it switches back to the RUNNABLE state.

SCHED_PREEMPT_RR: This is the default scheduler of xv6.

SCHED_NPREEMPT_SJF: This algorithm uses the exponential averaging-based CPU burst length estimation for
carrying out non-preemptive SJF scheduling. Since it is a non-preemptive scheduler, it should not call
yield() on timer interrupts. In a non-preemptive scheduler, a CPU burst of a process starts when the
process gets scheduled i.e., it state changes to RUNNING and the CPU burst comes to an end when the
process state changes to RUNNABLE, SLEEPING, or ZOMBIE. The RUNNING to RUNNABLE transition can happen only if
the process calls yield() through the yield system call. The RUNNING to SLEEPING transition can happen
if the process calls sleep(). The RUNNING to ZOMBIE transition can happen if the process exits. Whenever
a CPU burst ends, you need to do two things provided the just concluded CPU burst length is non-zero: 
(i) Compute the length of the just concluded CPU burst. We will be computing CPU burst length in terms of
ticks. To get the current ticks, use the following code snippet. The current ticks will be in the variable
xticks. The difference between ticks when a CPU burst ends and when it started is the CPU burst length.

uint xticks;
if (!holding(&tickslock)) {
   acquire(&tickslock);
   xticks = ticks;
   release(&tickslock);
}
else xticks = ticks;

(ii) Generate an estimate for the next CPU burst length and store it in the process table entry to be used
later by the scheduler. To carry out this estimate, you will need the previous estimate and the length of the
just concluded CPU burst. You initialize the estimate to zero when a process is created (this is s(0)).
You also need to fix the parameter "a". For this assignment, you will use a=1/2. One problem with the RISC-V
virtual machine that we are using is that it doesn't enable floating-point operations. So, if a=p/q, you need
to do the calculation of s(n+1) = (1-a)*t(n) + a*s(n) as s(n+1) = t(n) - (p*t(n))/q + (p*s(n))/q. You should
define p and q in assignment2/xv6-riscv/kernel/param.h instead of hard-coding them. For example, when a=1/2,
you can include the following two lines in assignment2/xv6-riscv/kernel/param.h.

#define SCHED_PARAM_SJF_A_NUMER 1  // p
#define SCHED_PARAM_SJF_A_DENOM 2  // q

You should do these two calculations only for the processes that are part of a batch and created through the forkp()
call. To be able to identify the batch processes, you may include a variable in the process table entry to remember
whether a process is created using forkp. This variable can be set to one in forkp(), but to zero in fork() and forkf().

The SCHED_NPREEMPT_SJF scheduler just picks the RUNNABLE process that has the smallest next CPU burst estimate.
However, if it comes across a RUNNABLE process that is not a batch process, that should be selected for scheduling
immediately. This ensures that the main process can run to completion quickly and create all batch processes.

SCHED_PREEMPT_UNIX: This scheduler implements a subset of the UNIX scheduler. Specifically, it maintains a CPU
usage and a dynamic priority for each process in the process table entry. Whenever a process transitions from
RUNNING to RUNNABLE or SLEEPING, it updates its CPU usage and dynamic priority. In this assignment, we will
assume that the CPU usage of a process increases by 200 if it transitions from RUNNING to RUNNABLE; otherwise
its CPU usage increases by 100 indicating that it went to sleep before expiry of the full quantum. Instead
of hardcoding the CPU usage values, you may include the following line in assignment2/xv6-riscv/kernel/param.h.

#define SCHED_PARAM_CPU_USAGE 200

Increment CPU usage by SCHED_PARAM_CPU_USAGE or by SCHED_PARAM_CPU_USAGE/2 depending on whether the state
transitions to RUNNABLE or SLEEPING respectively.

The scheduler first goes over all RUNNABLE processes and updates their CPU usage and dynamic priority as follows.

CPU usage = (CPU usage)/2;
priority = (base priority) + (CPU usage)/2;

Recall that the base priority is stored in the process table entry as part of the forkp() call. Next, the
scheduler picks the process with the minimum priority value for scheduling. However, if it comes across a RUNNABLE
process that is not a batch process, that should be selected for scheduling immediately. This ensures that the main
process can run to completion quickly and create all batch processes.

Note: xv6 boots up with the SCHED_PREEMPT_RR as the scheduler, but the scheduler can be changed by the user
program by calling schedpolicy(). So, in every iteration of the default scheduler's inner loop (which
goes over the RUNNABLE processes), it should be checked whether the scheduling policy has changed. If it has,
the scheduler should break out of this loop and execute the next iteration of the outer loop (the infinite loop)
and the code structure should allow it to choose the correct scheduling algorithm. You need to maintain a global
variable for recording the current scheduling policy. It should be initialized to SCHED_PREEMPT_RR in the
main function of assignment2/xv6-riscv/kernel/main.c.

----------------------
SCHEDULING QUANTUM
----------------------

For the preemptive scheduling algorithms, we need to fix the scheduling quantum which is same as the timer
interval. This is set in the timerinit() function of the assignment2/xv6-riscv/kernel/start.c file. Currently,
it is hardcoded as 1000000 cycles (see the line int interval = 1000000;). We will use a timer interval
of 100000 cycles in this assignment. I suggest that you include the following line in assignment2/xv6-riscv/kernel/param.h.

#define TIMER_INTERVAL 100000

Change the line "int interval = 1000000;" in timerinit() function to the following.

int interval = TIMER_INTERVAL;

--------------------------------
PRINTING STATISTICS FOR A BATCH
--------------------------------

After a batch of processes completes execution, the following statistics should be printed by xv6.

1. Batch execution time: This is the execution time of the entire batch. This is measured as the
number of ticks from the stime of the batch process that got scheduled first to the endtime of the
batch process to finish last. To know when a batch finishes, you may have to maintain the batch size
as observed from the forkp() calls (increment batch size by one in forkp()).

2. Average turnaround time of the batch: For each finishing batch process, compute its turnaround time
and maintain a running sum of the turnaround times of the batch processes. Take an average after all
batch processes have finished.

3. Average waiting time in RUNNABLE state: Whenever a batch process enters the RUNNABLE state, start
counting waiting time. When it comes out of RUNNABLE state and moves to RUNNING state, end counting
waiting time. Accumulate all waiting time instances for a batch process. Maintain a running sum of
the waiting times of the finished batch processes. Take an average after all batch processes have finished.

4. Average, minimum, and maximum completion time of the processes in a batch. A large difference between
the maximum and minimum completion times relative to the average is an indication of lack of fairness
meaning that some process in the batch finishes very quickly while some other process finishes quite late.

5. For the SCHED_NPREEMPT_SJF scheduler, print the total number of non-zero CPU bursts observed
across all processes in a batch, average CPU burst length observed in a batch, maximum CPU burst length
observed in a batch, and minimum CPU burst length observed in a batch. Also, print the same statistics for
the estimated non-zero CPU burst lengths. Additionally, compute the absolute difference between the estimated
burst length and the actual burst length provided both are non-zero, maintain a running sum of these
estimation errors, and print an average per estimation instance.

Make sure to reset all statistics counters at the end of a batch so that the statistics of the next executed
batch are printed correctly.

A sample output is shown below.

Batch execution time: 7643
Average turn-around time: 4911
Average waiting time: 4842
Completion time: avg: 6620, max: 8658, min: 4582
CPU bursts: count: 56, avg: 136, max: 204, min: 1
CPU burst estimates: count: 56, avg: 117, max: 197, min: 50
CPU burst estimation error: count: 46, avg: 50

---------------------------------
EVALUATING SCHEDULING ALGORITHMS
---------------------------------

I have included several test cases for evaluating the scheduling algorithms. The programs that we will
use to form the batches are testloop1.c, testloop2.c, testloop3.c, testloop4.c, and testlooplong.c.
These can be found in the assignment2/xv6-riscv/user/ directory. The testloop1.c program has large
CPU bursts interspersed with small I/O bursts (just sleep(1) calls). The testloop2.c program uses
yield() instead of sleep() to end each CPU burst. The testloop3.c program uses shorter CPU bursts than
testloop2.c. The testloop4.c program has no I/O burst at all except one at the end. The testlooplong.c
program has a reasonably large number of CPU bursts interspersed with yield() calls. I have included
seven batch files: batch1.txt, batch2.txt, batch3.txt, batch4.txt, batch5.txt, and batch7.txt. You may
have to change the scheduling policies in these batch files to evaluate different scheduling policies
on these batches. You need to conduct the following evaluations.

1. Comparison between non-preemptive FCFS and preemptive round-robin:

(a) Evaluate batch1.txt for SCHED_NPREEMPT_FCFS and SCHED_PREEMPT_RR. Report the differences in statistics.
Explain the observation.

(b) Evaluate batch2.txt for SCHED_NPREEMPT_FCFS and SCHED_PREEMPT_RR. Report the differences in statistics.
Explain the observation.

(c) Evaluate batch7.txt for SCHED_NPREEMPT_FCFS and SCHED_PREEMPT_RR. Report the differences in statistics.
Explain the observation.

2. CPU burst estimation error using exponential averaging:

Evaluate batch2.txt and batch3.txt for SCHED_NPREEMPT_SJF and report any observed differences in the CPU
burst estimation error. You should be paying attention to how the following ratio changes:
(the average CPU burst estimation error per estimation instance)/(the average CPU burst length).
Explain what you observe.

3. Comparison between non-preemptive FCFS and non-preemptive SJF:

Evaluate batch4.txt for SCHED_NPREEMPT_FCFS and SCHED_NPREEMPT_SJF. Report the differences in statistics.
Explain the observation.

4. Comparison between preemptive round-robin and preemptive UNIX:

(a) Evaluate batch5.txt for SCHED_PREEMPT_RR and SCHED_PREEMPT_UNIX. Report the differences in statistics.
Explain the observation.

(b) Evaluate batch6.txt for SCHED_PREEMPT_RR and SCHED_PREEMPT_UNIX. Report the differences in statistics.
Explain the observation.

-----------
SUBMISSION
-----------

Before you submit, execute "make clean" without double quotes from directory assignment2/xv6-riscv.
Send one submission email per group to cs330autumn2022@gmail.com. The submission email's
subject should be "Assignment#2 of Group XX" without double quotes where XX should be replaced
by your group number. Attach the following to the email.

1. Create a zip ball of your user/ and kernel/ directories as follows from the xv6-riscv directory (replace
XX by your group number).

zip -r GroupXX.zip user/ kernel/

Attach GroupXX.zip to the email.

2. Prepare a PDF report describing briefly the implementation of the SCHED_NPREEMPT_SJF and SCHED_PREEMPT_UNIX
schedulers. Include the observations and explanations that are sought regarding the given experiments in the
evaluation section. Attach the report with the submission email. The only acceptable format for the report is PDF.