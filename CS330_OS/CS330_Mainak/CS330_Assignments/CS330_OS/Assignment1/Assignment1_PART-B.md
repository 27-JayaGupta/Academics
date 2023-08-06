In this part of the assignment, you will add seven new system calls. Before you start,
it will be helpful to take a look at how the existing system calls are implemented. For
example, if you trace the execution of the getpid system call, you will see the following.
The syscall number for getpid is defined in kernel/syscall.h. The wrapper function
getpid() exposed to the users is defined in user/usys.S. However, you should never edit
user/usys.S. You only need to make an entry in user/usys.pl and usys.S containing all
wrapper functions is automatically generated from that. The wrapper function prototype
is declared in user/user.h. Once the wrapper function is called by a user program, the
ecall instruction will guide the execution through the trapping mechanism ultimately
landing in the usertrap() function of kernel/trap.c. This function figures out the
cause of the trap and if it is due to a system call, the syscall() function is called.
This function is defined in kernel/syscall.c. This function uses the syscall number
(from trapframe->a7) to index into the syscall function table which is also
defined in kernel/syscall.c. Notice that the SYS_getpid index of this table is
populated with the sys_getpid function. Therefore, sys_getpid function gets called
next. This function is declared in kernel/syscall.c and defined in kernel/sysproc.c.
When sys_getpid() returns, control comes back to syscall(), which returns to usertrap().
Finally, usertrap() calls usertrapret(), which is defined in trap.c. The
usertrapret() function executes the return from trap code and ultimately executes
sret to switch mode that brings you back to the instruction right after the ecall
instruction in the getpid() wrapper function, which returns back to the user program.

Similarly, you can trace the execution of a syscall that takes arguments, e.g., the
sleep system call. You will notice that complex system calls that need access to the
process table are usually implemented in a function in kernel/proc.c and that function
is called from a simpler function in kernel/sysproc.c. In general, all syscall
functions enter through a sys_* function in kernel/sysproc.c. Also, as a general
implementation rule, if the name of a syscall is "abcd", its wrapper function is named
"abcd", its syscall number is defined as SYS_abcd and it is entered through the
sys_abcd function.

If a system call needs to copy certain pieces of information from the kernel address
space to the user address space, the copyout function should be used. For example,
have a look at how the wait system call fills the pointer passed as its argument.

Before you get started, read Chapter 2 and upto Section 4.4 from Chapter 4 of the xv6
RISC-V book posted on the course webpage.

You need to implement the following system calls.

1. ```getppid```: Returns the pid of the parent of the calling process. Takes no argument.
If the calling process has no parent (for whatever reason), it returns -1. [3 points]

2. ```yield```: The calling process is de-scheduled i.e., it voluntarily yields the CPU to
other processes. Take help of the yield() function defined in kernel/proc.c. Returns
zero always. Takes no argument. [2 points]

3. ```getpa```: Takes a virtual address (i.e., a pointer) as the only arguments and returns
the corresponding physical address. For example, if x is a variable in the calling
program, getpa(&x) returns the physical address of x. Take help of the function
walkaddr() defined in kernel/vm.c. Given a virtual address A in a process p,
```walkaddr(p->pagetable, A) + (A & (PGSIZE - 1))``` computes the physical address of A. [4 points]

4. ```forkf```: This system call introduces a slight variation in the fork() call. The
usual fork() call returns to the code right after the fork() call in both parent and
child. In forkf() call, the parent behaves just like the usual fork() call, but
the child first executes a function right after returning to user mode and then returns
to the code after the forkf() call. The forkf system call takes the function address as
an argument. There is one requirement regarding the function which is passed as the argument
of forkf: the function must not have any argument. It will be helpful to understand how
the fork() call is implemented and from where the program counter is picked up when a trap
returns to user mode.

Consider the following example program that uses forkf().

```c
#include "kernel/types.h"
#include "user/user.h"

int g (int x)
{
   return x*x;
}

int f (void)
{
   int x = 10;

   fprintf(2, "Hello world! %d\n", g(x));
   return 0;
}

int
main(void)
{
  int x = forkf(f);
  if (x < 0) {
     fprintf(2, "Error: cannot fork\nAborting...\n");
     exit(0);
  }
  else if (x > 0) {
     sleep(1);
     fprintf(1, "%d: Parent.\n", getpid());
     wait(0);
  }
  else {
     fprintf(1, "%d: Child.\n", getpid());
  }

  exit(0);
}
```

The expected output of this program is shown below.

```c
Hello world! 100
4: Child.
3: Parent.
```

Explain the outputs of the program when the return value of f is 0, 1, and -1.
How does the program behave if the return value of f is changed to some integer
value other than 0, 1, and -1? How does the program behave if the return type
of f is changed to void and the return statement in f is commented? [18 points]

5. ```waitpid```: This system call takes two arguments: an integer and a pointer. The
integer argument can be a pid or -1. The system call waits for the process with
the passed pid to complete provided the pid is of a child of the calling process.
If the first argument is -1, the system call behaves similarly to the wait system
call. The normal return value of this system call is the pid of the process that
it has waited on. The system call returns -1 in the case of any error. It will be
helpful to see how the wait system call is implemented. Make sure to acquire and
release the appropriate locks as is done in the implementation of the wait system
call and as mentioned in the comments alongside the definition of the proc structure
in kernel/proc.h. [11 points]

6. ```ps```: This system call walks over the process table and prints the following pieces
of information for each process that has a state other than UNUSED: ```pid, parent pid,
state, command, creation time, start time, execution time, size```. For command, use
the name field in the proc structure. You need to introduce new fields in the proc
structure to record creation time of a process, start time of a process, and end time
of a process. For each of these, use the "ticks" variable to get the current time
as used in the sys_uptime() function in kernel/sysproc.c. The creation time of a process
is defined as the time when the process is allocated. The start time of a process
is defined as the time when the process is scheduled for the first time (recall that
a newly created process starts execution in a special kernel function; find out which
function it is). The end time of a process is defined as the time when the process
becomes zombie by calling exit(). When ps is called, it is possible that certain
processes have not yet terminated. For them, the execution time should be printed as
the current time minus the start time, while for those which have already terminated,
the execution time should be the end time minus the start time. Use the sz field of the
proc structure to get the size of a process. A sample output is shown below.

```c
pid=1, ppid=-1, state=sleep, cmd=init, ctime=0, stime=1, etime=101, size=0x0000000000003000
pid=2, ppid=1, state=sleep, cmd=sh, ctime=1, stime=1, etime=101, size=0x0000000000004000
pid=4, ppid=2, state=run, cmd=testps, ctime=92, stime=92, etime=10, size=0x0000000000003000
pid=5, ppid=4, state=zombie, cmd=testps, ctime=92, stime=92, etime=1, size=0x0000000000003000
```
Note that the init process has no parent process (because it is the root of the process
tree). Therefore, its ppid is printed as -1.

It will be helpful to see how the procdump() function is implemented in kernel/proc.c.
However, this function does not acquire/release any locks. Your implementation of ps
must acquire/release the proper locks as you have done in the waitpid implementation.

The return value of this system call is always zero. [6 points]

7. ```pinfo```: This system call is similar to ps, but instead of printing the information
about all processes, it returns the information about a specific process to the calling
program. This system call takes two arguments: an integer and a pointer to a procstat
structure. The procstat structure should be defined as shown below in a new file
```kernel/procstat.h```.

```c
struct procstat {
  int pid;     // Process ID
  int ppid;    // Parent ID
  char state[8];  // Process state
  char command[16]; // Process name
  int ctime;    // Creation time
  int stime;    // Start time
  int etime;    // Execution time
  uint64 size;  // Process size
};
```

This structure has the same pieces of information that the ps system call was printing. The
first argument of this system call can be a pid or -1. If it is a pid, then the system call
fills the second argument with the aforementioned pieces of information regarding the process having
the specified pid. If the first argument is -1, then the system call fills the second argument
with the aforementioned pieces of information regarding the calling process. Note that you
will need to include kernel/procstat.h in the user program that uses the pinfo system call.
Also, you will have to include kernel/procstat.h in ```kernel/proc.c```. 

The normal return value of this system call is zero. In case of any error (e.g., invalid pid
or invalid pointer, etc.), the return value should be -1. [8 points]

An example user program that uses pinfo is shown below.

```c
#include "kernel/types.h"
#include "kernel/procstat.h"
#include "user/user.h"

int
main(void)
{
  struct procstat pstat;

  int x = fork();
  if (x < 0) {
     fprintf(2, "Error: cannot fork\nAborting...\n");
     exit(0);
  }
  else if (x > 0) {
     sleep(1);
     fprintf(1, "%d: Parent.\n", getpid());
     if (pinfo(-1, &pstat) < 0) fprintf(1, "Cannot get pinfo\n");
     else fprintf(1, "pid=%d, ppid=%d, state=%s, cmd=%s, ctime=%d, stime=%d, etime=%d, size=%p\n",
         pstat.pid, pstat.ppid, pstat.state, pstat.command, pstat.ctime, pstat.stime, pstat.etime, pstat.size);
     if (pinfo(x, &pstat) < 0) fprintf(1, "Cannot get pinfo\n");
     else fprintf(1, "pid=%d, ppid=%d, state=%s, cmd=%s, ctime=%d, stime=%d, etime=%d, size=%p\n\n",
         pstat.pid, pstat.ppid, pstat.state, pstat.command, pstat.ctime, pstat.stime, pstat.etime, pstat.size);
     fprintf(1, "Return value of waitpid=%d\n", waitpid(x, 0));
  }
  else {
     fprintf(1, "%d: Child.\n", getpid());
     if (pinfo(-1, &pstat) < 0) fprintf(1, "Cannot get pinfo\n");
     else fprintf(1, "pid=%d, ppid=%d, state=%s, cmd=%s, ctime=%d, stime=%d, etime=%d, size=%p\n\n",
         pstat.pid, pstat.ppid, pstat.state, pstat.command, pstat.ctime, pstat.stime, pstat.etime, pstat.size);
  }

  exit(0);
}
```

The output is shown below.

```c
4: Child.
pid=4, ppid=3, state=run, cmd=testpinfo, ctime=31, stime=31, etime=0, size=0x0000000000003000

3: Parent.
pid=3, ppid=2, state=run, cmd=testpinfo, ctime=31, stime=31, etime=1, size=0x0000000000003000
pid=4, ppid=3, state=zombie, cmd=testpinfo, ctime=31, stime=31, etime=0, size=0x0000000000003000

Return value of waitpid=4
```

```General comments```: To test each implemented system call, feel free to write your own user programs.
Please do not submit these user programs. We will use our own set of test programs to grade your
submission.

```Submission```: Before you submit, execute "make clean" without double quotes from directory xv6-riscv.
Send one submission email per group to cs330autumn2022@gmail.com. The submission email's
subject should be "Assignment#1 of Group XX" without double quotes where XX should be replaced
by your group number. Attach the following to the email.

1. Four user program files from PART A. Name them exactly as mentioned in the assignment.

2. Move your test programs from the user/ directory to some other place. Create a zip ball of
your user/ and kernel/ directories as follows from the xv6-riscv directory (replace XX by your
group number).

```zip -r GroupXX.zip user/ kernel/```

Attach GroupXX.zip to the email.

3. Prepare a README file describing briefly implementation of each system call and the explanations that
are sought regarding the given test program that uses forkf(). Attach the README file with the
submission email. The only acceptable format for the README file is PDF. The README carries 8 points.