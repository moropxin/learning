# W4 D6+D7 · FileUtils 综合 + W4 复盘小测

> 2026-11-22（日）｜这是 W4 的最后一天（因为本周晚开始一天，D6 与 D7 合并）｜预计 3 小时

> 今天不是学新知识，是**把一个星期学的东西焊成一个能用的工具类**，然后用一张闭卷卷子验一遍。
> 学完今天，你应该能说出这句话："给你一个目录，我能写出代码统计它、搜索它、复制它。"

## 0. 今天会遇到的 6 个名词

| 名词 | 一句话 | 类比 |
| --- | --- | --- |
| 工具类（Utility Class） | 全是 `static` 方法、不放状态的类 | 一个工具箱，用完不用"记住"它 |
| `Files`（NIO） | JDK 7 提供的文件工具类，一行复制 | 官方出的"成品工具箱" |
| CSV | 逗号分隔的纯文本表格 | Excel 的"裸文本版" |
| 递归遍历 | 方法自己调用自己，钻进子目录 | 一层层打开文件夹 |
| 幂等 | 执行一次和执行多次结果一样 | 保存按钮按两次，文件不会变两倍大 |
| 防御性编程 | 先判空、先判存在，再干活 | 出门先看钥匙带没带 |

## 1. 今天 3 小时怎么分

| 分钟 | 做什么 |
| --- | --- |
| 90 | 读 §2–§4，把 `FileUtils.java` 完整敲一遍并跑通 |
| 40 | 动手任务 A（成绩 CSV 闭环）+ B（整目录复制） |
| 35 | §6 闭卷小测 10 题（不许翻前面几天的讲义） |
| 15 | 对照答案批改 + 填错题回炉表 + 写 3 个术语 + commit |

## 2. 为什么今天要写工具类

前五天你写的每一段 IO 代码，都是"一次性"的：这段读文件、那段复制文件，换个地方又要重写一遍。真实项目（包括后面苍穹外卖、黑马点评）里，文件操作会被反复调用——上传菜品图片、导出订单、读配置。如果每次都现写 `try-catch` 加三层流嵌套，代码里会到处是重复。

工具类的本质是把"**怎么做**"藏起来，只留"**做什么**"：

```java
// 没有工具类：每次都要写 6 行
try (BufferedReader r = new BufferedReader(new InputStreamReader(new FileInputStream(f), UTF_8))) { ... }

// 有工具类：一行
List<String> lines = FileUtils.readLines(f);
```

## 3. FileUtils 的设计：先定签名，再写实现

写工具类的顺序永远是"**先想清楚要哪些方法、参数是什么、返回什么、出错怎么办**"，再动手写。

| 方法 | 做什么 | 出错怎么办 |
| --- | --- | --- |
| `copyFile(File src, File dest)` | 复制文件（自动建父目录） | 抛 `UncheckedIOException` |
| `readLines(File f)` | 按 UTF-8 读成 `List<String>` | 同上 |
| `writeLines(File f, List<String> lines)` | 按 UTF-8 覆盖写 | 同上 |
| `appendLine(File f, String line)` | 追加一行（日志场景） | 同上 |
| `countChars(File f)` | 统计非空白字符数 | 同上 |
| `loadProps(File f)` | 读 `.properties` 成 `Properties` | 同上 |
| `listAll(File dir)` | 递归列出所有文件 | 目录不存在返回空 list |
| `groupByExt(File dir)` | 按扩展名分组 | 同上 |
| `search(File dir, String kw)` | 文件名或内容含关键词的文件 | 同上 |

三个设计决定，你要能讲出理由：

1. **全 `static`**：工具类不持有状态，`new FileUtils()` 没有意义，所以私有构造器 + 全静态。
2. **异常包装成 `UncheckedIOException`**：`IOException` 是编译时异常，如果每个方法都 `throws IOException`，调用方会被迫写一堆 `try-catch`，工具类就没起到"简化"的作用。包装成运行时异常，需要处理的调用方再 catch。
3. **不让调用方管流**：流在方法内部开、内部关（`try-with-resources`），调用方永远看不到 `InputStream`。

## 4. 完整实现

```java
// 文件：FileUtils.java
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.*;

/** IO 工具类：全部 static，不持有状态。 */
public final class FileUtils {

    private FileUtils() { }   // 私有构造器：禁止 new

    /** 复制文件，自动创建目标父目录。 */
    public static void copyFile(File src, File dest) {
        if (src == null || !src.isFile()) {
            throw new IllegalArgumentException("源文件不存在：" + src);
        }
        try {
            File parent = dest.getParentFile();
            if (parent != null) {
                parent.mkdirs();          // 多级目录一次建好
            }
            try (InputStream in = new FileInputStream(src);
                 OutputStream out = new FileOutputStream(dest)) {
                byte[] buf = new byte[8192];   // 8KB 缓冲，和 BufferedInputStream 默认值一致
                int len;
                while ((len = in.read(buf)) != -1) {
                    out.write(buf, 0, len);    // 必须写 len，不能写 buf.length
                }
            }
        } catch (IOException e) {
            throw new UncheckedIOException("复制失败：" + src, e);
        }
    }

    /** 按 UTF-8 读取所有行。 */
    public static List<String> readLines(File f) {
        List<String> lines = new ArrayList<>();
        try (BufferedReader r = new BufferedReader(
                new InputStreamReader(new FileInputStream(f), StandardCharsets.UTF_8))) {
            String line;
            while ((line = r.readLine()) != null) {   // 结束是 null，不是 ""
                lines.add(line);
            }
        } catch (IOException e) {
            throw new UncheckedIOException("读取失败：" + f, e);
        }
        return lines;
    }

    /** 按 UTF-8 覆盖写入所有行，每行自动加换行。 */
    public static void writeLines(File f, List<String> lines) {
        File parent = f.getParentFile();
        if (parent != null) {
            parent.mkdirs();
        }
        try (BufferedWriter w = new BufferedWriter(
                new OutputStreamWriter(new FileOutputStream(f), StandardCharsets.UTF_8))) {
            for (String line : lines) {
                w.write(line);
                w.newLine();          // 跨平台换行
            }
        } catch (IOException e) {
            throw new UncheckedIOException("写入失败：" + f, e);
        }
    }

    /** 追加一行（日志场景）。 */
    public static void appendLine(File f, String line) {
        File parent = f.getParentFile();
        if (parent != null) {
            parent.mkdirs();
        }
        try (BufferedWriter w = new BufferedWriter(
                new OutputStreamWriter(new FileOutputStream(f, true), StandardCharsets.UTF_8))) {
            w.write(line);
            w.newLine();
        } catch (IOException e) {
            throw new UncheckedIOException("追加失败：" + f, e);
        }
    }

    /** 统计非空白字符数（跳过空格、换行、制表符）。 */
    public static int countChars(File f) {
        int count = 0;
        for (String line : readLines(f)) {
            for (int i = 0; i < line.length(); i++) {
                char c = line.charAt(i);
                if (!Character.isWhitespace(c)) {
                    count++;
                }
            }
        }
        return count;
    }

    /** 读取 .properties 配置（用 Reader，避免中文乱码）。 */
    public static Properties loadProps(File f) {
        Properties props = new Properties();
        try (Reader r = new InputStreamReader(new FileInputStream(f), StandardCharsets.UTF_8)) {
            props.load(r);
        } catch (IOException e) {
            throw new UncheckedIOException("读取配置失败：" + f, e);
        }
        return props;
    }

    /** 递归列出目录下所有文件（不含目录）。 */
    public static List<File> listAll(File dir) {
        List<File> result = new ArrayList<>();
        collect(dir, result);
        return result;
    }

    /** 递归的"干活"部分：自己调用自己。 */
    private static void collect(File dir, List<File> out) {
        if (dir == null || !dir.exists()) {
            return;
        }
        File[] files = dir.listFiles();     // 可能返回 null（权限/不存在）
        if (files == null) {
            return;
        }
        for (File f : files) {
            if (f.isDirectory()) {
                collect(f, out);            // 目录就钻进去
            } else {
                out.add(f);                 // 文件就收下
            }
        }
    }

    /** 按扩展名分组，如 {"java": [...], "txt": [...]}。 */
    public static Map<String, List<File>> groupByExt(File dir) {
        Map<String, List<File>> map = new TreeMap<>();
        for (File f : listAll(dir)) {
            String name = f.getName();
            int dot = name.lastIndexOf('.');
            String ext = (dot < 0) ? "无扩展名" : name.substring(dot + 1).toLowerCase();
            map.computeIfAbsent(ext, k -> new ArrayList<>()).add(f);
        }
        return map;
    }

    /** 在文件名或内容里搜索关键词（区分大小写）。 */
    public static List<File> search(File dir, String keyword) {
        List<File> hits = new ArrayList<>();
        for (File f : listAll(dir)) {
            if (f.getName().contains(keyword)) {
                hits.add(f);
                continue;
            }
            try {
                for (String line : readLines(f)) {
                    if (line.contains(keyword)) {
                        hits.add(f);
                        break;
                    }
                }
            } catch (UncheckedIOException e) {
                // 二进制文件读不了，跳过（图片/压缩包）
            }
        }
        return hits;
    }

    /** 演示：先用最原始的写法，和上面工具类对照。 */
    public static void main(String[] args) {
        File src = new File("demo/src.txt");
        File dest = new File("demo/out/copy.txt");

        writeLines(src, List.of("第一行", "第二行", "第三行"));
        copyFile(src, dest);

        System.out.println("复制后的内容：" + readLines(dest));
        System.out.println("非空白字符数：" + countChars(dest));
        System.out.println("目录下所有文件：" + listAll(new File("demo")).size() + " 个");
        System.out.println("按扩展名分组：" + groupByExt(new File("demo")).keySet());
    }
}
// 输出：
// 复制后的内容：[第一行, 第二行, 第三行]
// 非空白字符数：9
// 目录下所有文件：2 个
// 按扩展名分组：[txt]
```

> ⚠️ 上面 `main` 里的输出路径是 `demo/src.txt`，第一次跑会自己在当前目录建 `demo` 文件夹。如果报"文件不存在"，先确认你在哪个目录下运行的（`System.getProperty("user.dir")` 打印一下）。

## 5. 动手任务 A–D（40 分钟）

### A. 成绩 CSV 闭环（核心任务）

```java
// 文件：ScoreDemo.java
import java.io.File;
import java.util.*;

public class ScoreDemo {

    static class Student {
        String name;
        int score;
        Student(String name, int score) { this.name = name; this.score = score; }
        @Override public String toString() { return name + ":" + score; }
    }

    public static void main(String[] args) {
        File csv = new File("demo/scores.csv");

        // 1. 写：内存对象 -> CSV 文本
        List<Student> students = List.of(
                new Student("张三", 88),
                new Student("李四", 92),
                new Student("王五", 75),
                new Student("赵六", 59));
        List<String> lines = new ArrayList<>();
        lines.add("name,score");                 // 表头
        for (Student s : students) {
            lines.add(s.name + "," + s.score);
        }
        FileUtils.writeLines(csv, lines);

        // 2. 读：CSV 文本 -> 内存对象
        List<Student> parsed = new ArrayList<>();
        List<String> raw = FileUtils.readLines(csv);
        for (int i = 1; i < raw.size(); i++) {   // 跳过表头
            String[] parts = raw.get(i).split(",");
            parsed.add(new Student(parts[0], Integer.parseInt(parts[1])));
        }

        // 3. 统计：平均分 / 最高分 / 不及格
        int sum = 0, max = Integer.MIN_VALUE;
        List<String> failed = new ArrayList<>();
        for (Student s : parsed) {
            sum += s.score;
            max = Math.max(max, s.score);
            if (s.score < 60) {
                failed.add(s.name);
            }
        }
        double avg = (double) sum / parsed.size();

        // 4. 排序：按分数降序
        parsed.sort((a, b) -> b.score - a.score);

        // 5. 写报告
        List<String> report = new ArrayList<>();
        report.add("=== 成绩报告 ===");
        report.add("人数：" + parsed.size());
        report.add(String.format("平均分：%.1f", avg));
        report.add("最高分：" + max);
        report.add("不及格：" + failed);
        report.add("排名：" + parsed);
        FileUtils.writeLines(new File("demo/report.txt"), report);

        FileUtils.readLines(new File("demo/report.txt")).forEach(System.out::println);
    }
}
// 输出：
// === 成绩报告 ===
// 人数：4
// 平均分：78.5
// 最高分：92
// 不及格：[赵六]
// 排名：[李四:92, 张三:88, 王五:75, 赵六:59]
```

**验收**：能解释"为什么读回来必须跳过第 1 行"、"`split(",")` 遇到名字里带逗号会怎样"。

### B. 整目录复制（递归）

```java
// 文件：CopyDirDemo.java
import java.io.File;

public class CopyDirDemo {

    public static void main(String[] args) {
        File srcDir = new File("demo");
        File destDir = new File("demo-backup");
        copyDir(srcDir, destDir);
        System.out.println("复制完成，目标文件数：" + FileUtils.listAll(destDir).size());
    }

    /** 递归复制目录：目录就建目录再钻进去，文件就复制。 */
    static void copyDir(File src, File dest) {
        if (!src.isDirectory()) {
            FileUtils.copyFile(src, dest);
            return;
        }
        dest.mkdirs();                       // 先把自己建出来
        File[] children = src.listFiles();
        if (children == null) {
            return;
        }
        for (File child : children) {
            copyDir(child, new File(dest, child.getName()));   // 相对路径拼出对称结构
        }
    }
}
// 输出：
// 复制完成，目标文件数：2 个
```

**验收**：能说出 `new File(dest, child.getName())` 为什么不能写成 `new File(child.getName())`。

### C. 日志追加（每跑一次多一行）

```java
// 文件：LogDemo.java
import java.io.File;
import java.time.LocalDateTime;

public class LogDemo {
    public static void main(String[] args) {
        File log = new File("demo/app.log");
        for (int i = 1; i <= 3; i++) {
            FileUtils.appendLine(log, LocalDateTime.now() + " INFO 第 " + i + " 次运行");
        }
        FileUtils.readLines(log).forEach(System.out::println);
    }
}
// 输出（时间每次不同）：
// 2026-11-22T14:03:11.512 INFO 第 1 次运行
// 2026-11-22T14:03:11.513 INFO 第 2 次运行
// 2026-11-22T14:03:11.514 INFO 第 3 次运行
```

### D. 统计代码行数（为后面读源码做准备）

用 `groupByExt` 找出某个目录下所有 `.java` 文件，统计：总文件数、总行数、空行数。写进 `demo/code-stat.txt`。

> 这个任务没有给代码——**这次你要自己拼**：`listAll` + `groupByExt` + `readLines` 三个方法组合起来就是答案。拼不出来就说明前面几个方法只是"敲过"，没"会过"。

## 6. W4 复盘小测（闭卷，35 分钟，满分 20）

> 规则：合上讲义，只开一个空白文件写答案。写不出来的可以直接写"不会"，不要猜——猜对的分是假的。

**1.（2 分）** `Error` 和 `Exception` 的区别是什么？举一个 `Error` 的例子。

> 答：Error是JVN系统级别崩溃(`OutOfMemoryError`、`StackOverflowError`),程序通常无法恢复，Exception是编译级别错误，可以自行解决

**2.（2 分）** 编译时异常和运行时异常，哪一个必须显式处理？为什么 Java 要这么设计？

> 答：编译时异常必须显式处理，因为编译异常时程序无法正常运行

**3.（2 分）** `try-with-resources` 相比手写 `finally` 里的 `close()`，好在哪两点？

> 答：N1.自动按次序关闭资源，代码清晰简洁
>     N2.close()抛异常不会覆盖主异常，会进getSussessed()

**4.（2 分）** 下面这段代码最终输出什么？为什么？

```java
static int f() {
    try { return 1; } finally { return 2; }
}
```

> 答：输出2，因为对于try的基本类型变量，finally的return可以覆盖

**5.（2 分）** 字节流和字符流的本质区别是什么？什么时候只能用字节流？

> 答：字节流一个一个读字节，字符流一段一段读字节。读二进制、视频、图片等文件时

**6.（2 分）** `new String(bytes, StandardCharsets.UTF_8)` 和 `new String(bytes)` 有什么区别？后者为什么会乱码？

> 答：前者可指定编码样式，后者使用系统默认编码。当读取字段编码类型和系统编码不一致时会出现乱码

**7.（2 分）** 缓冲流为什么必须 `flush()` 或 `close()`？`close()` 和 `flush()` 的关系是什么？

> 答：因为缓存区里不满8kb的数据仍存在，不flush()或close()的话该部分数据会随流关闭消失。close()会先执行flush()再执行自身，通俗而言close()是flush()的父类

**8.（2 分）** 对象序列化时，为什么 `transient` 字段读回来是 `null`？它和 `static` 不序列化的原因有何不同？

> 答：因为transient字段不可被序列化，transient指定的对象字段不被读进文件里，因此读回来的值是默认值null，static是类不是对象，在类加载时已经存在，无法被new出来，因此序列化没意义

**9.（2 分）** `Properties.load(InputStream)` 读中文为什么乱码？正确写法是什么？

> 答：因为load(inputstream)默认编码样式是Latin1,因此解码时中文会乱码。
>     正确写法是Properties.load(new InputStreamReader(new FileInputStream(f), StandardCharsets.UTF_8))

**10.（2 分）** 你写了一个工具类 `FileUtils.readLines(File)`，方法内部抛 `IOException` 时你包装成了 `UncheckedIOException` 再抛出。这样做的好处和坏处各是什么？

> 答：**优点**包装后出现异常时不用到处catch，真正实现工具类只调用的功能。
>     **缺点**当程序出现编译异常的时候不会立马抛异常，通常往后的时间点才会抛，因此要显式catch业务异常(如businessexception)

<details>
<summary>参考答案（批改完再看）</summary>

**1.** `Error` 是 JVM 层面的严重问题（`OutOfMemoryError`、`StackOverflowError`），程序通常无法恢复，不该 catch；`Exception` 是程序可以处理的异常。

**2.** 编译时异常（`IOException`、`SQLException`）必须显式处理（catch 或 throws），因为它们是"可预期的外部故障"；运行时异常（`NullPointerException`）不强制，因为是"代码 bug"，应该在开发期修掉而不是到处 catch。

**3.** ① 自动关闭，且**多个资源按逆序关闭**；② 资源关闭时的异常不会覆盖主异常，而是进 `getSuppressed()`。

**4.** 输出 **2**。`finally` 里的 `return` 会覆盖 `try` 里的 `return`，而且 `finally` 一定会执行（除非 `System.exit()`）。这就是"不要在 finally 里写 return"的原因。

**5.** 字节流以 `byte` 为单位（`InputStream`/`OutputStream`），字符流以 `char` 为单位并带编码（`Reader`/`Writer`）。**二进制文件（图片、视频、压缩包、class 文件）只能用字节流**——字符流会尝试解码，直接把文件搞坏。

**6.** 前者用 UTF-8 解码，后者用**平台默认编码**（Windows 中文版是 GBK）。同一个 UTF-8 字节数组用 GBK 解，就是"锟斤拷"式的乱码。

**7.** 缓冲区只在**满**或**显式 flush** 时才真正写磁盘，最后不满一块的数据留在内存里，进程结束就丢了。`close()` = 先 `flush()` 再释放文件句柄。

**8.** `transient` 是"这个字段属于对象，但我不想存"，所以不写进文件，读回来自然是对象类型的默认值 `null`；`static` 是"这个字段根本不属于对象，属于类"，序列化保存的是对象快照，类级别的字段不该进快照。

**9.** `load(InputStream)` 固定按 ISO-8859-1（Latin1）解码，中文必然乱码。正确写法：`props.load(new InputStreamReader(new FileInputStream(f), StandardCharsets.UTF_8))`。

**10.** 好处：调用方不用到处写 `try-catch`，工具类才能真正"简化代码"；坏处：**丢失了编译期检查**，调用方可能根本不知道这里会抛异常，运行到一半才崩。所以关键业务里要显式 catch 并给出用户能懂的提示（呼应 day2 的 `BusinessException`）。

</details>

## 7. 错题回炉表（小测后 15 分钟内填）

| 题号 | 得分 | 错在哪（一句话） | 回哪看 | ☐ 已订正 |
| --- | --- | --- | --- | --- |
| 1 | 1.0 | 把 Exception 说成"编译级别错误"——它是所有可处理异常的父类，含运行时异常 | day1 §异常体系 | ☐ |
| 2 | 1.5 | 理由停在"编译不过"，没答到"可预期的外部故障，强制你处理" | day1 §三类异常 | ☐ |
| 3 | 1.5 | 写了"按次序关闭"，应为**逆序**（后开先关） | day2 §try-with-resources | ☐ |
| 4 | 2.0 | —（结论与理由都对） | day1 §finally 与 return | ☑ |
| 5 | 1.0 | 字符流是"按字符（带编码）"，不是"一段一段读字节" | day3 §字节流边界 | ☐ |
| 6 | 2.0 | —（默认编码 + 解码不一致，答得准） | day4 §编码与解码 | ☑ |
| 7 | 1.0 | "close 是 flush 的父类"错——是 close **内部先调** flush | day5 §缓冲流 | ☐ |
| 8 | 1.5 | static 的理由说成"无法被 new 出来"，应为"属于类、不进对象快照" | day5 §序列化 | ☐ |
| 9 | 2.0 | —（Latin1 + 正确写法，完整） | day5 §Properties | ☑ |
| 10 | 1.0 | 缺点应是"**丢失编译期检查**，调用方不知道会抛"，不是"往后才抛" | day2 §异常设计 | ☐ |

| **小计** | **14.5** | / 20 —— 判定：**良**（12–15.5） | — |

## 8. 高频坑

| 坑 | 说明 |
| --- | --- |
| `write(buf)` 不写长度 | 最后一次读不满 8KB 时，尾部会写进垃圾字节，文件变大且损坏 |
| `listFiles()` 不判 null | 目录不存在或无权限时返回 null，直接 for-each 会 NPE |
| 递归没有终止条件 | 符号链接会造成无限递归；`if (!dir.exists()) return` 是最低保护 |
| 工具类方法里悄悄吞异常 | `catch (IOException e) { }` 是万恶之源，至少打印或包装重抛 |
| CSV 用 `split(",")` | 字段里含逗号或引号就崩，真实场景要用 `split(",(?=(?:[^\"]*\"[^\"]*\")*[^\"]*$)")` 或专用库 |
| 路径里硬编码 `\` | Windows 能跑，Linux 上就断，统一用 `/` 或 `File.separator` |

## 9. 30 秒卡

- **工具类三原则**：全 static、私有构造器、异常包装成运行时。
- **文件操作五件套**：`copyFile` / `readLines` / `writeLines` / `appendLine` / `listAll`。
- **递归两要素**：终止条件（`exists`/`listFiles == null`）+ 自调用（拼子路径）。
- **编码永远写死** `StandardCharsets.UTF_8`，别信平台默认。
- **`close()` = flush + 释放句柄**，try-with-resources 会自动帮你做。

## 10. 今天要记的 3 个术语（写进术语表）

- 工具类 / 私有构造器 / `UncheckedIOException`
- 递归遍历 / `listFiles()` 返回 null
- CSV / 幂等写入 / 追加模式 `new FileOutputStream(f, true)`

## 11. 面试一句话

> **你平时怎么写文件操作的代码？**
> "我一般先抽一个 `FileUtils` 工具类，把复制、按行读写、递归遍历、按扩展名分组这些收进去，方法全 `static`、私有构造器，`IOException` 统一包装成 `UncheckedIOException`，这样业务代码只关心'做什么'。编码一律写死 `UTF-8`，流全部用 `try-with-resources` 保证关闭。读配置文件用 `load(Reader)` 而不是 `load(InputStream)`，因为后者固定按 Latin1 解码，中文会乱码。复制大文件用 8KB 数组或 `Files.copy`，逐字节复制在 5MB 文件上会慢两个数量级。"

## 12. 明天预告

**W5 D1（11/23，周一）：多线程入门**——线程的三种创建方式、`Thread` 常用方法、以及那个面试必问的问题："为什么 `start()` 而不是 `run()`？"

> 📌 W4 到此收官：异常 + IO 一共 6 天，产出 `FileUtils` 工具类 + 成绩 CSV 闭环 + 一份小测。明天开始 W5，节奏会变快——多线程和反射只留"够面试 + 够看懂框架"的深度，不再逐章细抠。

---

## 附：批改与点评（11/23 阅）

### 得分表

| 题号 | 主题 | 得分 | 满分 |
| --- | --- | --- | --- |
| 1 | Error vs Exception | 1.0 | 2 |
| 2 | 编译期异常为什么强制处理 | 1.5 | 2 |
| 3 | try-with-resources 两点好处 | 1.5 | 2 |
| 4 | finally 里的 return | 2.0 | 2 |
| 5 | 字节流 vs 字符流 | 1.0 | 2 |
| 6 | 默认编码为什么乱码 | 2.0 | 2 |
| 7 | flush / close 关系 | 1.0 | 2 |
| 8 | transient vs static | 1.5 | 2 |
| 9 | Properties 编码 | 2.0 | 2 |
| 10 | 包装成运行时异常的利弊 | 1.0 | 2 |
| **合计** | | **14.5** | **20** |

**判定：良（12–15.5）**。W4 六天批改走势：15.5 / 14.5 / 16 / 14.5 / 15 / **14.5**。

### 逐题批注

**1（扣 1）** Error 部分答得很准。问题在"**Exception 是编译级别错误**"——Exception 是**所有可处理异常的父类**，里面既有编译时异常（`IOException`）也有运行时异常（`NullPointerException`）。"编译期 / 运行期"是对异常**再分类**的维度，不是 Exception 的定义。正确说法：`Throwable` 下分 `Error` 和 `Exception`，Exception 下再分编译时与运行时。

**2（扣 0.5）** "必须显式处理"对。理由"编译异常时程序无法正常运行"其实是**结果**（编译不过），不是**设计目的**。Java 这么设计是因为 IO/SQL 这类是**可预期的外部故障**（文件被删、网络断），强制你写处理逻辑，避免"能编译、一上线就崩"。

**3（扣 0.5）** 第二点 `getSuppressed()` 答得很好（拼写是 `getSuppressed`，不是 `getSussessed`）。第一点写"按次序关闭"——准确说是**逆序**（后打开的先关闭），因为资源之间可能有依赖（比如 `BufferedWriter` 依赖 `FileOutputStream`，必须先关外层）。

**4（满分）** 结论 2、理由"finally 的 return 覆盖 try 的 return"都对。小瑕疵："对于 try 的基本类型变量"这句没必要，覆盖和基本类型无关——是**返回值被 finally 的 return 覆盖**，引用类型一样。

**5（扣 1）** 场景对了（二进制/图片/视频只能用字节流）。但定义错："字符流**一段一段读字节**"——字符流是按**字符**读（内部带解码器，一次处理 1–4 个字节拼成一个 char）。你这句话把字符流说成了"批量字节流"，恰好丢掉它存在的理由：**处理文字和编码**。

**7（扣 1）** 前半完全对（不满 8KB 留在缓冲区，不刷就丢）。后半"**close() 是 flush() 的父类**"是这一份里最需要改的一句——`close()` 和 `flush()` 不是继承关系，是**包含关系**：`close()` 的实现里**先调用 flush()**，再释放文件句柄。可以记成"close = flush + 关文件"。

**8（扣 0.5）** `transient` 答得对。`static` 的理由"无法被 new 出来"错位——静态字段**可以被访问**，它不序列化的原因是"**属于类、不属于任何一个对象**"，而序列化保存的是对象状态快照，往里塞类级别的字段既冗余又会产生"读回来要不要覆盖类变量"的歧义。

**10（扣 1）** 优点对。缺点写"编译异常不会立马抛，往后的时间点才抛"，方向偏了——真正的缺点不是"抛得晚"，而是"**丢失了编译期检查**"：本来编译器会逼调用方处理，包装成运行时异常后，调用方可能根本不知道这里会抛，运行到一半才崩。你后半句提到"要显式 catch 业务异常"是对的补救，但要先把"丢失编译期检查"说清楚。

### 整体评价

**这一份暴露的是"定义层面"的模糊，不是"操作层面"的不会。** 凡是"怎么做"的题（4、6、9）你都满分；凡是"是什么 / 为什么这么设计"的题（1、5、7）你都在扣分。这是典型的"会用不会讲"——平时敲代码够用，面试时会被追问穿。

三个高频错点，按优先级：
1. **`close()` 与 `flush()` 的关系**（这道错了两次，day5 的 C1 也是没讲清）
2. **Exception 的层级结构**（Error / Exception / 编译时 / 运行时，四个词的关系要能画出来）
3. **字符流为什么存在**（不是"批量字节"，是"按字符 + 处理编码"）

### 回炉任务（15 分钟内，做完打勾）

- [ ] 默画异常体系：`Throwable` → `Error` / `Exception` → 编译时 / 运行时，每层各举 2 个类名
- [ ] 用一句话说清 `close()` 和 `flush()` 的关系（不许出现"父类"两个字）
- [ ] 用一句话说清"字符流存在的理由"（不许出现"一段一段"）