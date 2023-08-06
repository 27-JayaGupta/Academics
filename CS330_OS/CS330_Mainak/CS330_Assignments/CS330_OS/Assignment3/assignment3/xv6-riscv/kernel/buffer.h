

struct buffer_elem {
    int x; // value stored in buffer
    int full; // is buffer elem full or empty
    struct sleeplock lk;
    struct cond_t inserted;
    struct cond_t deleted;
};
