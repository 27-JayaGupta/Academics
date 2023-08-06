#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"
#include "condvar.h"
#include "semaphore.h"

void
sem_init (struct semaphore *s, int x)
{
    s->value = x;
    cond_init(&s->cv, "sem_cond_var");
    initsleeplock(&s->mutex, "sem_lock");
}

void
sem_wait (struct semaphore *s)
{
    acquiresleep(&s->mutex);
    while(s->value <= 0) {
        cond_wait(&s->cv, &s->mutex);
    }
    s->value -= 1;
    releasesleep(&s->mutex);
}   

void
sem_post (struct semaphore *s)
{
    acquiresleep(&s->mutex);
    s->value += 1;
    cond_signal(&s->cv);
    releasesleep(&s->mutex);
}
