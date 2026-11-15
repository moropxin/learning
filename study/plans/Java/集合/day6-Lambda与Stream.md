# W3 D6+D7 合并日 · Lambda / Stream + 集合复盘

> 2026-11-16（一）｜W3 第 6–7 天合并｜档位 A 基础｜配套课程：黑马「Lambda 表达式」「Stream 流」
>
> **为什么合并**：W3 因 D1/D2 各顺延一天，D5 已在 11/15 完成，剩 D6（Lambda+Stream）与 D7（复盘）压进今天 3 小时。
> **今天的取舍**：方法引用（`::`）只看不写；`groupingBy` 会用就行，不下钻。**Lambda 和 Stream 的基本链路必须写熟**，W6 的学生管理项目、后面的 JavaWeb 都天天用。

## 0. 今天会遇到的 7 个名词

| 名词 | 一句话人话 | 类比 |
| --- | --- | --- |
| 函数式接口 | 只有一个抽象方法的接口，能用 Lambda 表示 | 只留一个插口的插座 |
| Lambda `->` | 把一个"方法"当成值传进去的简写 | 一句话交代一件事 |
| 方法引用 `::` | Lambda 的进一步简写，直接借用现成方法 | 直接指人，不用复述 |
| Stream（流） | 对集合做"流水线加工"的工具 | 传送带 |
| 中间操作 | 返回新流，可以继续接（filter/map/sorted） | 传送带上的一个工位 |
| 终结操作 | 结束流并出结果（collect/forEach/count） | 传送带尽头的打包台 |
| `collect` | 把流重新收集成集合 | 打包成箱 |

## 1. 今天 3 小时怎么分

| 时段 | 分钟 | 做什么 |
| --- | --- | --- |
| 看课 | 55 | Lambda 基础 + 函数式接口 + Stream 入门（**方法引用跳过，只看 5 分钟**） |
| 敲代码 | 65 | 用 Lambda 写 `Comparator` 排序；把 D5 的词频统计改写成 Stream；练 `filter/map/sorted/collect/groupingBy` |
| 复盘小测 | 25 | 下面 §9 的闭卷 10 题（不许翻笔记） |
| 错题回炉 | 20 | 对照 §9 答案，把错的题回原文看一遍，填 §10 回炉表 |
| 记录 | 15 | 笔记 3 条 + 术语 3 个 + 更新 `PROGRESS.md`（W3 收尾）+ commit |

## 2. Lambda：把"一段逻辑"当参数传

**先看没有 Lambda 时有多啰嗦**。需求：按年龄给学生排序。

```java
List<Student> list = new ArrayList<>();
// ... 加几个学生

// 老写法：匿名内部类，一个排序写了 6 行，核心逻辑只有一行 a.getAge() - b.getAge()
list.sort(new Comparator<Student>() {
    @Override
    public int compare(Student a, Student b) {
        return a.getAge() - b.getAge();
    }
});
```

**Lambda 写法**：

```java
list.sort((a, b) -> a.getAge() - b.getAge());
```

**为什么能这么简写**？因为 `Comparator` 是**函数式接口**——它只有一个抽象方法 `compare`，编译器能推断出"你写的这个箭头函数，就是 `compare` 的实现"。

```java
@FunctionalInterface          // 这个注解表示"函数式接口"，强制只能有一个抽象方法
public interface Comparator<T> {
    int compare(T o1, T o2);  // 只有一个抽象方法 → 可以用 Lambda
}
```

### Lambda 的语法与省略规则

```java
// 完整写法
(Student a, Student b) -> { return a.getAge() - b.getAge(); }

// 省略 1：参数类型可以省（编译器能推断）
(a, b) -> { return a.getAge() - b.getAge(); }

// 省略 2：只有一个语句且是返回值时，{} 和 return 都能省
(a, b) -> a.getAge() - b.getAge();

// 只有一个参数时，() 也能省
x -> System.out.println(x);

// 没有参数时，() 不能省
() -> System.out.println("hi");
```

| 规则 | 说明 |
| --- | --- |
| 参数类型可省 | 编译器按接口方法推断 |
| 单参数可省 `()` | 多参数不行 |
| 单语句可省 `{}` 和 `return` | 有多条语句就必须加 `{}` |
| 捕获外部变量 | 只能捕获 **`final` 或"事实不变"（effectively final）** 的变量 |

> **"事实不变"陷阱**：在 Lambda 里引用外面的 `int count = 0;` 然后写 `count++`，编译直接报错——因为 Lambda 捕获的是**变量的副本**，改了也影响不到外面。要用可变计数就用数组 `int[] count = {0}` 或 `AtomicInteger`。

## 3. 方法引用 `::`（今天只看不写）

规则：**当 Lambda 只是"原样调用某个已有方法"时，可以简写成 `::`**。

```java
// Lambda
list.forEach(s -> System.out.println(s));
// 方法引用：更短
list.forEach(System.out::println);

// Lambda
list.sort((a, b) -> a.getAge() - b.getAge());
// 不能简化成 ::，因为不是"原样调用"

// Lambda
list.forEach(s -> s.printName());
// 简化
list.forEach(Student::printName);
```

**四种形式（认得出来就行，今天不要求写）**：

| 形式 | 例子 |
| --- | --- |
| 对象::实例方法 | `System.out::println` |
| 类::静态方法 | `Integer::parseInt` |
| 类::实例方法（第一个参数当调用者） | `String::length` |
| 类::构造方法 | `Student::new` |

## 4. Stream 三步走

```java
List<String> names = List.of("张三", "李四", "王五", "张三丰");

List<String> result = names.stream()        // ① 创建流
        .filter(n -> n.length() > 2)        // ② 中间操作（可接多个）
        .map(String::toUpperCase)           // ② 中间操作
        .collect(Collectors.toList());      // ③ 终结操作（出结果）

System.out.println(result);   // [张三丰]
```

| 阶段 | 常见方法 | 特点 |
| --- | --- | --- |
| ① 创建 | `list.stream()`、`Arrays.stream(arr)`、`Stream.of(...)` | 从数据源造流 |
| ② 中间 | `filter` `map` `distinct` `sorted` `limit` `skip` `flatMap` | **返回新流，惰性**，不调用终结操作就什么都不做 |
| ③ 终结 | `collect` `forEach` `count` `anyMatch` `findFirst` `reduce` | **触发执行**，返回结果 |

**三条铁律**：

1. **流只能消费一次**——用完再操作会抛 `IllegalStateException: stream has already been operated upon or closed`。
2. **中间操作是惰性的**——只写 `filter` 不写终结操作，什么都不会发生。
3. **不改变原集合**——`stream()` 是对原集合的"视图加工"，结果要 `collect` 才有。

```java
Stream<String> s = names.stream();
s.forEach(System.out::println);
s.forEach(System.out::println);   // 第二遍直接抛异常
```

## 5. 常用 API（背这 8 个就够用）

| 方法 | 作用 | 例子 |
| --- | --- | --- |
| `filter` | 过滤 | `.filter(s -> s.startsWith("张"))` |
| `map` | 转换（一变一） | `.map(String::length)` |
| `distinct` | 去重（靠 `equals`/`hashCode`） | `.distinct()` |
| `sorted` | 排序 | `.sorted(Comparator.comparing(Student::getAge))` |
| `limit` / `skip` | 取前 N / 跳过前 N | `.limit(3)` |
| `collect` | 收集成集合 | `.collect(Collectors.toList())` |
| `groupingBy` | 分组 | `.collect(Collectors.groupingBy(Student::getClassName))` |
| `count` | 计数 | `.count()` |

```java
// 场景 1：取前 3 名（按分数降序）
List<Student> top3 = students.stream()
        .sorted(Comparator.comparingInt(Student::getScore).reversed())
        .limit(3)
        .collect(Collectors.toList());

// 场景 2：按班级分组，得到 Map<String, List<Student>>
Map<String, List<Student>> groups = students.stream()
        .collect(Collectors.groupingBy(Student::getClassName));

// 场景 3：求平均分
double avg = students.stream()
        .mapToInt(Student::getScore)     // 转成 IntStream（有 sum/average 等方法）
        .average()
        .orElse(0);

// 场景 4：把 List<Student> 的姓名收集成 Set
Set<String> names = students.stream()
        .map(Student::getName)
        .collect(Collectors.toSet());
```

> `Comparator.comparing(Student::getAge)` 比写 `(a, b) -> a.getAge() - b.getAge()` 更安全（不会整数溢出），也更像"业务代码"。习惯它。

## 6. 把 D5 的词频统计改写成 Stream

昨天你是这么写的：

```java
Map<String, Integer> count = new HashMap<>();
for (String w : words) {
    count.put(w, count.getOrDefault(w, 0) + 1);
}
```

用 Stream 可以一行搞定：

```java
String text = "java mysql java redis mysql spring java";

Map<String, Integer> count = Arrays.stream(text.split(" "))
        .collect(Collectors.groupingBy(
                w -> w,                              // 分组键：单词本身
                Collectors.counting()                // 每组计数
        ));
System.out.println(count);   // {java=3, mysql=2, redis=1, spring=1}
```

**再进一步：按次数倒序取前 2 名**（这才是业务里常见的"排行榜"写法）：

```java
List<Map.Entry<String, Integer>> top2 = count.entrySet().stream()
        .sorted(Map.Entry.<String, Integer>comparingByValue().reversed())
        .limit(2)
        .collect(Collectors.toList());
```

> 这段先照抄能跑通就行，不用现在就理解泛型那串 `Map.Entry.<String, Integer>` 的写法——W6 补完反射和泛型深化后回头看会很清楚。

## 7. 动手任务（65 分钟）

**任务 A：Lambda 排序（15 分钟）**
准备 5 个 `Student`（姓名、年龄、分数）。分别用匿名内部类和 Lambda 按年龄升序、按分数降序各排一次。

**任务 B：Stream 基础链路（20 分钟）**
对上面 5 个学生：① 过滤出分数 ≥60 的；② 取出他们的姓名；③ 收集成 `List`；④ 打印。再写一个"按班级分组"的。

**任务 C：词频统计 Stream 版（15 分钟）**
把 D5 的任务 A 用 `groupingBy + counting` 重写，对比两种写法的输出是否一致。

**任务 D：排行榜（15 分钟）**
在任务 C 基础上，按次数倒序输出前 3 名（照 §6 的写法，能跑通就算过）。

**验收标准**

- [ ] 能说出"什么样的接口才能用 Lambda"
- [ ] 能把一个匿名内部类改写成 Lambda
- [ ] 能说出 Stream 的"创建 → 中间 → 终结"三段，各举一个方法
- [ ] 能说出为什么流不能重复使用、为什么中间操作不执行

## 8. Stream 练习（先写，再看答案）

### A. 判断（4 题）

**A1.** `Stream` 的 `filter` 方法执行后，会立刻遍历集合。

> 答：

**A2.** 同一个 `Stream` 对象可以被 `forEach` 两次。

> 答：

**A3.** `list.stream()` 会修改原 `List` 的内容。

> 答：

**A4.** `Comparator` 是函数式接口，所以可以用 Lambda 表示。

> 答：

### B. 选择（3 题）

**B1.** 下列哪个是**终结操作**：
A. `filter`　B. `map`　C. `sorted`　D. `collect`

> 答：

**B2.** 把 `List<Student>` 按班级分组，应该用：
A. `groupBy`　B. `Collectors.groupingBy`　C. `partitioningBy`　D. `map`

> 答：

**B3.** 下列 Lambda 写法正确的是：
A. `(a, b) -> { a + b }`　B. `a, b -> a + b`　C. `(a, b) -> a + b`　D. `(a; b) -> a + b`

> 答：

### C. 简答（3 题）

**C1.** 用自己的话说：为什么 `Comparator` 能写成 Lambda？

> 答：

**C2.** Stream 的中间操作"惰性"是什么意思？举一个体现它的例子。

> 答：

**C3.** 一个 `for` 循环和一个 Stream 链式调用，你什么时候用哪个？

> 答：

## 9. 复盘闭卷小测（25 分钟，10 题，不许翻笔记）

> 覆盖 W3 全部内容（List / 迭代器 / 泛型 / Set / Map / Lambda）。每题 1 分，满分 10。

**1.** `ArrayList` 默认初始容量是多少？扩容几倍？

> 答：

**2.** `LinkedList.get(500)` 的时间复杂度是多少？为什么？

> 答：

**3.** `for (String s : list) { list.remove(s); }` 为什么会抛异常？异常全名是什么？

> 答：

**4.** 泛型在什么时候被擦除成什么？因此哪两件事不能做？

> 答：

**5.** 自定义对象放进 `HashSet` 要重写哪两个方法？只写一个会怎样？

> 答：

**6.** `HashSet` 判断元素重复的条件是什么？

> 答：

**7.** `HashMap.put` 在 key 已存在时返回什么？

> 答：

**8.** 遍历 `HashMap` 为什么推荐 `entrySet()`？

> 答：

**9.** `TreeMap` 和 `HashMap` 的三个区别。

> 答：

**10.** Stream 的"创建 / 中间 / 终结"各举两个方法。

> 答：

## 10. 错题回炉表（做完小测再填）

| 题 | 错在哪（一句话） | 回哪看 | ☐ 已订正 |
| --- | --- | --- | --- |
| 1 | | D1 §3 | ☐ |
| 2 | | D2 §3 | ☐ |
| 3 | | D2 §5 | ☐ |
| 4 | | D3 §5 | ☐ |
| 5 | | D4 §5 | ☐ |
| 6 | | D4 §5 | ☐ |
| 7 | | D5 §3 | ☐ |
| 8 | | D5 §5 | ☐ |
| 9 | | D5 §7 | ☐ |
| 10 | | 今天 §4 | ☐ |

<details>
<summary>复盘小测参考答案</summary>

**1.** 默认容量 10，**懒初始化**（第一次 `add` 才创建数组）；扩容 **1.5 倍**（`oldCapacity + (oldCapacity >> 1)`），用 `Arrays.copyOf` 搬数据。

**2.** O(n)。`LinkedList` 没有下标，`get(500)` 要从 `first` 或 `last` 出发**折半遍历**，走到第 500 个节点才拿到，所以是 O(n)（准确说是 O(i)）。循环里 `get(i)` 累加就是 O(n²)。

**3.** 因为增强 for 底层是迭代器，遍历过程中 `list.remove` 会让 `modCount` 变化，而迭代器里的 `expectedModCount` 没同步，下一次 `next()` 检查时不一致就抛 `ConcurrentModificationException`（fail-fast 机制）。修法：用 `Iterator.remove()`、`removeIf` 或先收集再删。

**4.** 泛型只在**编译期**有效，编译后类型参数被擦除成 `Object`（**有上界则擦成上界**）。因此：① 不能 `new T()`、不能 `new T[10]`（运行时不知道 T 是什么）；② 不能用基本类型当类型参数（`List<int>` 不行，要用 `List<Integer>`）。

**5.** 要重写 `equals` **和** `hashCode`。只写 `equals` 时，用的还是 `Object` 默认的 `hashCode`（按内存地址算），两个逻辑上相等的对象哈希值不同 → 落到不同桶 → 根本轮不到比 `equals` → 去重失效。

**6.** **`hashCode` 相同 且 `equals` 为 true**（`TreeSet` 例外，它看 `compare` 返回 0）。

**7.** 返回**被覆盖的旧 value**；如果这个 key 之前不存在，返回 `null`。

**8.** `keySet()` 只给 key，取 value 还要再 `get(key)`，而 `get` 内部要重新算哈希、重新定位桶，相当于每个元素多走一次查找；`entrySet()` 直接返回 `Entry`（同时持有 key 和 value），一次遍历拿全，省掉二次定位。

**9.** ① 顺序：`HashMap` 无序，`TreeMap` 按 key 排序；② 底层：哈希表 vs 红黑树；③ `TreeMap` 的 key 必须能比较且**不能为 null**，`HashMap` 可以。另外性能上 `HashMap` 平均 O(1)，`TreeMap` O(log n)。

**10.** 创建：`stream()`、`Stream.of()`；中间：`filter`、`map`（还有 `distinct`/`sorted`/`limit`）；终结：`collect`、`forEach`（还有 `count`/`findFirst`/`reduce`）。

</details>

## 11. 高频坑

| 坑 | 说明 |
| --- | --- |
| 流重复使用 | 抛 `IllegalStateException`，要重新 `stream()` |
| 只写中间操作不写终结操作 | 惰性，什么都不执行，看起来"没效果" |
| 以为 `stream()` 会改原集合 | 不会，结果要 `collect` |
| Lambda 里改外部局部变量 | 编译报错，只能捕获 effectively final |
| `sorted` 前没 `map` 到可比类型 | 自定义对象要传 `Comparator`，否则 `ClassCastException` |
| `Collectors.toMap` 遇到重复 key | 抛 `IllegalStateException`，要传第三个参数处理冲突 |
| 把 Stream 当"更快的 for" | 数据量小的时候 Stream 未必更快，可读性优先 |

## 12. 30 秒卡

- **Lambda 的前提**：目标类型是**函数式接口**（只有一个抽象方法）。
- **Stream 三段**：创建 → 中间（惰性，返回流）→ 终结（触发执行，出结果）。
- **三条铁律**：只能用一次、中间操作惰性、不改原集合。
- **四个常用**：`filter` 过滤、`map` 转换、`sorted` 排序、`collect` 收集。
- **分组**：`Collectors.groupingBy(键函数)`。

## 13. 今天要记的 3 个术语（写进术语表）

- 函数式接口
- `effectively final`
- 流（Stream）/ 中间操作 / 终结操作
- `Collectors.groupingBy`

## 14. 面试一句话

> **Lambda 和 Stream 分别解决了什么问题？**
> Lambda 解决的是"给接口传一段逻辑还要写一堆匿名内部类"的啰嗦问题——只要目标类型是**函数式接口**（只有一个抽象方法），就能用 `参数 -> 逻辑` 的写法，编译器自动推断类型并生成实现。Stream 解决的是"集合加工要写多重 for 循环"的问题，把过滤、转换、排序、分组串成一条链：`stream()` 创建、`filter/map/sorted` 等中间操作（惰性，返回新流）、`collect/forEach` 等终结操作触发执行。要注意流只能消费一次、中间操作不改原集合。

## 15. W3 收尾（做完这篇就结束本周）

- [ ] 把今天 3 个术语补进 [术语表](../00-JavaSE总体路线与术语表.md)
- [ ] 更新 [`PROGRESS.md`](../../PROGRESS.md)：W3 ✅，填今日日志（含复盘小测得分）
- [ ] `git commit`：`docs: W3 收尾（集合复盘小测 x/10）`
- [ ] 下周 W4 是**异常 + IO**，从 11/17（二）开始，比原计划晚一天；11/22（日）晚复盘并 1 小时补上
