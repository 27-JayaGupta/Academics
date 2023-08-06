# Assignment 1
In the first part of this assignment, you will implement a few user programs
in the ```xv6``` OS environment. 

Step1: Machines, tools, etc.
----------------------------

```
sudo apt-get install git build-essential gdb-multiarch qemu-system-misc gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu
```

Step2: Unboxing xv6
--------------------

* Copy assignment1.zip to your space and unzip it using "unzip assignment1.zip" command without the double quotes. 
* This will create a directory named
assignment1 and under that you will see a directory named xv6-riscv. This
directory has the basic xv6 OS code. 
* Change directory to xv6-riscv using
"cd xv6-riscv" command without the double quotes. There are two main directories:
```kernel and user```. 
* The first one has the OS kernel code and the second one has
all user programs. 

In the first part of this assignment, you will be writing
a few new user programs using the system calls discussed in the class. The
```mkfs``` directory has the code to create the fg the boot code,ile system containin
the kernel code, and the user programs. 

To build the code and run the system, type the command:
```
make clean
make qemu
```

 This will compile the
OS kernel and the user programs, create the file system, and boot the virtual
machine with xv6 OS. At the end of this sequence, you will get the xv6 shell prompt,
which is a $. The last few lines are shown below.

```
xv6 kernel is booting

hart 2 starting
hart 1 starting
init: starting sh
$
```

xv6 has a limited number of user programs. You can type ```ls``` at the shell prompt to see which commands are available. The output of ls is
shown below.
```s
$ ls
.              1 1 1024
..             1 1 1024
README         2 2 2226
cat            2 3 23920
echo           2 4 22744
forktest       2 5 13104
grep           2 6 27272
init           2 7 23848
kill           2 8 22720
ln             2 9 22672
ls             2 10 26152
mkdir          2 11 22816
rm             2 12 22808
sh             2 13 41680
stressfs       2 14 23824
usertests      2 15 156032
grind          2 16 37992
wc             2 17 25056
zombie         2 18 22208
console        3 19 0
$
```

* Note that the kernel or the user directory are not part of the file system that is
presented to you, which is why you don't see them here. Although xv6 does not have
a "ps" command available, you can see the current processes by typing ```"Ctrl p"``` and then <Enter>. 
* Since the user
directory is not visible to you in the xv6 environment, you need to shutdown the xv6
virtual machine to add any new user program. 
* To shutdown the virtual machine, you
need to type ```"Ctrl a"``` followed by ```"x"``` without the double quotes. At any time if you
need to boot the OS, just type "make qemu" without the double quotes.

Step3: Adding new user programs
--------------------------------

You will add four new user programs. All user programs are to be added under the user
directory. 
* If a new user program is named foo.c, you need to add $U/_foo to the UPROGS
list in xv6-riscv/Makefile. 

## User programs
1. ```uptime.c``` : Write a program to print the up time of the xv6 virtual machine. Use the
wrapper around the uptime system call. Add ```$U/_uptime``` to UPROGS list in xv6-riscv/Makefile.
If you run "make qemu" without double quotes, it will compile uptime.c and bring
up the virtual machine. Now if you run ls, you should see uptime in that list. You can
test your implementation by running uptime at the $ prompt of xv6. [5 points]

2. ```forksleep.c```: This program takes two integers m and n as command line arguments.
m can take any positive integer value and n can be 0 or 1. You should check these conditions.
The main process forks a child. If n is zero, the child process should sleep for m 
ticks (achieved by calling sleep(m)) and then should print out "pid: Child." without the
double quotes where pid can be obtained by calling getpid(). In this case, the parent
process should print out "pid: Parent." without the double quotes immediately after
returning from fork(). If n is one, the parent process should sleep for m ticks after
returning from fork() and then print out "pid: Parent.", while in this case, the
child process should print out "pid: Child." immediately without sleeping.
As usual, test your program by booting the virtual machine and running forksleep at the
$ prompt. You should see the prints appearing in opposite orders for two different input
values of n. Two sample outputs are shown below. 

```c
$ forksleep 10 0
3: Parent.
4: Child.
$
```
```c
$ forksleep 10 1
6: Child.
5: Parent.
$
```

3. ```pipeline.c``` : This program takes two integers n and x as command line arguments.
n must be a positive integer, while x can take any integer value. You should check these.
The program creates a pipeline of n processes (including the top-level process that
starts the main function). Each process adds its pid to x and passes the new value of
x to the next process in the pipeline using a pipe. Each process, after incrementing x,
prints "pid: x". Make sure to close a file descriptor after you are done using it;
otherwise you will run out of file descriptors for large n. Also, ensure that a process
waits for all its children to complete and apply this rule recursively. Two sample
outputs are given below. [12 points]

```c
$ pipeline 10 0
7: 7
8: 15
9: 24
10: 34
11: 45
12: 57
13: 70
14: 84
15: 99
16: 115
$
```
```c
$ pipeline 1 100
18: 118
$
```

4. ```primefactors.c``` : This program takes an integer n in the range [2, 100] as command line
argument and prints the prime factors of n with multiplicity. You may assume that a global
array of all primes in the range [2, 100] is given to you as follows.

```c
int primes[]={2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97};
```
The program creates a pipeline of processes. The first process (top-level) divides n as many
times as possible by 2, passes on the end-result to the next process through a pipe, and prints
out 2 those many times followed by its pid. The next process divides n as many times as possible
by the next prime number from the primes array, passes on the end-result to the next process
through a pipe, and prints out that prime number those many times followed by its pid. The
procedure continues until n drops to 1. A few sample outputs are shown below. In each line,
the number within the square brackets is the pid of the process responsible for printing
that line. [15 points]
```
$ primefactors 90
2, [3]
3, 3, [4]
5, [5]
$
```

```
$ primefactors 84
2, 2, [6]
3, [7]
7, [9]
$
```

```
$ primefactors 75
3, [11]
5, 5, [12]
$
```

```
$ primefactors 48
2, 2, 2, 2, [13]
3, [14]
$
```

```
$ primefactors 65
5, [17]
13, [20]
$
```

```
$ primefactors 89
89, [44]
$
```