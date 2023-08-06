struct barrier {
    int state; // 0 for free and 1 for occupied
    int id; //barrier_id
    int num_proc_count; // num of processes entered in the barrier
    struct sleeplock lk;
    struct cond_t cv;
};