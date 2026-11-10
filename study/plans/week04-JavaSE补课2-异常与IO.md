# W4 · JavaSE 补课②：异常 + IO 流

> 2026-11-16（一）– 11-22（日）｜档位：A 基础｜里程碑：**M2 能安全读写文件**

> 异常和 IO 是"写业务代码最早用上、也最早埋坑"的两块。IO 学不好，后面文件上传、Excel 导入、日志、序列化全卡；异常学不好，代码里会到处 `e.printStackTrace()`，面试一问"你项目里怎么处理异常"就露怯。

## 一、本周目标

1. 分清 `Error` / `Exception`、受检 / 非受检，知道什么时候 `throws`、什么时候 `try-catch`。
2. 会写自定义异常，会用 `try-with-resources` 自动关资源。
3. 会用 `File` 类做路径操作（创建、判断、遍历目录）。
4. 字节流 / 字符流各一套会用；知道什么时候必须用字符流（中文）。
5. 会缓冲流、序列化、`Properties` 读配置；能写一个"文件工具类"。

## 二、时间节奏（A 档：看课 85 + 敲代码 55 + 算法 25 + 记录 15 = 180 分钟）

| 时间块 | 分钟 | 做什么 |
| --- | --- | --- |
| 看课 | 85 | 黑马「异常」「IO 流」对应章节，1.25 倍速 |
| 敲代码 | 55 | 当天代码自己写，**每天至少一次"文件读 → 处理 → 写"的完整闭环** |
| 算法 | 25 | 力扣 1 题（字符串/数组） |
| 记录 | 15 | 笔记 3 条 + **术语 3 个**（追加进 [术语表](Java/00-JavaSE总体路线与术语表.md)）+ commit + 日志 |

## 三、每日任务

| 天 | 今日目标 | 看课章节 | 动手任务 | 验收标准 |
| --- | --- | --- | --- | --- |
| 一 11/16 | 异常体系 | 异常概述、`try-catch`、`throws`、`finally` | 写 5 个会抛异常的示例（空指针、数组越界、类型转换、数字格式、IO），分别捕获并打印"是什么、为什么、怎么修" | 能说清 `finally` 与 `return` 的关系 |
| 二 11/17 | 自定义异常 + 资源关闭 | 自定义异常、`try-with-resources`、多异常捕获 | 定义 `BusinessException`（带错误码）；写一个"读文件失败 → 包装成业务异常"的方法 | 代码里没有裸的 `e.printStackTrace()` |
| 三 11/18 | `File` + 字节流 | `File` 类、`FileInputStream`/`OutputStream` | 用字节流复制一张图片；遍历某个目录，打印所有 `.java` 文件 | 图片复制后能打开，大小一致 |
| 四 11/19 | 字符流 + 编码 | `FileReader`/`Writer`、`InputStreamReader`、编码表 | 用字符流写一个"GBK 文件转 UTF-8"的小工具；故意用错编码读一次，看乱码长什么样 | 能解释"为什么字节流读中文会乱码" |
| 五 11/20 | 缓冲流 + 对象流 + `Properties` | `BufferedReader`/`Writer`、序列化、`Properties` | 用缓冲流按行读一个大文本并统计行数；把一个 `Student` 对象序列化再反序列化；用 `Properties` 读 `config.properties` | 能说出 `transient` 的作用 |
| 六 11/21 | 综合：文件工具类 | 本周笔记 | 写 `FileUtils`：`copyDir`（递归复制目录）、`countWords`（统计文件词频）、`appendLog`（追加写日志） | 三个方法都能跑；异常都处理了 |
| 日 11/22 | 复盘 + 小测 | 本周笔记 | 闭卷小测 10 题；把 `FileUtils` 用 try-with-resources 重构一遍 | 10 题 ≥7 分 |

## 四、本周产出

- `Java/00-JavaSE总体路线与术语表.md`：本周追加术语 ≥15 个（每天 3 个，只写自己的话）

- `study/notes/java-异常与IO.md`：异常体系图 + 字节/字符流对照表 + 编码表速查
- `study/projects/io-practice/`：`FileUtils`（复制目录/词频/日志）+ 5 个异常示例类
- `PROGRESS.md`：W4 打卡 + 日志

## 五、自查 3 问 + 1 个术语关（周日口述）

1. 受检异常和非受检异常，分别该怎么处理？为什么 `IOException` 必须处理而 `NullPointerException` 不用？
2. 什么情况下必须用字符流，不能只用字节流？
3. `try-with-resources` 相比 `finally` 里 `close()`，好在哪？

- [ ] 4. 本周新出现的名词，你能用一句话讲清 80% 吗？（对照 `Java/00-JavaSE总体路线与术语表.md`）

## 六、高频坑 / 减法清单

| 坑 | 说明 |
| --- | --- |
| 捕获后什么都不做 | `catch (Exception e) {}` 是最糟的写法，至少记日志或包装重抛 |
| 用 `e.printStackTrace()` 当日志 | 生产环境输出到 stderr，收集不到；用日志框架 |
| 字节流读中文 | `FileInputStream` 按字节读，一个汉字 3 字节会被截断 → 乱码，必须字符流或指定编码 |
| 忘记关流 | 用 try-with-resources（JDK 7+），不要手写 finally |
| 相对路径 | `new File("a.txt")` 是相对**工作目录**，不是 class 文件目录，调试时经常找不到文件 |
| 序列化类改了字段 | 不写 `serialVersionUID` 会抛 `InvalidClassException` |

**减法顺序**：先砍「对象流/序列化」→ 再砍「`Properties`」→ 再砍「GBK 转码工具」。**`File`、字节流、字符流、缓冲流不能砍**，JavaWeb 上传下载全靠它。

## 七、面试一句话

> **`try-with-resources` 的原理是什么？**
> 资源类实现 `AutoCloseable`（或 `Closeable`）后，编译器会把 `try` 块展开成 `try-finally`，在 `finally` 里按**声明的逆序**调用 `close()`，并且会把关闭时的异常用 `addSuppressed()` 挂到主异常上，不会覆盖原始异常。所以它比手写 `finally` 更安全——手写时如果 `close()` 也抛异常，原始异常就丢了。
