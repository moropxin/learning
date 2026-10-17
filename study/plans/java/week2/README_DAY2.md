# Day 2 学习总结 — 封装 / 继承 / 多态 自查

目录
- 概要
- 今日完成项（产出）
- 代码清单与运行说明
- LeetCode 刷题：题目 3（Longest Substring Without Repeating Characters）完成记录
- 关键知识点速记
- 常见错误与修正（我今天遇到的）
- 复盘结论与下一步计划
- 提交与归档检查清单

---

## 概要
今天目标是巩固面向对象三大核心：封装、继承、和多态；通过笔试选择题自查与两个手写练习（封装示例 + 多态示例）来验证理解；同时完成一题 LeetCode（第 3 题，滑动窗口主题）以保持算法手感。

此 README 是 Day2 的学习回顾与交付说明，便于后续复习与周末汇总。

## 今日完成项（产出）
- 学习笔记（核心概念与区别）已整理到 day2.md（若无请从 README 模板复制并填写）。
- 代码实现：
  - Person.java（封装示例）
  - Employee.java（继承示例）
  - MainRun.java（运行与访问控制验证）
  - Shape.java / Circle.java / Rectangle.java / Square.java（多态示例）
  - PolymorphismDemo.java（多态运行演示）
  - LeetCode3_LongestSubstring.java（第 3 题实现与注释）
- 错题与选择题自查（把错题与解析写入 day2.md）

## 代码清单与运行说明
- 所有 Java 源文件放在：study/plans/java/week2/code/（如果你使用不同目录，请调整路径）。
- 运行示例（在项目根或包含这些文件的目录）：
  - javac *.java
  - java MainRun
  - java PolymorphismDemo
  - java LeetCode3_LongestSubstring

建议把各个 demo 写成独立的 main 方法类，方便单元运行与验证。

## LeetCode 刷题记录 — 题目 3（Longest Substring Without Repeating Characters）
- 题目要点：找最长不含重复字符的子串。
- 采用思路：滑动窗口 + 哈希表（记录字符上次出现位置或字符是否在窗口中）。
- 时间复杂度：O(n)，空间复杂度：O(min(n, charset))。
- 我的实现文件：LeetCode3_LongestSubstring.java（已提交）
- 复盘要点：边界条件（空字符串）、字符集处理（ASCII 或 Unicode）、窗口左指针在遇到重复时移动到正确位置（max(lastIndex+1, left)）。

## 关键知识点速记（精炼）
- 封装：用 private 隐藏字段，通过 getter/setter 控制访问与校验。
- 继承：子类复用父类实现，构造器中先执行父类构造器（super），注意 is-a 关系。
- 多态：父类型引用指向子类对象，编译看引用，运行看对象；重写实现运行时绑定。
- 重载 vs 重写：重载（overload）是同类内不同参数列表；重写（override）是子类提供父类方法的新实现。
- static 与 final：static 方法按类调用（不具备运行时多态），final 方法不可覆盖。

## 常见错误与修正（我今天遇到的）
1. 忽略 private 字段不能被子类直接访问：修正为通过 protected 方法或 public getter 访问。
2. 在向下转型前没有使用 instanceof 检查：增加判断以防 ClassCastException。
3. 滑动窗口左指针移动逻辑错误（直接 left = lastIndex）：修正为 left = Math.max(left, lastIndex + 1)。

把以上错因与修复记录到 day2.md 的“错题与修复”小节，方便回顾。

## 复盘结论与下一步计划
- 理解层面：对封装/继承/多态的核心概念有清晰的认识，能在代码里验证常见行为（构造器顺序、访问权限、运行时绑定）。
- 实战层面：完成多态示例并亲手调试后，对父引用指向子对象调用方法的行为更有直观感受。
- 算法手感：第 3 题使用滑动窗口解法完成，能稳定在 30–40 分钟内写出并复盘。

下一步（周内）：
- Day3：接口 vs 抽象类练习 + 两道 LeetCode（数组/字符串）中等题。
- 将 Day2 的代码合并到仓库指定目录，并在 README 中补充运行输出示例与关键截屏（如果有）。

## 提交与归档检查清单（提交前请确认）
- [ ] day2.md（学习笔记 + 错题解析）已完成并放在 study/plans/java/week2/
- [ ] code/ 下的 Java 源文件已加入并能单独编译运行
- [ ] LeetCode3_LongestSubstring.java 已包含注释并说明测试用例
- [ ] commit message 使用："week2 day2: OOP 自查（封装/继承/多态） + 练习"

---

如果你希望，我可以：
- 1) 把上面的 README 文件直接写入仓库（指定路径）。
- 2) 同步创建 code/ 目录并填入示例 Java 文件（含可运行 main），或者只创建 README 由你手动添加代码。

我现在已经把这份 README 模板准备好了，告诉我是否要我把它直接提交到仓库（我已默认目标仓库为 moropxin/learning 并会写入 study/plans/java/week2/README_DAY2.md），如果确认我将立即提交并回报结果。