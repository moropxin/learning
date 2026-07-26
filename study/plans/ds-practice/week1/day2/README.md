# Week1 — Day2：排序算法练习

目录
- 今日目标
- 文件说明
- 如何编译与运行
- 示例输入与期望输出
- 本地测试步骤（请按顺序执行并把运行结果粘到“运行结果”处）
- 常见问题排查
- 学习笔记模板（请填入）
- 提交与分支建议
- 参考资料

## 今日目标
- 理解并实现三种排序：插入排序（insertion sort）、归并排序（merge sort）、快速排序（quick sort）。
- 能够在本地编译、运行并验证常见边界条件（空数组、单元素、有序、逆序、重复元素）。
- 把代码与本次学习记录 push 到仓库：study/plans/ds-practice/week1/day2/

## 文件说明
- main.c —— 排序算法实现与简单测试（day2/main.c）
- README.md ——（本文件）记录编译/运行方法、测试结果与学习笔记

## 如何编译（在仓库根或 day2 目录下执行）
编译：
```bash
gcc -std=c11 -O2 -Wall study/plans/ds-practice/week1/day2/main.c -o day2/main
```
（调试/内存检查可用）
```bash
gcc -std=c11 -O2 -Wall -fsanitize=address,undefined -g study/plans/ds-practice/week1/day2/main.c -o day2/main
```

## 如何运行
```bash
./day2/main
```
把终端完整输出复制到下面的“运行结果”区域。

## 示例输入与期望输出
- 原数组： [1,3,4,2,6]
- 期望：
  - insertion_sort: [1,2,3,4,6]
  - merge_sort:     [1,2,3,4,6]
  - quick_sort:     [1,2,3,4,6]

## 本地测试步骤（按顺序）
1. 编译（参考上面命令）。如报错，粘出编译器的第一条错误信息。
2. 运行程序，复制并粘贴完整输出到“运行结果”部分。
3. 修改测试数据并再次运行（尝试以下样例）：
   - 空数组
   - 单元素：[42]
   - 已排序：[1,2,3,4,5]
   - 逆序：[5,4,3,2,1]
   - 含重复：[3,1,2,3,3,0]
4. 若使用随机枢轴，重复运行多次，观察 quick_sort 输出是否稳定正确。

运行结果（请把 ./day2/main 的输出粘在这里）
- 编译命令及输出：
- 程序运行输出（完整复制）：

## 常见问题排查（快速参考）
- 无法编译：
  - 检查文件路径和文件名是否正确。
  - 若使用 ssize_t，包含 <sys/types.h> 或改用 ptrdiff_t（包含 <stddef.h>）。
  - 若出现 zero-length array（int empty[] = {}），改为 int *empty = NULL 或移除测试。
- 运行崩溃（segfault）：
  - 用 AddressSanitizer 编译运行：-fsanitize=address,undefined。
  - 检查数组下标、递归基线（l >= r）、以及无符号下溢（size_t p - 1）。
- quick_sort 行为异常：
  - 确保 partition 的实现没越界；在递归左侧调用前检查 p > lo（或 p > 0）。
  - 若遇到性能退化，启用随机枢轴或 median-of-three。

## 学习笔记（请你填写）
- 我学到：
  - 1)
  - 2)
- 我不太理解：
  - 1)
  - 2)
- 下一步想做：
  - 1)

## 提交与分支建议
- 建议新建分支并提交：
```bash
git checkout -b week1-day2
git add study/plans/ds-practice/week1/day2/main.c study/plans/ds-practice/week1/day2/README.md
git commit -m "week1/day2: add sorting implementations and tests"
git push -u origin week1-day2
```
- 或提交到 main（如果你确定直接更新主分支）：
```bash
git add ...
git commit -m "week1/day2: add sorting implementations and tests"
git push origin main
```

## 参考资料（可选）
- 算法导论 / 任意教材章节：归并排序与快速排序
- 在线讲解（中文）：B 站搜索 “归并排序 快速排序 C 语言”
- LeetCode（练习排序相关题目）：https://leetcode.cn/
