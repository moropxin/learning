# W3 · JavaSE 补课①：集合框架 + Lambda/Stream

> 2026-11-09（一）– 11-15（日）｜档位：A 基础｜里程碑：**M2 集合能独立写业务代码**
>
> **为什么改成补课**：原计划假设"Java 最新版已学完"，实际课程只讲到基础语法/面向对象，集合之后（集合、IO、多线程、反射）都还没学透。所以 W3 不是自检，是**正经学**——这三周补的是后面 JavaWeb、SSM、项目的共同地基，跳不过去。

## 一、本周目标

1. 能说清集合体系：`Collection`（List/Set）与 `Map` 两条线，各自什么场景用。
2. `ArrayList` 会增删改查 + 三种遍历；知道底层数组、扩容 1.5 倍。
3. `HashSet` 去重的条件（重写 `equals` + `hashCode`）；`TreeSet` 会用比较器排序。
4. `HashMap` 会存、会遍历（三种方式）、会用 `getOrDefault`/`computeIfAbsent` 写统计。
5. 会用 Lambda + Stream 做过滤/映射/统计，替代 for 循环。

> 本周不追求"底层源码全懂"，只要求：**能写出来、能讲清楚用哪个、能说明为什么**。红黑树、扩容死锁这些留到 W6 综合和周八股里再补。

## 二、时间节奏（A 档：看课 85 + 敲代码 55 + 算法 25 + 记录 15 = 180 分钟）

| 时间块 | 分钟 | 做什么 |
| --- | --- | --- |
| 看课 | 85 | 黑马「集合」对应章节，1.25 倍速，**边看边暂停敲** |
| 敲代码 | 55 | 当天知识点自己写一遍，不看视频抄 |
| 算法 | 25 | 力扣 1 题（数组/哈希，跟当天集合对应） |
| 记录 | 15 | 笔记 3 条 + `git commit` + 日志 |

> 新手期的铁律：**看课时间不能超过动手时间的两倍**。看懂 ≠ 会写，视频里敲一遍、自己再默写一遍，才算过。

## 三、每日任务

| 天 | 今日目标 | 看课章节 | 动手任务 | 验收标准 |
| --- | --- | --- | --- | --- |
| 一 11/09 | 集合体系 + `List`（ArrayList） | 集合概述、Collection、List、ArrayList 底层 | 建 `ListDemo`：存 5 个学生，做增删改查 + 三种遍历（普通 for / 增强 for / 迭代器） | 能不看笔记写出三种遍历；能说出 ArrayList 扩容规则 |
| 二 11/10 | `LinkedList` + 迭代器 + 并发修改异常 | LinkedList、Iterator、List 选型 | 用 `LinkedList` 实现一个"撤销"栈；复现 `ConcurrentModificationException` 并用 `Iterator.remove` 修好 | 能解释"为什么 foreach 里 remove 会炸" |
| 三 11/11 | 泛型 + 可变参数 | 泛型类/方法/接口、通配符、可变参数 | 写一个泛型 `Box<T>`；写 `sum(int... nums)`；把周一的 `ListDemo` 改成泛型版 | 能说清 `<? extends T>` 和 `<? super T>` 分别用来读还是写 |
| 四 11/12 | `Set`（HashSet/LinkedHashSet/TreeSet） | Set 家族、哈希值、去重原理、比较器 | 自定义 `Student`（姓名+年龄）放进 `HashSet` 去重；再用 `TreeSet` 按年龄排序 | 能解释"为什么没重写 hashCode 就去重失败" |
| 五 11/13 | `Map`（HashMap/TreeMap） | Map 接口、HashMap 底层、遍历、TreeMap | 写词频统计：读一段字符串，统计每个单词出现次数，按次数倒序输出 | 三种遍历都会写；`getOrDefault` 会用 |
| 六 11/14 | Lambda + 方法引用 + Stream | Lambda 表达式、函数式接口、方法引用、Stream 入门 | 用 Stream 重写词频统计的过滤/排序部分；练 `filter/map/sorted/collect/groupingBy` | 能把一个 for 循环改写成 Stream 链式调用 |
| 日 11/15 | 复盘 + 小测 + 综合案例 | 本周笔记 | 闭卷小测 10 题；做「斗地主发牌」或「学生成绩统计」综合案例 | 10 题 ≥7 分；案例能跑通 |

## 四、本周产出

- `study/notes/java-集合.md`：集合体系图（List/Set/Map 各自实现类 + 选型表）+ 5 个高频坑
- `study/projects/collection-practice/`：`ListDemo`、`SetDemo`、`MapDemo`、`StreamDemo`、综合案例（5 个可运行的类）
- `PROGRESS.md`：W3 打卡 + 每日日志

## 五、自查 3 问（周日口述）

1. `ArrayList` 和 `LinkedList` 什么时候用哪个？为什么业务里 90% 用 `ArrayList`？
2. `HashSet` 是怎么去重的？只重写 `equals` 不重写 `hashCode` 会怎样？
3. 一个 `for` 循环和一个 Stream 链式调用，什么时候该用哪个？

## 六、高频坑 / 减法清单

| 坑 | 说明 |
| --- | --- |
| 觉得"看懂了就是会了" | 集合必须手写：不看视频、不看笔记，从空白类开始敲一遍 |
| `==` 比较对象 | 比的是地址；字符串/对象一律 `equals`（`Integer` 的 127/128 陷阱也是这个） |
| foreach 里删元素 | 会抛 `ConcurrentModificationException`，用 `Iterator.remove()` 或 `removeIf` |
| 自定义对象进 `HashSet`/`HashMap` 不重写 `hashCode` | 去重/查找直接失效，这是最高频的 bug |
| `TreeSet` 放自定义对象不实现 `Comparable` | 运行时抛 `ClassCastException`，必须给比较器 |
| Stream 只遍历一次 | 流用完就废，再操作要重新 `stream()` |

**减法顺序**：周内若掉队，先砍「Stream 的方法引用」→ 再砍「TreeMap」→ 再砍综合案例的第二个。**List/Map/Set 三件套不能砍**，JavaWeb 和项目天天用。

## 七、面试一句话

> **ArrayList 和 LinkedList 怎么选？**
> 查询多用 `ArrayList`——底层数组，随机访问 O(1)，内存连续、缓存友好；频繁在头部增删才用 `LinkedList`，但它每个节点要额外存前后指针，随机访问 O(n)。实际业务里 90% 用 `ArrayList`，因为真正需要频繁头插的场景很少，而且 `ArrayDeque` 比 `LinkedList` 更适合当队列用。
