# Day1 — 链表练习：删除倒数第 k 个节点 & 环检测

本文件夹包含你今天的练习代码和学习总结：
- linked_list_day1.c — 你上传并测试的 C 示例代码（实现 removeNthFromEnd 与 detectCycle）。
- README.md — 今日学习大纲与复盘（本文件）。

今日目标
- 理解并实现删除链表倒数第 k 个节点（双指针，O(n) 时间 O(1) 空间）。
- 理解并实现 Floyd 龟兔算法检测环并定位环入口。

时间分配（约2小时）
- 0–20 分钟：复习链表基础与画图推演。
- 20–60 分钟：手工跟踪 3–5 个示例用例。
- 60–110 分钟：实现并测试代码（注意 free）。
- 110–120 分钟：读 20–50 行他人 C 代码并写短复盘。

测试用例（建议在 main 中加入/手测）
- removeNthFromEnd:
  - [1,2,3,4,5], k=2 -> [1,2,3,5]
  - [1], k=1 -> []
  - [1,2], k=2 -> [2]
- detectCycle:
  - 无环 -> NULL
  - 环入口为 head
  - 单节点自环

编译与运行（示例）
- gcc -std=c11 -O2 -Wall linked_list_day1.c -o linked_list_day1
- ./linked_list_day1

复盘点（建议）
- 使用 dummy 节点避免特殊情况。
- 操作指针前先做 NULL 检查，释放内存防止泄露。
- 用 Valgrind 检查内存（可选）。

建议的提交信息（commit message）
- "Add ds-practice/week2/day1: linked_list_day1.c and README"
