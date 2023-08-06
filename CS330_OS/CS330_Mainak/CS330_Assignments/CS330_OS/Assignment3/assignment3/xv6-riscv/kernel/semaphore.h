struct semaphore{
    uint value; // initial value of semaphore
    struct sleeplock mutex;     // lock 
    struct cond_t cv;
};