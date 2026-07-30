#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAXN 100

typedef struct {
    int data[MAXN];
    int top;
} Stack;

void init(Stack *s) { s->top = -1; }
bool isEmpty(Stack *s) { return s->top == -1; }
bool isFull(Stack *s) { return s->top == MAXN-1; }
bool push(Stack *s, int v) {
    if (isFull(s)) return false;
    s->data[++s->top] = v;
    return true;
}
bool pop(Stack *s, int *out) {
    if (isEmpty(s)) return false;
    *out = s->data[s->top--];
    return true;
}
int peek(Stack *s, int *out) {
    if (isEmpty(s)) return 0;
    *out = s->data[s->top];
    return 1;
}

/* 测试用 main */
int main() {
    Stack s;
    init(&s);
    for (int i = 0; i < 5; ++i) push(&s, i*10);
    int x;
    printf("Popping all elements from stack:\n");
    while (pop(&s, &x)) {
        printf("pop: %d\n", x);
    }
    if (!pop(&s, &x)) printf("pop on empty stack returned false as expected\n");
    return 0;
}
