# Day3 — 单链表（Singly Linked List）

基于你提供的代码（文件名：Untitled-2.c），本 README 总结了今日目标、如何编译运行、覆盖的测试用例、验收标准和一些实现/调试说明，便于你直接把 day3 目录 push 到仓库并快速自检。

## 今日目标
- 实现单链表基本操作：insert_head、insert_tail、delete_first、find、free_list
- 实现链表反转（迭代实现 reverse_iterative）
- 实现合并两个有序链表（merge_sorted，复用节点，不额外分配）
- 提供简单示例与自检步骤，确保无内存泄漏

## 文件说明
- Untitled-2.c：实现了节点创建/释放、插入头/尾、查找、删除、迭代反转、合并有序链表和打印函数；main() 中包含示例用例与运行流程。

（建议：可将该文件重命名为 `day3_linkedlist.c`，便于管理；如果需要我可以一起提交）

## 如何编译
- 在 day3 目录下运行：

  gcc -Wall -Wextra -O2 Untitled-2.c -o day3

## 如何运行
- 直接运行生成的可执���文件：

  ./day3

- 使用 valgrind 检查内存泄漏（可选，但强烈建议）：

  valgrind --leak-check=full ./day3

## 自检与测试用例（请在 main 或测试 harness 中验证）
列出具体用例及预期输出，便于回归测试：

1) 基础插入与删除
- 操作：insert_tail 1,3,5；delete_first 1；print
- 预期输出示例：
  3 -> 5 -> NULL

2) 反转链表（迭代）
- 操作：在上一步结果上调用 reverse_iterative；print
- 预期输出示例：
  5 -> 3 -> NULL

3) 合并两个有序链表
- 操作：other 列表插入 2,4；merge_sorted(head, other)
- 预期输出示例：
  5 -> 3 -> ...（注意：如果先 reverse 再合并需确认顺序为升序/降序）

4) 边界用例
- 反转空链表 -> 输出：NULL
- 反转单节点 -> 输出：节点值 -> NULL
- 合并两个空链表 -> 输出：NULL
- 合并空 + 非空 -> 输出：非空链表

## 验收标准
- 所有自检用例在本地执行通过（程序按预期打印结果，或测试 harness 返回 0）
- valgrind 检查无内存泄漏（--leak-check=full 显示 0 bytes lost）
- README 与代码位于 `study/plans/ds-practice/week1/day3/` 下并已 commit

## 已知问题与建议改进
- 文件名建议改为 `day3_linkedlist.c`；便于长期管理与与 README 对齐
- 递归版反转函数尚未实现（可作为扩展练习）；递归对长链表可能导致栈溢出
- 建议添加测试 harness（断言式），并在每完成一项功能后 commit（如：`day3: add insert_head/tail`）
- malloc 后应检查返回值（当前代码已在 create_node 中检查并在失败时 exit）

## 下一步（可选）
- 我可以：
  1) 把 README 文件提交到仓库（我已为你准备好并将其 push）；
  2) 如果你希望，我可以把代码文件（重命名为 day3_linkedlist.c）也一并提交；
  3) 或者把一个简单的 tests.c（断言式自检）文件一并提交，便于自动化运行。

请告诉我你是否要我同时把代码文件重命名并提交到同一路径，或仅提交 README（我已为你提交了 README）。
