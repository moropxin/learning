# W4 D4 · 字符流与编码

> 2026-11-20（五）｜W4 第 4 天｜档位 A 基础｜配套课程：黑马「IO」— `FileReader`/`FileWriter`、`InputStreamReader`、编码表
>
> 今天要拿到三样东西：**① 用字符流读写文本（不再一个字节一个字节拼中文）；② 能指定编码读写文件；③ 写一个"GBK 文件转 UTF-8"的小工具，并亲眼见一次乱码。**
>
> 昨天字节流读中文是 `ä½ å¥½`，今天要让它变成"你好"。这是所有"导入 CSV/导出 Excel、读写配置文件"的基础。

> 🧩 **代码块约定**：每个 `java` 代码块都是完整可运行的文件——复制 → 存成 `// 文件：` 后的名字 → 运行。测试文件由代码自己生成。

## 0. 今天会遇到的 8 个名词

| 名词 | 一句话人话 | 类比 |
| --- | --- | --- |
| 字符流 | 按**字符**搬运，内部帮你做编码解码 | 翻译陪同，不是搬箱子 |
| `Reader` / `Writer` | 字符流的两个顶层抽象类 | 读字 / 写字 |
| 编码（encode） | 字符 → 字节 | 说出去的话录成音频 |
| 解码（decode） | 字节 → 字符 | 音频放出来给人听 |
| 编码表（charset） | 字符和字节的对照表：ASCII / GBK / UTF-8 | 密码本 |
| UTF-8 | 变长编码：英文 1 字节、中文 3 字节 | 国际通用语 |
| GBK | 中文 2 字节（Windows 中文版默认） | 地方方言 |
| `flush()` / `close()` | 把缓冲区刷到磁盘 / 刷完再关闭 | 把水倒出来 / 倒完关龙头 |

## 1. 今天 3 小时怎么分

| 时段 | 分钟 | 做什么 | 产出 |
| --- | --- | --- | --- |
| 看课 | 85 | `FileReader`/`FileWriter`、`InputStreamReader`/`OutputStreamWriter`、编码表，1.25 倍速边看边敲 | 跟敲代码 |
| 敲代码 | 55 | 字符流读写 + GBK↔UTF-8 转换工具 + 乱码实验 | `io-demo/` 四个类 |
| 算法 | 25 | 力扣 1 题（哈希/字符串） | AC 记录 |
| 记录 | 15 | 笔记 3 条 + **术语 3 个**（写进 [术语表](../00-JavaSE总体路线与术语表.md)）+ commit + 日志 | 提交完成 |

## 2. 昨天的问题：一个汉字被拆成了三份

| 内容 | 编码 | 占字节 | 字节流的困境 |
| --- | --- | --- | --- |
| `A` | UTF-8 | 1 | 没问题 |
| `你` | UTF-8 | 3 | 读一次只拿到 1/3 个汉字，转 `char` 必乱码 |
| `你` | GBK | 2 | 读一次只拿到半个字 |

字符流的本质：**在字节流外面套一层"解码器"**，一次凑够一个完整字符再交给你。

```mermaid
flowchart LR
    F["文件字节"] --> IS["InputStream"]
    IS --> R["InputStreamReader<br/>按 charset 解码"]
    R --> RD["Reader<br/>给字符"]
    RD --> APP["你的代码"]
```

## 3. 先看清编码：三张表 + 一个默认值

```java
// 文件：CharsetInfoDemo.java
import java.nio.charset.Charset;

public class CharsetInfoDemo {

    public static void main(String[] args) throws Exception {
        // JDK 默认字符集（Windows 中文版通常是 GBK，Linux/macOS 多是 UTF-8）
        System.out.println("系统默认编码：" + Charset.defaultCharset());

        String text = "A你";

        // 同一段文字在不同编码下的字节数完全不同
        System.out.println("UTF-8 字节数：" + text.getBytes("UTF-8").length);   // 1 + 3
        System.out.println("GBK   字节数：" + text.getBytes("GBK").length);     // 1 + 2

        // 把字节打印出来，看清"编码"到底做了什么
        byte[] utf8 = text.getBytes("UTF-8");
        StringBuilder sb = new StringBuilder();
        for (byte b : utf8) {
            sb.append(String.format("%02X ", b));
        }
        System.out.println("UTF-8 的字节：" + sb);
    }
}
// 输出（JDK 17 + Windows 中文版）：
// 系统默认编码：GBK
// UTF-8 字节数：4
// GBK   字节数：3
// UTF-8 的字节：41 E4 BD A0
```

> **结论**：`A` 的 UTF-8 是 `41`，`你` 是 `E4 BD A0`——一个汉字三字节。所以字节流单独读一个字节是毫无意义的碎片。

## 4. `FileReader`：按字符读文本

```java
// 文件：FileReaderDemo.java
import java.io.FileWriter;
import java.io.FileReader;
import java.io.IOException;

public class FileReaderDemo {

    public static void main(String[] args) throws IOException {
        // 先造一个 UTF-8 文本
        try (FileWriter w = new FileWriter("hello.txt")) {
            w.write("第一行：你好，世界\n");
            w.write("第二行：Java IO\n");
        }

        // 方式 1：一次读一个字符（返回 int，-1 表示末尾）
        try (FileReader r = new FileReader("hello.txt")) {
            int c;
            StringBuilder sb = new StringBuilder();
            while ((c = r.read()) != -1) {
                sb.append((char) c);            // 这里拿到的是完整字符，不会乱码
            }
            System.out.println("方式 1 读到：\n" + sb);
        }

        // 方式 2：用 char[] 缓冲区（推荐，快得多）
        try (FileReader r = new FileReader("hello.txt")) {
            char[] buf = new char[1024];
            int len;
            StringBuilder sb = new StringBuilder();
            while ((len = r.read(buf)) != -1) {
                sb.append(buf, 0, len);         // 同样只取本次读到的 len 个字符
            }
            System.out.println("方式 2 读到：\n" + sb);
        }
    }
}
// 输出：
// 方式 1 读到：
// 第一行：你好，世界
// 第二行：Java IO
//
// 方式 2 读到：
// 第一行：你好，世界
// 第二行：Java IO
```

> ⚠️ `FileReader` 用的是**系统默认编码**。在 GBK 系统上读一个 UTF-8 文件照样乱码——所以要精确控制编码，用 §6 的 `InputStreamReader`。

## 5. `FileWriter`：写文本的三个细节

```java
// 文件：FileWriterDemo.java
import java.io.FileWriter;
import java.io.IOException;

public class FileWriterDemo {

    public static void main(String[] args) throws IOException {

        // 1. 默认覆盖：第二次 new 会清空文件
        try (FileWriter w = new FileWriter("out.txt")) {
            w.write("第一段\n");
        }
        try (FileWriter w = new FileWriter("out.txt")) {   // 覆盖了上面的内容
            w.write("第二段\n");
        }

        // 2. 追加模式：第二个参数 true
        try (FileWriter w = new FileWriter("out.txt", true)) {
            w.write("第三段（追加）\n");
        }

        // 3. flush vs close：flush 只把缓冲区刷到磁盘，流还能继续写
        try (FileWriter w = new FileWriter("out.txt", true)) {
            w.write("第四段");
            w.flush();          // 此刻内容已经落盘
            w.write("，还在写\n"); // flush 之后仍可继续写
        }

        // 看看结果
        try (FileWriter w = new FileWriter("out.txt", true)) {
            w.write("换行用 System.lineSeparator() 跨平台安全：" + System.lineSeparator());
        }
        System.out.println("写入完成，内容见 out.txt");
    }
}
// out.txt 最终内容：
// 第二段
// 第三段（追加）
// 第四段，还在写
// 换行用 System.lineSeparator() 跨平台安全：
```

| 细节 | 说明 |
| --- | --- |
| `new FileWriter(file)` | **覆盖**（清空原内容） |
| `new FileWriter(file, true)` | **追加** |
| `flush()` | 把缓冲区刷到磁盘，**流不关闭**，可以继续写 |
| `close()` | 先 flush 再释放资源；**不 close 可能丢数据** |
| `\n` vs `\r\n` | Windows 用 `\r\n`，Linux 用 `\n`，跨平台用 `System.lineSeparator()` |

## 6. 指定编码：`InputStreamReader` / `OutputStreamWriter`

这是今天最有用的两行代码：**字节流 + 编码 = 字符流**。

```java
// 文件:CharsetConvertDemo.java
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.IOException;
import java.nio.charset.Charset;

public class CharsetConvertDemo {

    public static void main(String[] args) throws IOException {

        // 1. 先造一个 GBK 编码的文件（模拟"别人给的 GBK 文件"）
        try (OutputStreamWriter w = new OutputStreamWriter(
                new FileOutputStream("gbk.txt"), Charset.forName("GBK"))) {
            w.write("姓名：张三\n城市：北京\n");
        }
        System.out.println("GBK 文件已生成，大小：" + new java.io.File("gbk.txt").length() + " 字节");

        // 2. 用 UTF-8 去读 GBK 文件 —— 故意读错，看乱码
        try (InputStreamReader r = new InputStreamReader(
                new FileInputStream("gbk.txt"), Charset.forName("UTF-8"))) {
            char[] buf = new char[256];
            int len = r.read(buf);
            System.out.println("用 UTF-8 读 GBK 文件：" + new String(buf, 0, len));
        }

        // 3. 用正确的 GBK 读，再以 UTF-8 写出 —— 转换完成
        try (InputStreamReader r = new InputStreamReader(
                new FileInputStream("gbk.txt"), Charset.forName("GBK"));
             OutputStreamWriter w = new OutputStreamWriter(
                     new FileOutputStream("utf8.txt"), Charset.forName("UTF-8"))) {
            char[] buf = new char[1024];
            int len;
            while ((len = r.read(buf)) != -1) {
                w.write(buf, 0, len);
            }
        }

        // 4. 验证：用 UTF-8 读回来
        try (InputStreamReader r = new InputStreamReader(
                new FileInputStream("utf8.txt"), Charset.forName("UTF-8"))) {
            char[] buf = new char[256];
            int len = r.read(buf);
            System.out.println("转换后用 UTF-8 读：" + new String(buf, 0, len));
        }
    }
}
// 输出：
// GBK 文件已生成，大小：24 字节
// 用 UTF-8 读 GBK 文件：���� ��� ��
// 转换后用 UTF-8 读：姓名：张三
// 城市：北京
```

**记住这一句**：`new InputStreamReader(字节流, 编码)` 和 `new OutputStreamWriter(字节流, 编码)` ——**想控制编码就用它们，不要用 `FileReader`/`FileWriter`**（它们只认系统默认编码）。

## 7. 乱码实验：把三种情况一次看清

```java
// 文件：GarbledDemo.java
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;

public class GarbledDemo {

    public static void main(String[] args) throws IOException {
        String text = "中文测试";

        // 用 UTF-8 写出
        try (FileOutputStream out = new FileOutputStream("g.txt")) {
            out.write(text.getBytes(StandardCharsets.UTF_8));
        }

        byte[] bytes = java.nio.file.Files.readAllBytes(new File("g.txt").toPath());
        System.out.println("文件实际字节：" + bytes.length + " 个");

        // 正确解码
        System.out.println("按 UTF-8 解：" + new String(bytes, StandardCharsets.UTF_8));
        // 用 GBK 解 UTF-8 的字节 → 乱码
        System.out.println("按 GBK   解：" + new String(bytes, Charset.forName("GBK")));
        // 用 ISO-8859-1 解 → 每个字节变一个字符，也是乱码
        System.out.println("按 Latin1 解：" + new String(bytes, StandardCharsets.ISO_8859_1));
    }
}
// 输出：
// 文件实际字节：12 个
// 按 UTF-8 解：中文测试
// 按 GBK   解：涓枃娴嬭瘯
// 按 Latin1 解：ä¸­æ–‡æµ‹è¯•
```

**乱码的唯一原因**：**写出用的编码 ≠ 读入用的编码**。排查顺序永远是：先确定文件真实编码 → 再用同一编码读。

## 8. 动手任务（55 分钟）

**任务 A：跑通今天全部示例（20 分钟）**
`CharsetInfoDemo` → `FileReaderDemo` → `FileWriterDemo` → `CharsetConvertDemo` → `GarbledDemo`，把 §7 的三种乱码输出抄进笔记。

**任务 B：写 `convert(File src, Charset from, File dest, Charset to)`（15 分钟）**
通用转换方法：用 `InputStreamReader(src流, from)` + `OutputStreamWriter(dest流, to)`，1024 字符缓冲，返回转换的字符数。

**任务 C：统计文本文件的行数与字数（10 分钟）**
用 `InputStreamReader` 按 UTF-8 读一个中文文件，统计：行数（`\n` 个数）、字符数、最长一行长度。禁止用 `Files.readAllLines`（练流式读）。

**任务 D：给昨天复制的图片"读中文"（10 分钟）**
用 `FileReader` 去读一个 `.png` 文件，观察会发生什么（`MalformedInputException` 或乱码），理解"字符流只能用于文本"。

**验收标准**

- [ ] 能说出 UTF-8 和 GBK 下一个汉字各占几字节
- [ ] 能说出 `FileReader` 用的编码是什么（系统默认），以及为什么它会坑
- [ ] 能写出 `new InputStreamReader(字节流, 编码)` 两行式
- [ ] 能说出乱码的唯一原因和排查顺序

## 9. 练习（先写，再看答案）

### A. 判断（5 题）

**A1.** `FileReader` 默认使用 UTF-8 编码读取文件。

> 答：

**A2.** 一个汉字在 UTF-8 中固定占 3 个字节。

> 答：

**A3.** `flush()` 之后流就被关闭了，不能再写。

> 答：

**A4.** `new FileWriter(file)` 会清空原文件内容。

> 答：

**A5.** 用 `InputStreamReader` 可以指定任意字符集，从而正确读取 GBK 文件。

> 答：

### B. 选择（5 题）

**B1.** 一个汉字在 GBK 编码下占几个字节：
A. 1　B. 2　C. 3　D. 4

> 答：

**B2.** 要把 GBK 文件转成 UTF-8，正确的组合是：
A. `FileReader` + `FileWriter`　B. `InputStreamReader(GBK)` + `OutputStreamWriter(UTF-8)`　C. `FileInputStream` + `FileOutputStream`　D. `BufferedReader` + `BufferedWriter`

> 答：

**B3.** `FileWriter` 以追加模式打开文件，构造器第二个参数应为：
A. `false`　B. `true`　C. 文件名　D. 编码

> 答：

**B4.** 下面哪种情况**不会**产生乱码：
A. UTF-8 写、GBK 读　B. GBK 写、GBK 读　C. UTF-8 写、Latin1 读　D. 字节流逐字节转 `char`

> 答：

**B5.** `close()` 和 `flush()` 的关系是：
A. 完全一样　B. `close()` 会先 flush 再释放资源　C. `flush()` 会关闭流　D. `close()` 不刷缓冲区

> 答：

### C. 简答（3 题）

**C1.** 为什么 `FileReader` 读 UTF-8 文件在某些机器上会乱码？

> 答：

**C2.** 乱码的根本原因是什么？你排查一个乱码文件会按什么顺序做？

> 答：

**C3.** 字节流和字符流分别适合什么场景？各举一例。

> 答：

<details>
<summary>参考答案</summary>

**A 判断**：A1 **错**（用系统默认编码，Windows 中文版是 GBK）｜A2 **错**（常用汉字多为 3 字节，但不是"固定"；生僻字/emoji 占 4 字节）｜A3 **错**（`flush` 只刷缓冲，流仍可用；`close` 才关闭）｜A4 **对**｜A5 **对**

**B 选择**：B1 B｜B2 B｜B3 B｜B4 B｜B5 B

**C1**：`FileReader` 内部用的是 `Charset.defaultCharset()`，也就是**运行环境的默认编码**。Windows 中文版默认是 GBK，如果文件本身是 UTF-8 写的，用 GBK 去解码就会乱码；而在 Linux/macOS（默认 UTF-8）上同一份代码就正常。所以代码在不同机器上表现不一致——这也是为什么生产代码里要显式指定编码（`InputStreamReader` + `StandardCharsets.UTF_8`）。

**C2**：根本原因是**写入时的字节序列用错误的编码去解码**（写出编码 ≠ 读入编码），或者按字节切割了多字节字符。排查顺序：① 先用十六进制/`file` 命令或文本工具确认文件**真实编码**；② 检查读入代码用的编码（`defaultCharset` 还是显式指定）；③ 统一成同一编码（推荐全项目 UTF-8）；④ 确认没有用字节流逐字节转 `char`。

**C3**：字节流（`InputStream`/`OutputStream`）按字节搬运，**不关心内容**，适合二进制文件——图片、音频、视频、压缩包、class 文件、网络传输。字符流（`Reader`/`Writer`）按字符搬运、内部带编码表，适合**文本**——读配置文件、CSV/日志、写页面内容，能避免中文乱码。

</details>

## 10. 高频坑

| 坑 | 说明 |
| --- | --- |
| 用 `FileReader` 读 UTF-8 文件 | 用的是系统默认编码，换台机器就乱码；要显式 `InputStreamReader` |
| 用 `FileWriter` 写文件不管编码 | 同上，Windows 会写成 GBK，前端拿到乱码 |
| 字符流读图片/压缩包 | `MalformedInputException` 或数据损坏，二进制一律用字节流 |
| `write(buf)` 不写 `len` | 与字节流同一个坑：尾部写进残留字符 |
| 只 `flush` 忘了 `close` | 资源没释放，文件可能被占用 |
| `\n` 硬编码换行 | 跨平台不稳，用 `System.lineSeparator()` |
| 追加模式传 `false` | 每次都清空，日志文件永远只有一行 |

## 11. 30 秒卡

- **字符流 = 字节流 + 编码表**，一次交给你一个完整字符。
- **UTF-8**：英文 1 字节、中文 3 字节；**GBK**：中文 2 字节。
- **`FileReader`/`FileWriter` 用系统默认编码**，要可控就用 `InputStreamReader`/`OutputStreamWriter` + `Charset`。
- **乱码 = 写读编码不一致**，排查先确认真实编码再统一。
- **二进制用字节流，文本用字符流**。
- **`flush` 刷缓冲不关闭，`close` 先刷再关**。

## 12. 今天要记的 3 个术语（写进术语表）

- 字符流 / `Reader` / `Writer`
- 编码与解码 / charset
- `InputStreamReader` / `OutputStreamWriter`
- `flush` vs `close`

## 13. 面试一句话

> **Java IO 里字节流和字符流的区别？怎么避免乱码？**
> 字节流（`InputStream`/`OutputStream`）以字节为单位，不关心内容编码，适合图片、音视频、压缩包等二进制数据；字符流（`Reader`/`Writer`）以字符为单位，内部维护编码表，把字节按 charset 解码成字符，适合文本文件。`FileReader`/`FileWriter` 用的是系统默认编码，跨机器容易乱码，所以生产代码一般用 `InputStreamReader(new FileInputStream(f), StandardCharsets.UTF_8)` 显式指定编码。乱码的本质是写出和读入的编码不一致（或多字节字符被按字节切割），排查时先确认文件真实编码，再统一全链路为 UTF-8。

## 14. 明天预告

**W4 D5：缓冲流 + 对象流 + `Properties`**——`BufferedReader` 按行读（`readLine`）、`BufferedWriter` 批量写、`ObjectOutputStream` 序列化对象（含 `transient` 与 `serialVersionUID`）、用 `Properties` 读 `config.properties`。明天的 `BufferedReader.readLine()` 是后面读日志、读 CSV 的主力。
