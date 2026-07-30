# Day4 — 栈与队列、递归

今日目标
- 实现数组栈、循环队列，解决括号匹配，用递归练习基本题目。

如何编译
- gcc -Wall -O2 stack.c -o stack
- gcc -Wall -O2 queue.c -o queue
- gcc -Wall -O2 parentheses.c -o parentheses
- gcc -Wall -O2 recursion.c -o recursion

测试
- ./parentheses
- ./stack
- ./queue
- ./recursion

主要实现说明
- stack.c: 使用固定数组实现简单栈，包含 push/pop/peek/isEmpty/isFull。
- queue.c: 使用固定容量实现循环队列，包含 enqueue/dequeue/isEmpty/isFull。
- parentheses.c: 基于栈实现括号匹配（LeetCode 20）。
- recursion.c: 包含阶乘与朴素递归 Fibonacci 的示例。

提交说明
- 本次提交为 Week1 的 Day4 示例代码与运行说明模板。
