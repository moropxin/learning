# W3 D3 · 泛型与可变参数

> 2026-11-12（四）晚｜W3 第 3 天｜档位 A 基础｜配套课程：黑马「泛型」— 泛型类/方法/接口、通配符、可变参数
>
> 今天的目标只有一个：**看懂 `List<User>` 里的尖括号到底在干嘛**，以及为什么框架里到处是 `<T>`。这是你后面看 Spring、MyBatis 代码时天天要碰的东西。

## 0. 今天会遇到的 7 个名词（先扫一眼，学完回来对一遍）

| 名词 | 一句话人话 | 类比 |
| --- | --- | --- |
| 泛型 | 给类/方法贴上"我只处理某种类型"的标签 | 分类垃圾桶 |
| 类型参数 `T` | 占位符，用的时候才填真正的类型 | 填空题的空格 |
| 类型擦除 | 泛型只在**编译期**有用，跑起来就被抹掉 | 贴纸撕掉后都是同一个桶 |
| 上界 `extends` | 限制"只能是某个类或它的子类" | 只收"水果及其子类" |
| 下界 `super` | 限制"只能是某个类或它的父类" | 只收"苹果及其父类" |
| 通配符 `?` | "某种类型，我不确定/不关心是哪种" | 随便什么但得同类 |
| 可变参数 `T...` | 参数个数不定的简写，本质是数组 | 自助餐，能吃几个吃几个 |

## 1. 今天 3 小时怎么分

| 时段 | 分钟 | 做什么 | 产出 |
| --- | --- | --- | --- |
| 看课 | 80 | 泛型类/方法/接口、通配符、可变参数，1.25 倍速，边看边敲 | 跟敲代码 |
| 敲代码 | 60 | 泛型 `Box<T>` + 泛型方法 + 可变参数工具类 + 改造昨天的 `ListDemo` | 4 个能跑的类 |
| 算法 | 25 | 力扣 1 题（哈希类，如「有效的字母异位词」） | AC 记录 |
| 记录 | 15 | 笔记 3 条 + **术语 3 个**（写进 [术语表](../00-JavaSE总体路线与术语表.md)）+ commit + 日志 | 提交完成 |

## 2. 没有泛型会怎样（先感受痛点）

```java
// 假设没有泛型，集合只能装 Object
List list = new ArrayList();
list.add("张三");
list.add(18);        // 编译器不拦你，因为都是 Object

// 取的时候必须强转
String name = (String) list.get(0);   // 没问题
String age  = (String) list.get(1);   // 运行时炸：ClassCastException
```

两个问题：

1. **装错了类型，编译器不管**，要等运行时才发现（可能上线才炸）。
2. **每次取都要强转**，代码又丑又容易写错。

泛型的解法：**把"装什么类型"提前告诉编译器**。

```java
List<String> list = new ArrayList<>();
list.add("张三");
// list.add(18);      // 编译期直接报错：需要 String，传了 int
String name = list.get(0);   // 不用强转，编译器已经知道是 String
```

一句话：**泛型 = 把类型检查从"运行时"提前到"编译期"，顺便省掉强转。**

## 3. 泛型类：自己写一个 `Box<T>`

```java
// <T> 是"类型参数"，T 只是名字，习惯用 T（Type）/E（Element）/K,V（Key,Value）
// 它像方法的参数，只不过传的是"类型"而不是"值"
public class Box<T> {
    private T content;              // 这个字段的类型，等 new 的时候才确定

    public void put(T content) {    // 参数类型是 T
        this.content = content;
    }

    public T get() {                // 返回值类型是 T
        return content;
    }
}
```

```java
// 用的时候把 T 填成具体类型
Box<String> stringBox = new Box<>();   // 这里的 T = String
stringBox.put("hello");
String s = stringBox.get();            // 不用强转

Box<Integer> intBox = new Box<>();     // 这里的 T = Integer
intBox.put(123);
int i = intBox.get();

// stringBox.put(123);  // 编译报错：Box<String> 只能装 String
```

> `Box<String>` 和 `Box<Integer>` 在**编译器眼里是两个不同的类型**，不能互相赋值。这就是泛型带来的安全性。

## 4. 泛型方法：`<T>` 写在返回值前面

```java
public class ArrayUtil {
    // 注意：<T> 写在 static 后面、返回值前面，表示"这个方法自己声明了一个类型参数"
    public static <T> void printArray(T[] arr) {
        for (T t : arr) {
            System.out.print(t + " ");
        }
        System.out.println();
    }

    // 交换数组两个位置
    public static <T> void swap(T[] arr, int i, int j) {
        T temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}
```

```java
String[] names = {"A", "B", "C"};
ArrayUtil.printArray(names);      // 编译器自动推断 T = String
Integer[] nums = {1, 2, 3};
ArrayUtil.printArray(nums);       // T = Integer

ArrayUtil.swap(names, 0, 2);      // [C, B, A]
```

**泛型类 vs 泛型方法，怎么分清？**

| 写法 | 含义 |
| --- | --- |
| `class Box<T> { }` | 整个类共享这一个 `T` |
| `public <T> void m(T t) { }` | 只有这个方法用 `T`，和类上的 `T` 无关 |
| `public static <T> T m() { }` | 静态方法必须自己声明 `<T>`（因为静态方法不属于对象，拿不到类上的 `T`） |

## 5. 类型擦除：为什么不能 `new T()`

**关键事实**：泛型只活在**编译期**。编译完成后，`Box<String>` 和 `Box<Integer>` 在 JVM 里都是同一个 `Box`，里面的 `T` 全部被替换成 `Object`（有上界就替换成上界）。这叫**类型擦除**。

```java
Box<String> a = new Box<>();
Box<Integer> b = new Box<>();
System.out.println(a.getClass() == b.getClass());  // true！运行时它俩是同一个类
```

正因如此，下面这些都**编译不过**：

```java
// 1. 不能 new 一个"类型参数"
// T obj = new T();                 // 擦除后是 new Object()，编译器不让你写

// 2. 不能 new 泛型数组
// T[] arr = new T[10];             // 数组要在运行时知道具体类型，而 T 已经没了

// 3. 不能用基本类型当类型参数
// List<int> list = new ArrayList<>();   // 必须是包装类 List<Integer>

// 4. 静态成员不能用类上的 T
// public static T instance;        // 静态先于对象存在，T 还没确定
```

> **类型擦除的唯一考点**：**泛型是编译期的语法糖，运行时不存在。** 面试问"泛型怎么实现的"，答案就是这五个字。

## 6. 通配符：`? extends` 和 `? super`

场景：你写一个方法，参数是"某种 List"，但**不想限定死是 `List<String>` 还是 `List<Integer>`**。

```java
// 需求：打印任意类型的 List
public static void print(List<?> list) {   // ? 表示"某种类型，我不关心"
    for (Object o : list) {
        System.out.println(o);
    }
}
```

再进一步，需要"能读"或"能写"时，就要用上界/下界：

```java
// 上界 extends：只能接收 Number 及其子类（Integer、Double...）
// 作用：你可以安全地"读"出来当 Number 用
public static double sum(List<? extends Number> list) {
    double total = 0;
    for (Number n : list) {      // 读出来一定能当 Number 用
        total += n.doubleValue();
    }
    return total;
    // list.add(1);              // 不能写！因为你不知道具体是 Integer 还是 Double
}

// 下界 super：只能接收 Integer 及其父类（Number、Object）
// 作用：你可以安全地"写"进去
public static void addInts(List<? super Integer> list) {
    list.add(1);                 // 能写，写 Integer 一定安全
    list.add(2);
    // Integer i = list.get(0);  // 读出来只能是 Object，不能直接当 Integer
}
```

**PECS 口诀（面试高频）**：

> **P**roducer **E**xtends，**C**onsumer **S**uper
> 只往外**读**（生产者）用 `? extends T`；只往里**写**（消费者）用 `? super T`。

| 你可能想做什么 | 用什么 | 为什么 |
| --- | --- | --- |
| 从集合里读数据 | `? extends T` | 子类一定能当父类用（协变安全） |
| 往集合里写数据 | `? super T` | 父类容器装得下子类对象（逆变安全） |
| 又读又写 | 别用通配符，用具体类型 `T` | — |

## 7. 可变参数 `T...`

**需求**：写一个求和方法，参数个数不定。

```java
// 老写法 1：重载一堆方法 —— 笨
// public static int sum(int a, int b)
// public static int sum(int a, int b, int c)

// 老写法 2：传数组 —— 调用时要自己 new 数组，麻烦
// sum(new int[]{1,2,3});

// 可变参数：本质还是数组，但调用时可以"直接列出来"
public static int sum(int... nums) {
    int total = 0;
    for (int n : nums) {     // 在方法内部，nums 就是一个 int[]
        total += n;
    }
    return total;
}
```

```java
sum();              // 0 个参数，nums 是空数组（不是 null！）
sum(1);             // 1
sum(1, 2, 3);       // 6
sum(new int[]{1,2,3});  // 也可以直接传数组，等价
```

**四条规矩（考试/面试都爱考）**：

1. 一个方法**最多一个**可变参数。
2. 可变参数**必须是最后一个参数**：`void m(String s, int... nums)` ✅；`void m(int... nums, String s)` ❌。
3. 方法内部它**就是数组**，所以 `nums.length`、`nums[0]` 都能用。
4. 不传参数时它是**长度为 0 的数组，不是 null**——所以不用判空，直接 for 就行。

```java
// 组合用法：必填参数在前，可变参数在后
public static void log(String level, String... messages) {
    for (String m : messages) {
        System.out.println("[" + level + "] " + m);
    }
}
log("INFO");                       // 不打印
log("INFO", "启动成功");            // [INFO] 启动成功
log("ERROR", "超时", "重试失败");   // 两行
```

## 8. 动手任务（60 分钟，不许抄视频）

**任务 A：泛型 `Box<T>`（15 分钟）**
写 `Box<T>`（`put`/`get`/`isEmpty`），分别用 `String`、`Integer`、自定义的 `Student` 各 new 一个，验证"装错类型编译就报错"。

**任务 B：泛型方法（15 分钟）**
写 `ArrayUtil.printArray(T[])` 和 `ArrayUtil.swap(T[], i, j)`，用 `String[]` 和 `Integer[]` 各调一次。

**任务 C：可变参数工具类（15 分钟）**
写 `MathUtil.sum(int... nums)` 和 `MathUtil.max(int... nums)`（空参数时 `max` 返回什么？自己定一个合理规则并写注释）。

**任务 D：改造昨天的 `ListDemo`（15 分钟）**
把 `List` 全部加上泛型 `<Student>`，写一个 `List<Student>` 的增删改查；再写一个 `printAll(List<? extends Student>)` 接收它。

**验收标准**

- [T] 能解释 `Box<String>` 和 `Box<Integer>` 为什么不能互相赋值
- [T] 能说出为什么不能 `new T()`、不能 `new T[10]`
- [T] 能说出 `? extends` 和 `? super` 分别用来读还是写
- T ] 能说出可变参数在方法内部是什么类型

## 9. 练习（先写，再看答案）

### A. 判断（5 题）

**A1.** `List<String>` 和 `List<Integer>` 在运行时是两个不同的类。

> 答：错，都是上升为object类

**A2.** 泛型可以让 `List<int>` 正常工作。

> 答：错，泛型只是引用类型，程序运行时消除

**A3.** 静态方法中可以直接使用类上声明的类型参数 `T`。

> 答：错，静态方法无法创建对象，new不出来

**A4.** 可变参数在方法内部就是一个数组。

> 答：对

**A5.** `List<? extends Number>` 类型的集合，可以往里 `add` 一个 `Integer`。

> 答：错，extend只能读不能写

### B. 选择（5 题）

**B1.** 下列哪个是合法的泛型声明：
A. `class Box<int> {}`　B. `class Box<T> {}`　C. `class Box<T, T> {}`　D. `class Box<> {}`

> 答：B

**B2.** `List<? super Integer>` 最适合用来：
A. 只读数据　B. 只写数据　C. 反射创建对象　D. 排序

> 答：B

**B3.** 关于类型擦除，正确的是：
A. 运行时会检查泛型类型　B. 泛型信息在运行时被擦除　C. 泛型会生成新的类文件　D. 泛型只能用于基本类型

> 答：B

**B4.** `public static int sum(int... nums)` 调用 `sum()` 时，`nums` 是：
A. `null`　B. 长度为 0 的数组　C. 编译报错　D. 长度为 1 的数组

> 答：B

**B5.** 下列关于可变参数的说法，错误的是：
A. 一个方法只能有一个可变参数　B. 可变参数必须在参数列表最后　C. 可变参数可以放在任意位置　D. 可变参数本质是数组

> 答：C

### C. 简答（3 题）

**C1.** 用大白话说清"泛型"解决了什么问题（不要背定义，讲痛点）。

> 答：此前集合装object类，不区分子类别，取出来要强转，泛型提前处理了，并规范了数组类型

**C2.** 什么是类型擦除？它导致哪两件事不能做？

> 答：当程序运行时泛型类型擦除，新建泛型对象、新建泛型数组不能做，不能用基本类型做参数，要用包装类

**C3.** `? extends T` 和 `? super T` 分别适合什么场景？用 PECS 口诀说明。

> 答：第一个适合只读，父类可包容子类。第二个适合只写，子类可放进父类里。

<details>
<summary>参考答案</summary>

**A 判断**：A1 错（擦除后是同一个类）｜A2 错（泛型只能是引用类型，要用 `List<Integer>`）｜A3 错（静态方法必须自己声明 `<T>`）｜A4 对｜A5 错（`extends` 只能读不能写）

**B 选择**：B1 B（C 重复了 T，D 缺少类型参数名）｜B2 B（下界适合写）｜B3 B｜B4 B（空数组，不是 null）｜B5 C

**C1**：以前集合只能装 `Object`，装错类型编译器不管，取出来还要强转，一错就 `ClassCastException`。泛型把"装什么类型"写在声明里，让编译器提前检查，同时省掉强转。本质是**把类型错误从运行时提前到编译期**。

**C2**：泛型只在编译期存在，编译后所有 `T` 都被替换成 `Object`（或有上界则替换成上界），运行时拿不到具体类型信息，这叫类型擦除。两个直接后果：① 不能 `new T()`、不能 `new T[10]`（运行时不知道 T 是什么）② 不能用基本类型做类型参数（`List<int>` 不行，只能用 `List<Integer>`）。

**C3**：PECS——Producer Extends, Consumer Super。只从集合里**读**（当生产者）用 `? extends T`，因为子类可以安全地当父类用；只往集合里**写**（当消费者）用 `? super T`，因为父类容器能装子类对象。既读又写就别用通配符，直接用具体类型 `T`。

</details>

## 10. 高频坑

| 坑 | 说明 |
| --- | --- |
| `List<int>` | 泛型只能是引用类型，用 `List<Integer>` |
| 以为泛型运行时有类型检查 | 运行时已被擦除，`instanceof List<String>` 编译不过 |
| `new T()` / `new T[10]` | 擦除后不知道 T 是什么，编译器禁止 |
| 静态成员用类上的 `T` | 静态方法必须自己声明 `<T>` |
| `? extends` 里 `add` | 编译报错；`extends` 只能读 |
| `? super` 里直接取 | 取出来是 `Object`，要强转 |
| 可变参数放中间 | 编译报错，必须是最后一个 |
| 以为不传可变参数是 `null` | 是**空数组**，直接遍历不会 NPE |

## 11. 30 秒卡

- **泛型解决的问题**：类型错误从运行时提前到编译期 + 省掉强转。
- **类型擦除**：泛型只在编译期存在，运行时被擦成 `Object`（或上界）。
- **三个不能**：不能 `new T()`、不能 `new T[]`、静态方法不能直接用类上的 `T`。
- **PECS**：生产者 `extends`（读）、消费者 `super`（写）。
- **可变参数**：本质数组、只能一个、必须在最后、不传时是空数组。

## 12. 今天要记的 3 个术语（写进术语表）

从下面挑真的卡住的 3 个：

- 泛型（Generic）
- 类型擦除（Type Erasure）
- 通配符（Wildcard）
- 上界 / 下界（extends / super）
- PECS
- 可变参数（Varargs）

## 13. 面试一句话

> **Java 泛型是怎么实现的？什么是类型擦除？**
> Java 泛型是**编译期**的语法糖，通过类型擦除实现：编译完成后，所有类型参数都被替换成 `Object`（有上界则替换成上界），并自动插入强制转换，所以运行时 `List<String>` 和 `List<Integer>` 是同一个类。这带来三个限制：不能 `new T()`、不能创建泛型数组、不能用基本类型作为类型参数。擦除是为了兼容泛型出现之前的旧代码（向后兼容）。
