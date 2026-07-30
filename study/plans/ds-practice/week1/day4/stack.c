#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAXN 100

typedef struct {
    int data[MAXN];
    int top;     //栈顶
} Stack;

void init(Stack *s) { s->top = -1; }              //初始化栈
bool isEmpty(Stack *s) { return s->top == -1; }   //空栈
bool isFull(Stack *s) { return s->top == MAXN-1; }//满栈
bool push(Stack *s, int v) { //进栈
    if (isFull(s)) return false;
    s->data[++s->top] = v;
    return true;
}
bool pop(Stack *s, int *out) { //出栈
    if (isEmpty(s)) return false;
    *out = s->data[s->top--];
    return true;
}

int main() {
    Stack s;
    init(&s);
    push(&s, 10);
    push(&s, 20);
    int x;
    while (pop(&s, &x)) {
        printf("pop: %d\n", x);
    }
    return 0;
}
