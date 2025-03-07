// =======================================
// Begin code for process queue
// =======================================
#define MAX_UINT64 (-1)
#define EMPTY MAX_UINT64
#define QUEUE_HEAD NPROC
#define QUEUE_TAIL (NPROC + 1)
// a node of the linked list
struct qentry {
    uint64 pass; // used by the stride scheduler to keep the list sorted
    uint64 prev; // index of previous qentry in list
    uint64 next; // index of next qentry in list
};
// a fixed size table where the index of a process in proc[] is the same in qtable[]
struct qentry qtable[NPROC+2];

void process_queue_init() {
    qtable[QUEUE_HEAD].pass = 0;
    qtable[QUEUE_HEAD].prev = EMPTY;
    qtable[QUEUE_HEAD].next = QUEUE_TAIL;
    qtable[QUEUE_TAIL].pass = MAX_UINT64;
    qtable[QUEUE_TAIL].prev = QUEUE_HEAD;
    qtable[QUEUE_TAIL].next = EMPTY;
}

int enqueue(int pindex) {
    int prev;

    if (pindex < 0 || NPROC <= pindex) {
        return -1;
    }

    prev = qtable[QUEUE_TAIL].prev;

    qtable[pindex].next = QUEUE_TAIL;
    qtable[pindex].prev = prev;
    qtable[prev].next = pindex;
    qtable[QUEUE_TAIL].prev = pindex;

    return 0;
}

int dequeue() {
    int head_next, head_next_next, head_next_prev;

    head_next = qtable[QUEUE_HEAD].next;

    if (head_next < 0 || NPROC <= head_next) {
        return -1;
    }

    // Don't have to worry queue being full because
    // will detect when it can't find a space in proc[]

    head_next_next = qtable[head_next].next;
    head_next_prev = qtable[head_next].prev;
    qtable[head_next_next].prev = head_next_prev;
    qtable[head_next_prev].next = head_next_next;
    qtable[head_next].next = EMPTY;
    qtable[head_next].prev = EMPTY;

    // return PID
    return head_next;
}

int insert(int pindex, int pass) {
    int curr, prev;

    if (pindex < 0 || NPROC <= pindex) {
        return -1;
    }

    curr = qtable[QUEUE_HEAD].next;
    while (qtable[curr].pass <= pass) {
        curr = qtable[curr].next;
    }

    prev = qtable[curr].prev;

    qtable[pindex].next = curr;
    qtable[pindex].prev = prev;
    qtable[pindex].pass = pass;
    qtable[prev].next = pindex;
    qtable[curr].prev = pindex;

    return 0;
}
// =======================================
// End code for process queue
// =======================================
