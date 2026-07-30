#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define CAP 5

typedef struct {
    int data[CAP];
    int head; // index of first element
    int tail; // index after last element
    int size;
} CircularQueue;

void initQ(CircularQueue *q) { q->head = 0; q->tail = 0; q->size = 0; }
bool isEmptyQ(CircularQueue *q) { return q->size == 0; }
bool isFullQ(CircularQueue *q) { return q->size == CAP; }
bool enqueue(CircularQueue *q, int v) {
    if (isFullQ(q)) return false;
    q->data[q->tail] = v;
    q->tail = (q->tail + 1) % CAP;
    q->size++;
    return true;
}
bool dequeue(CircularQueue *q, int *out) {
    if (isEmptyQ(q)) return false;
    *out = q->data[q->head];
    q->head = (q->head + 1) % CAP;
    q->size--;
    return true;
}

int main() {
    CircularQueue q;
    initQ(&q);
    printf("Enqueue 1..5 (one will fail when full):\n");
    for (int i = 1; i <= 5; ++i) {
        bool ok = enqueue(&q, i);
        printf("enqueue %d -> %s\n", i, ok?"ok":"fail");
    }
    int x;
    printf("Dequeue all elements:\n");
    while (dequeue(&q, &x)) {
        printf("dequeue: %d\n", x);
    }
    printf("Attempt dequeue on empty -> %s\n", dequeue(&q, &x)?"ok":"fail");
    return 0;
}
