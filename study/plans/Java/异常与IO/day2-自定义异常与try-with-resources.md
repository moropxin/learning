# W4 D2 · 自定义异常 + try-with-resources

> 2026-11-18（三）｜W4 第 2 天｜档位 A 基础｜配套课程：黑马「异常」— 自定义异常、`try-with-resources`、多异常捕获
>
> 今天要拿到三样东西：**① 能自己定义带错误码的业务异常；② 会把底层异常"包装"成业务异常而不丢原因；③ 关流不再写 `finally`，改用 `try-with-resources`。**
>
> 这三样是后面写接口的标配：苍穹外卖里每一层都在抛 `BusinessException`，全局异常处理器 `@RestControllerAdvice` 接的就是它。

> 🧩 **代码块约定**：下面每个 `java` 代码块都是**一份完整可运行的文件**——复制 → 存成 `// 文件：` 后面那个名字 → 运行。多个文件时按出现顺序依次创建，最后一个带 `main` 的运行。

## 0. 今天会遇到的 8 个名词

| 名词 | 一句话人话 | 类比 |
| --- | --- | --- |
| 自定义异常 | 自己写一个异常类，专门表示"我这门业务的错误" | 公司内部专用的报错单 |
| 业务异常 | 表示"用户操作不符合规则"，要给用户看 | "余额不足"，不是"数据库崩了" |
| 错误码 | 用数字/枚举区分错误类型，前端据此提示 | 医院分诊号 |
| 异常包装（wrap） | 把底层异常包一层，换成业务异常，但保留原因 | 快递外层套个箱子，里面东西还在 |
| `cause` | 被包住的那个"原始异常" | 箱子里的原物 |
| `AutoCloseable` | 能自动关闭的资源接口（`close()`） | 用完自动关的水龙头 |
| try-with-resources | `try (资源) {}` 语法，自动关资源 | 智能插座，断电就关 |
| 抑制异常（suppressed） | 关资源时又抛的异常，不覆盖主异常，被挂上去 | 主账单旁附一张小票 |

## 1. 今天 3 小时怎么分

| 时段 | 分钟 | 做什么 | 产出 |
| --- | --- | --- | --- |
| 看课 | 85 | 自定义异常、`try-with-resources`、多异常捕获，1.25 倍速边看边敲 | 跟敲代码 |
| 敲代码 | 55 | 写 `ErrorCode` + `BusinessException` + `ConfigLoader`（读文件失败包装成业务异常） | `error-demo/` 四个类 |
| 算法 | 25 | 力扣 1 题（数组/字符串） | AC 记录 |
| 记录 | 15 | 笔记 3 条 + **术语 3 个**（写进 [术语表](../00-JavaSE总体路线与术语表.md)）+ commit + 日志 | 提交完成 |

## 2. 为什么光用 JDK 的异常不够

昨天那五个异常（`NullPointerException`、`IOException`…）说的是**"计算机出了什么故障"**，但业务要的是**"用户做错了什么"**：

| 场景 | JDK 异常能表达吗 | 想要的信息 |
| --- | --- | --- |
| 用户余额 5 元买 100 元的东西 | `IllegalArgumentException`? 太泛 | `BALANCE_NOT_ENOUGH`：余额不足，还差 95 元 |
| 优惠券已过期 | `IllegalStateException`? 太泛 | `VOUCHER_EXPIRED`：券已过期 |
| 手机号已注册 | 没有合适的内置异常 | `PHONE_EXISTS`：该手机号已注册 |

**自定义异常 = 给"业务规则被违反"起一个专门的名字**，让上层能精确 catch、让前端能精确提示。

## 3. 自定义异常怎么写（先把骨架敲出来）

```java
// 文件：BusinessException.java
public class BusinessException extends RuntimeException {

    // 业务错误码，前端/日志用它区分错误类型
    private final int code;

    // 只给消息（最常用）
    public BusinessException(String message) {
        super(message);
        this.code = 500;
    }

    // 消息 + 错误码
    public BusinessException(int code, String message) {
        super(message);
        this.code = code;
    }

    // 消息 + 原因（包装底层异常时用，见 §5）
    public BusinessException(String message, Throwable cause) {
        super(message, cause);
        this.code = 500;
    }

    // 全套
    public BusinessException(int code, String message, Throwable cause) {
        super(message, cause);
        this.code = code;
    }

    public int getCode() {
        return code;
    }
}
```

**四个构造器为什么都要写？** 因为父类 `RuntimeException` 有这四个，你不写就只能用父类的签名，调用方没法一次性传齐"码 + 消息 + 原因"。IDE 里 `Alt+Insert → Constructor` 可以一键生成。

**为什么继承 `RuntimeException` 而不是 `Exception`？**

| 继承 | 效果 | 适用 |
| --- | --- | --- |
| `RuntimeException` | 调用方**不必**强制 catch/throws，代码干净 | 业务异常（参数错、余额不足）——交给全局异常处理器统一处理 |
| `Exception` | 调用方**必须**处理，编译器强制 | 外部资源类（自定义的文件/网络组件），逼调用方考虑失败 |

> 记住一句话：**业务异常继承 `RuntimeException`，让上层"想接就接"；工具类异常继承 `Exception`，逼上层"必须接"。**

## 4. 加一层错误码枚举（让码有名字）

```java
// 文件：ErrorCode.java
public enum ErrorCode {

    SUCCESS(0, "成功"),
    BALANCE_NOT_ENOUGH(1001, "余额不足"),
    VOUCHER_EXPIRED(1002, "优惠券已过期"),
    PHONE_EXISTS(1003, "手机号已注册"),
    CONFIG_READ_FAIL(2001, "配置读取失败");

    private final int code;
    private final String message;

    ErrorCode(int code, String message) {
        this.code = code;
        this.message = message;
    }

    public int getCode() {
        return code;
    }

    public String getMessage() {
        return message;
    }
}
```

```java
// 文件：OrderService.java
public class OrderService {

    private int balance = 5;

    // 扣款：余额不够就抛业务异常，调用方一眼看懂
    public void pay(int amount) {
        if (amount > balance) {
            throw new BusinessException(
                    ErrorCode.BALANCE_NOT_ENOUGH.getCode(),
                    ErrorCode.BALANCE_NOT_ENOUGH.getMessage() + "，还差 " + (amount - balance) + " 元"
            );
        }
        balance -= amount;
        System.out.println("支付成功，余额 " + balance);
    }

    public static void main(String[] args) {
        OrderService service = new OrderService();
        try {
            service.pay(100);
        } catch (BusinessException e) {
            System.out.println("错误码 " + e.getCode() + "：" + e.getMessage());
        }
    }
}
// 输出：
// 错误码 1001：余额不足，还差 95 元
```

## 5. 异常包装：底层异常换成业务异常，但别丢掉原因

真实代码里最常见的一段：

```java
// 伪代码（反面示例）：直接抛出底层异常，上层不知道业务含义
try {
    String cfg = Files.readString(Path.of("app.properties"));
} catch (IOException e) {
    throw e;   // 上层看到的是 IOException，不知道"这是配置读取失败"
}
```

正确做法是**包装**——换一个业务视角的说法，同时把原始异常塞进 `cause`：

```java
// 文件：ConfigLoader.java
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

public class ConfigLoader {

    /**
     * 读取配置文件。
     * 失败时抛业务异常，原始 IOException 保留在 cause 里，排查时照样能看到根因。
     */
    public static String load(String path) {
        try {
            return Files.readString(Path.of(path));
        } catch (IOException e) {
            // 关键：第二个参数 e 就是 cause，不能丢
            throw new BusinessException(
                    ErrorCode.CONFIG_READ_FAIL.getCode(),
                    ErrorCode.CONFIG_READ_FAIL.getMessage() + "：" + path,
                    e
            );
        }
    }

    public static void main(String[] args) {
        try {
            String content = load("app.properties");
            System.out.println("配置内容：" + content);
        } catch (BusinessException e) {
            System.out.println("业务错误码：" + e.getCode());
            System.out.println("业务消息：" + e.getMessage());
            System.out.println("原始原因：" + e.getCause().getClass().getSimpleName());
        }
    }
}
// 输出（app.properties 不存在时）：
// 业务错误码：2001
// 业务消息：配置读取失败：app.properties
// 原始原因：NoSuchFileException
```

**为什么要包装？**

1. **上层只认业务语义**：Controller 层只 care "CONFIG_READ_FAIL"，不需要知道 `NoSuchFileException` 还是 `AccessDeniedException`。
2. **不丢根因**：`cause` 让日志里能打出完整链路（`Caused by: java.nio.file.NoSuchFileException: app.properties`）。
3. **能统一处理**：全局异常处理器 catch 一个 `BusinessException` 就能返回统一 JSON。

> ⚠️ 反例：`catch (IOException e) { throw new BusinessException("失败"); }`——不传 cause，日志里根因**彻底消失**，线上出事只能猜。

## 6. 多异常捕获：一次接住几种

```java
// 文件：MultiCatchDemo.java
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

public class MultiCatchDemo {

    public static void main(String[] args) {

        // 写法 1：| 并列（JDK7+），几种异常同样处理时用它
        try {
            int n = Integer.parseInt("abc");              // NumberFormatException
            String cfg = Files.readString(Path.of("x.txt"));  // 若格式对会到这里抛 NoSuchFileException
            System.out.println(n + cfg);
        } catch (NumberFormatException | IOException e) {
            System.out.println("参数或文件有问题：" + e.getClass().getSimpleName());
        }

        // 写法 2：分开捕获，分别处理（顺序：子类在前，父类在后）
        try {
            Object o = "hello";
            Integer i = (Integer) o;                      // ClassCastException
            System.out.println(i);
        } catch (ClassCastException e) {
            System.out.println("类型转换失败：" + e.getMessage());
        } catch (RuntimeException e) {                    // 兜底放最后
            System.out.println("其他运行时异常");
        }
    }
}
// 输出：
// 参数或文件有问题：NumberFormatException
// 类型转换失败：class java.lang.String cannot be cast to class java.lang.Integer
```

**⚠️ 这里有个必踩的坑**：如果 try 里抛出的异常**不属于**你 catch 的那几种，`catch` 是接不住的，程序直接终止：

```java
// 文件：UncaughtDemo.java
public class UncaughtDemo {
    public static void main(String[] args) {
        try {
            int n = Integer.parseInt(args[0]);   // 不传参时 args 长度为 0 → 越界
            System.out.println(n);
        } catch (NumberFormatException e) {      // 只接了数字格式，接不住越界
            System.out.println("接住了");
        }
    }
}
// 不带参数运行的真实输出（程序终止，catch 根本没执行）：
// Exception in thread "main" java.lang.ArrayIndexOutOfBoundsException: Index 0 out of bounds for length 0
//     at UncaughtDemo.main(UncaughtDemo.java:5)
```

> 所以"多异常捕获"不是把所有异常都接住，**catch 只接你列出来的类型**。要兜底就显式写 `catch (Exception e)`，或者用 `finally` 做善后。

## 6.1 完整兜底写法（推荐模板）

```java
// 文件：RobustDemo.java
public class RobustDemo {

    public static void main(String[] args) {
        // ① 先做参数校验，把"可预见的错误"挡在外面
        if (args.length == 0) {
            System.out.println("用法：java RobustDemo <数字>");
            return;
        }
        try {
            int n = Integer.parseInt(args[0]);
            System.out.println("解析结果：" + n);
        } catch (NumberFormatException e) {          // ② 可预见的异常，精确捕获
            System.out.println("不是合法数字：" + args[0]);
        } catch (Exception e) {                       // ③ 兜底：不认识的一律记录，不让程序崩
            System.out.println("未知错误：" + e.getClass().getSimpleName());
        } finally {
            System.out.println("结束（无论成败都会执行）");
        }
    }
}
// 不带参数运行：用法：java RobustDemo <数字>
// 传 abc 运行：不是合法数字：abc → 结束（无论成败都会执行）
// 传 42 运行：解析结果：42 → 结束（无论成败都会执行）
```

## 7. try-with-resources：不再手写 finally 关流

### 7.1 老写法有多烦

```java
// 文件：OldStyleRead.java
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

public class OldStyleRead {

    public static void main(String[] args) {
        BufferedReader reader = null;            // 必须提到外面，finally 才能用
        try {
            reader = new BufferedReader(new FileReader("a.txt"));
            System.out.println(reader.readLine());
        } catch (IOException e) {
            System.out.println("读失败：" + e.getMessage());
        } finally {
            if (reader != null) {                // 不判空就 NPE
                try {
                    reader.close();              // close 自己也会抛异常！
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
```

三个痛点：**变量要提到外面、必须判空、close 还要再套一个 try**。开两个流就是两层嵌套。

### 7.2 新写法

```java
// 文件：TryWithResourcesDemo.java
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

public class TryWithResourcesDemo {

    public static void main(String[] args) {
        // try (...) 里声明的资源，出作用域自动 close()，不用写 finally
        try (BufferedReader reader = new BufferedReader(new FileReader("a.txt"))) {
            System.out.println(reader.readLine());
        } catch (IOException e) {
            System.out.println("读失败：" + e.getClass().getSimpleName());
        }
        System.out.println("资源已自动关闭");
    }
}
// 输出（a.txt 不存在时）：
// 读失败：FileNotFoundException
// 资源已自动关闭
```

**能放进 `try (...)` 的条件**：实现了 `AutoCloseable`（或 `Closeable`）接口。`InputStream` / `OutputStream` / `Reader` / `Writer` / `Connection` / `Statement` / `ResultSet` 全都是。

### 7.3 多个资源：逆序关闭

```java
// 文件：TwoResourcesDemo.java
class Res implements AutoCloseable {
    private final String name;
    Res(String name) { this.name = name; System.out.println("打开 " + name); }
    @Override public void close() { System.out.println("关闭 " + name); }
}

public class TwoResourcesDemo {
    public static void main(String[] args) {
        // 用 ; 分隔多个资源，关闭顺序与声明顺序相反
        try (Res a = new Res("A"); Res b = new Res("B")) {
            System.out.println("使用中");
        }
    }
}
// 输出：
// 打开 A
// 打开 B
// 使用中
// 关闭 B
// 关闭 A
```

**为什么逆序？** 因为 `B` 可能依赖 `A`（先开的后关，符合"后进先出"）。

### 7.4 关闭时又抛异常怎么办（抑制异常）

```java
// 文件：SuppressedDemo.java
class BadRes implements AutoCloseable {
    @Override public void close() {
        throw new IllegalStateException("关闭时炸了");
    }
}

public class SuppressedDemo {
    public static void main(String[] args) {
        try (BadRes r = new BadRes()) {
            throw new RuntimeException("业务炸了");     // 主异常
        } catch (RuntimeException e) {
            System.out.println("主异常：" + e.getMessage());
            for (Throwable t : e.getSuppressed()) {     // 关闭时的异常被"抑制"挂在这里
                System.out.println("被抑制：" + t.getMessage());
            }
        }
    }
}
// 输出：
// 主异常：业务炸了
// 被抑制：关闭时炸了
```

> 这就是 try-with-resources 比手写 `finally` 强的地方：**主异常不会被 `close()` 的异常覆盖**，两者都能看到。手写 finally 时，close 抛的异常会把业务异常吞掉。

## 8. 动手任务（55 分钟）

**任务 A：四个类跑起来（20 分钟）**
按 §3 → §4 → §5 顺序创建 `BusinessException.java`、`ErrorCode.java`、`OrderService.java`、`ConfigLoader.java`，四个都能跑出注释里的输出。

**任务 B：加一个业务异常（10 分钟）**
在 `ErrorCode` 里加 `PHONE_EXISTS(1003, "手机号已注册")`，写一个 `register(String phone)`：已存在（用 `Set<String>` 模拟）就抛 `BusinessException`，main 里 catch 打印"错误码 + 消息"。

**任务 C：把昨天的读文件例子改成 try-with-resources（15 分钟）**
写 `ReadConfig.java`：用 `try (BufferedReader ...)` 读 `app.properties` 的前三行，任何 `IOException` 都包装成 `BusinessException`（带 cause），并打印 `e.getCause()`。

**任务 D：亲眼见一次"没 catch 住"（10 分钟）**
把 §6 的 `UncaughtDemo.java` 整份跑起来（**不带参数运行**），看清楚：栈轨迹里是谁抛的、`catch` 为什么没执行。然后改成 §6.1 的 `RobustDemo.java` 两种保护写法（先判 `args.length` + `catch (Exception e)` 兜底），对比输出。

**验收标准**

- [T] 能说出自定义异常为什么常继承 `RuntimeException`
- [T] 能说出包装异常时必须传 `cause`，不传会丢什么
- [T] 能说出 `try (...)` 里能放什么类型的对象（`AutoCloseable`）
- [T] 能说出多个资源的关闭顺序，以及"抑制异常"是什么

## 9. 练习（先写，再看答案）

### A. 判断（5 题）

**A1.** 自定义异常必须继承 `Exception`，不能继承 `RuntimeException`。

> 答：错，继承RuntimeException时异常可以给前端、日志返回异常，可自行决定扔不扔

**A2.** 抛出 `RuntimeException` 的子类时，调用方不写 `try-catch` 也能编译通过。

> 答：对，为运行异常，编译期可通过

**A3.** 把 `IOException` 包装成 `BusinessException` 时，如果不传 `cause`，原始异常信息就丢了。

> 答：对，会上升为父类异常

**A4.** `try (A a = new A(); B b = new B())` 的关闭顺序是先关闭 `A` 再关闭 `B`。

> 答：错，逆序关闭，先开A后B，再闭B后A

**A5.** 只有实现了 `AutoCloseable` 接口的对象才能放进 `try (...)`。

> 答：对，Closeable是其子接口

### B. 选择（5 题）

**B1.** 下面哪个最适合做业务异常：
A. 继承 `Error`　B. 继承 `RuntimeException`　C. 继承 `Throwable`　D. 直接用 `Exception`

> 答：B

**B2.** `throw new BusinessException("余额不足", e)` 中 `e` 的作用是：
A. 覆盖原异常　B. 作为 cause 保留根因　C. 打印日志　D. 无关参数

> 答：B

**B3.** `try (Res a = new Res()) { ... }` 结束时自动调用：
A. `a.finalize()`　B. `a.close()`　C. `a.flush()`　D. 什么都不调

> 答：B

**B4.** 多个资源正确关闭顺序是：
A. 与声明顺序相同　B. 与声明顺序相反　C. 随机　D. 由 JVM 决定

> 答：B

**B5.** 主异常抛出后 `close()` 也抛异常，结果是：
A. `close()` 的异常覆盖主异常　B. 主异常保留，`close()` 异常进入 `getSuppressed()`　C. 两个都丢失　D. 编译报错

> 答：A

### C. 简答（3 题）

**C1.** 为什么业务异常通常继承 `RuntimeException` 而不是 `Exception`？

> 答：业务异常时通常为用户操作不当导致的异常，这类异常出现的很频繁，当用Exception时从Service到Colltroller层都要throws自定义异常，代码非常不清晰，而出现运行异常时往往只有全局异常处理器才能处理。所以用RuntimeException后调用方随时可接，统一由`@RestControllerAdvice` 转成 JSON，职责最清晰

**C2.** 异常包装时为什么必须传 `cause`？不传会怎样？

> 答：cause里保存的是异常对象、留下异常根，方便日志追踪和异常排查。不传的话底层异常会丢失，当程序出现异常时会无法追踪排查到根本异常原因

**C3.** `try-with-resources` 相比手写 `finally` 关资源，好在哪三点？

> 答：N1.不用写finally结构，不用判空
>     N2.可多开流，各个流自动按次序开关
>     N3.不会吞主异常，close()抛的异常会作为抑制异常挂在主异常getSuppressed()上，两者留痕。手写finall时会覆盖业务异常，只有close()异常日志

<details>
<summary>参考答案</summary>

**A 判断**：A1 **错**（可以继承 `RuntimeException`，且业务异常推荐如此）｜A2 **对**（编译器不强制处理运行时异常）｜A3 **对**｜A4 **错**（逆序：先关 `B` 再关 `A`）｜A5 **对**（`Closeable` 是其子接口）

**B 选择**：B1 B｜B2 B｜B3 B｜B4 B｜B5 B

**C1**：业务异常表示"用户操作违反业务规则"（余额不足、券过期、手机号已注册），这类错误**几乎每层都可能出现**。如果继承 `Exception`（编译时异常），那么从 Service 到 Controller 每一层方法签名都得 `throws BusinessException`，代码被污染，而大多数层其实处理不了、只想让它冒泡到全局异常处理器。继承 `RuntimeException` 后调用方"想接就接"，最终由 `@RestControllerAdvice` 统一转成 JSON，职责最清晰。

**C2**：`cause` 保存的是**原始异常对象**，异常打印时它会以 `Caused by: ...` 的形式出现在栈轨迹里，保留了真正的根因（比如 `NoSuchFileException: app.properties`）。不传 cause 就等于把底层异常**彻底扔掉**，日志里只剩一句"配置读取失败"，线上排查时不知道是文件不存在、权限不够还是路径写错。

**C3**：① **不用写 finally、不用判空**，资源在 `try (...)` 里声明即可自动关闭；② **多个资源自动逆序关闭**，开两个流不用嵌套两层；③ **不会吞掉主异常**——`close()` 抛的异常会作为"抑制异常"挂在主异常的 `getSuppressed()` 上，两者都留痕；手写 `finally` 时 `close()` 的异常会覆盖业务异常。

</details>

## 10. 高频坑

| 坑 | 说明 |
| --- | --- |
| 包装异常不传 `cause` | 根因丢失，日志只剩一句业务话术，线上无法排查 |
| 自定义异常里没写 `serialVersionUID` | IDE 会警告；线上序列化（RPC）时最好加上 `private static final long serialVersionUID = 1L;` |
| `catch (Exception e) { throw new BusinessException("失败"); }` | 把参数错误、空指针也包成"业务错误"，掩盖真 bug |
| `try (...)` 里放了没实现 `AutoCloseable` 的对象 | 编译报错："不兼容的类型" |
| 在 `finally` 里 `close()` 且没判空 | 空指针，把真正的异常盖掉 |
| 业务异常继承 `Exception` 后满屏 `throws` | 每层签名都要加，代码噪音大；业务异常应用 `RuntimeException` |
| 一个 `catch` 里 `|` 并列了有继承关系的两个类 | 编译报错（子类已被父类覆盖） |

## 11. 30 秒卡

- **自定义异常 = 给业务错误起名字**，通常 `extends RuntimeException`。
- **四件套**：错误码枚举 + 异常类（带 code）+ 抛的地方 + 全局处理器接（后面学 Spring 时补）。
- **包装必传 cause**：`throw new BusinessException(code, msg, e)`，否则根因消失。
- **`try (资源)` 自动关**，条件是实现 `AutoCloseable`；多资源**逆序关**。
- **关闭异常不会覆盖主异常**，进 `getSuppressed()`。

## 12. 今天要记的 3 个术语（写进术语表）

- 业务异常 / 自定义异常
- 异常包装（wrap）/ `cause`
- `AutoCloseable` / try-with-resources
- 抑制异常（suppressed）

## 13. 面试一句话

> **你们的项目里异常是怎么处理的？**
> 分三层：底层（DAO/工具类）把受检异常和运行时异常**包装**成自定义的 `BusinessException`，包装时一定带上 `cause` 保留根因；业务层用错误码枚举（如 `ErrorCode.BALANCE_NOT_ENOUGH`）表达具体规则，异常类里带 `code` 和 `message`；最外层用全局异常处理器（`@RestControllerAdvice`）统一 catch，把 `BusinessException` 转成统一 JSON，其他未知异常记日志后返回 500，避免堆栈泄露给前端。另外资源关闭统一用 `try-with-resources`，避免 finally 里 close 抛异常把主异常吞掉。

## 14. 明天预告

**W4 D3：`File` + 字节流**——用 `FileInputStream`/`FileOutputStream` 复制一张图片，遍历目录打印所有 `.java` 文件。今天的 `try-with-resources` 明天会天天用。
