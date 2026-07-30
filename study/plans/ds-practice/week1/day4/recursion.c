#include <stdio.h>

long factorial(int n) { // 阶乘之和
    if (n <= 1) return 1;
    return n * factorial(n-1);
}

int fib(int n) { // 斐波那契数列
    if (n <= 1) return n;
    return fib(n-1) + fib(n-2);
}

int main() {
    printf("5! = %ld\n", factorial(5));
    printf("fib(10) = %d\n", fib(10));
    return 0;
}
