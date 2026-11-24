# W5 D3 · 等待唤醒与 Lock

> 2026-11-25（三）｜W5 第 3 天｜预计 3 小时

> 昨天学的是"互斥"——一次只让一个线程进。
> 今天学"协作"——一个线程放东西，另一个线程取东西，放满了要等、取空了也要等。
> 这类问题用 `synchronized` 硬扛会写出"忙等"（空转烧 CPU），必须用 `wait/notify`。

## 0. 今天会遇到的 8 个名词

| 名词 | 一句话 | 类比 |
| --- | --- | --- |
| `wait()` | 让当前线程**释放锁**并等待 | 服务员把窗口让出来去休息室等 |
| `notify()` | 随机唤醒一个在这把锁上等的线程 | 广播"来一位客人" |
| `notifyAll()` | 唤醒所有等待线程 | 全体集合 |
| 虚假唤醒 | 没被 `notify` 却自己醒了 | 睡觉时被自己翻身弄醒 |
| 生产者-消费者 | 一个放、一个取，用缓冲区解耦 | 传送带 |
| `Lock` / `ReentrantLock` | 手动加锁解锁的锁 | 手动门锁，要自己开自己锁 |
| `Condition` | `Lock` 版的 `wait/notify` | 分成"缺货区"和"满仓区"两个休息室 |
| `await()` / `signal()` | `Condition` 里对应的等待/唤醒 | 同上 |

## 1. 今天 3 小时怎么分

| 分钟 | 做什么 |
| --- | --- |
| 85 | 看课 + 读 §2–§6（重点是"为什么必须 while"和"为什么 wait 要释放锁"） |
| 55 | 敲生产者-消费者两版（`wait/notify` 一版、`Lock/Condition` 一版） |
| 25 | 算法 1 题 |
| 15 | 术语 + 练习 + commit |

## 2. 为什么 `synchronized` 不够用

想象一个缓冲区，最多放 5 个：

- 生产者想放，但**满了**，只能等消费者取走
- 消费者想取，但**空了**，只能等生产者放入

如果只用 `synchronized`，你能想到的写法是"拿到锁 → 发现满了 → 释放锁 → 再抢锁 → 再看满没满"：

```java
// 忙等（错误示范）：空转烧 CPU
while (true) {
    synchronized (LOCK) {
        if (size < 5) { 放; break; }
    }
    // 没拿到就继续抢 —— CPU 被这一行烧光
}
```

这种写法叫**忙等（busy waiting）**。线程在疯狂抢锁、疯狂判断，CPU 占用拉满，但什么活都没干。

`wait/notify` 解决的就是这个：**主动把锁交出来，进入休眠，等别人叫醒**。

## 3. `wait/notify` 的三条铁律

```mermaid
flowchart LR
    A["synchronized 内"] --> B["调用 wait"]
    B --> C["释放锁<br/>进入 WAITING"]
    C --> D["被 notify 唤醒"]
    D --> E["重新抢锁"]
    E --> F["从 wait 之后继续执行"]
```

**铁律一：只能在 `synchronized` 块里调用。**
`wait()`/`notify()` 属于 `Object`，底层依赖监视器锁。不在同步块里调用直接抛 `IllegalMonitorStateException`。

**铁律二：`wait()` 会释放锁，`sleep()` 不会。**
这就是昨天那道判断题的延伸。`wait()` 释放锁是为了让别的线程能进来改变条件（比如消费者进来取走东西），否则大家互相等死。

**铁律三：被唤醒后要**重新抢锁**，不是立刻执行。**
`notify()` 之后，等待线程从 `WAITING` 变成 `BLOCKED`，要去抢锁；抢到了才从 `wait()` 那一行之后继续执行。

```java
// 文件：WaitNotifyBasicDemo.java
public class WaitNotifyBasicDemo {

    static final Object LOCK = new Object();
    static boolean ready = false;

    public static void main(String[] args) throws InterruptedException {
        Thread waiter = new Thread(() -> {
            synchronized (LOCK) {
                while (!ready) {                 // 必须用 while，见 §4
                    try {
                        System.out.println("等待线程：条件不满足，交出锁去睡");
                        LOCK.wait();             // 释放锁 + 进入 WAITING
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                        return;
                    }
                }
                System.out.println("等待线程：被唤醒且条件满足，开始干活");
            }
        }, "等待线程");

        waiter.start();
        Thread.sleep(300);                       // 让等待线程先跑起来

        synchronized (LOCK) {
            ready = true;
            LOCK.notifyAll();                    // 唤醒在 LOCK 上等的线程
            System.out.println("主线程：已通知");
        }
    }
}
// 输出：
// 等待线程：条件不满足，交出锁去睡
// 主线程：已通知
// 等待线程：被唤醒且条件满足，开始干活
```

## 4. 虚假唤醒：为什么必须用 `while` 而不是 `if`

`wait()` 可能在**没有 `notify`** 的情况下返回（JVM 允许，叫**虚假唤醒** spurious wakeup）。如果写成 `if`，醒来一次就直接往下执行了，条件可能还是不满足。

```java
// 文件：FalseWakeupDemo.java
public class FalseWakeupDemo {

    static final Object LOCK = new Object();
    static int count = 0;

    public static void main(String[] args) throws InterruptedException {
        Thread consumer = new Thread(() -> {
            synchronized (LOCK) {
                // ❌ 错误写法：if 只判断一次
                // if (count == 0) { LOCK.wait(); }
                // System.out.println("消费：" + count--);   // 可能消费到 -1

                // ✅ 正确写法：while 循环判断，醒来后重新检查
                while (count == 0) {
                    try {
                        LOCK.wait();
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                        return;
                    }
                }
                System.out.println("消费：" + count);
                count--;
            }
        }, "消费者");

        consumer.start();
        Thread.sleep(100);

        synchronized (LOCK) {
            count = 5;
            LOCK.notifyAll();
        }
        consumer.join();
    }
}
// 输出：
// 消费：5
```

**一句话记住**：`wait()` 必须包在 `while (条件不满足)` 里，**永远不要用 `if`**。

## 5. 生产者-消费者（`wait/notify` 版）

```java
// 文件：ProducerConsumerWaitNotify.java
import java.util.LinkedList;
import java.util.Queue;

public class ProducerConsumerWaitNotify {

    static final int MAX = 5;                          // 缓冲区容量
    static final Queue<Integer> BUFFER = new LinkedList<>();
    static final Object LOCK = new Object();

    static class Producer implements Runnable {
        @Override
        public void run() {
            int id = 1;
            while (true) {
                synchronized (LOCK) {
                    while (BUFFER.size() == MAX) {      // 满了：等消费者取
                        try {
                            LOCK.wait();
                        } catch (InterruptedException e) {
                            Thread.currentThread().interrupt();
                            return;
                        }
                    }
                    BUFFER.offer(id);
                    System.out.println("生产 " + id + "，当前 " + BUFFER.size() + " 个");
                    LOCK.notifyAll();                   // 叫醒可能等在"空"上的消费者
                    id++;
                }
                sleep(300);
            }
        }
    }

    static class Consumer implements Runnable {
        @Override
        public void run() {
            while (true) {
                synchronized (LOCK) {
                    while (BUFFER.isEmpty()) {          // 空了：等生产者放
                        try {
                            LOCK.wait();
                        } catch (InterruptedException e) {
                            Thread.currentThread().interrupt();
                            return;
                        }
                    }
                    Integer item = BUFFER.poll();
                    System.out.println("消费 " + item + "，当前 " + BUFFER.size() + " 个");
                    LOCK.notifyAll();                   // 叫醒可能等在"满"上的生产者
                }
                sleep(700);
            }
        }
    }

    public static void main(String[] args) {
        new Thread(new Producer(), "生产者").start();
        new Thread(new Consumer(), "消费者").start();
    }

    static void sleep(long ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }
}
// 输出（交替出现，缓冲区不会超过 5、不会小于 0）：
// 生产 1，当前 1 个
// 生产 2，当前 2 个
// 消费 1，当前 1 个
// 生产 3，当前 2 个
// ...
```

**三个必须解释清楚的点：**

1. **为什么两个 `wait()` 都要用 `while`**——虚假唤醒 + 被叫醒后条件可能又被别人改了。
2. **为什么用 `notifyAll()` 而不是 `notify()`**——`notify()` 随机唤醒一个，可能唤醒的是"同一类"线程（比如唤醒另一个生产者，但生产者还是满的，于是它又睡回去，真正的消费者没被叫醒）→ **唤醒丢失**。`notifyAll()` 唤醒所有，各自重新判断，代价是"惊群"（都醒来抢锁，但只有一个能进）。
3. **为什么 `sleep` 放在 `synchronized` 外面**——在锁里 sleep 会让锁一直不释放，白白拖慢别人。

## 6. 用 `Lock` + `Condition` 重写

`ReentrantLock` 是 `synchronized` 的手动版，`Condition` 是它的 `wait/notify`。它比 `synchronized` 多两个能力：**可以分成多个等待队列**、**可以 `tryLock` 超时**。

```java
// 文件：ProducerConsumerLock.java
import java.util.LinkedList;
import java.util.Queue;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class ProducerConsumerLock {

    static final int MAX = 5;
    static final Queue<Integer> BUFFER = new LinkedList<>();

    static final Lock LOCK = new ReentrantLock();
    static final Condition NOT_FULL = LOCK.newCondition();   // 生产者在这里等
    static final Condition NOT_EMPTY = LOCK.newCondition();  // 消费者在这里等

    static class Producer implements Runnable {
        @Override
        public void run() {
            int id = 1;
            while (true) {
                LOCK.lock();                       // 手动加锁
                try {
                    while (BUFFER.size() == MAX) {
                        NOT_FULL.await();          // 只等"不满"，不等"不空"
                    }
                    BUFFER.offer(id);
                    System.out.println("生产 " + id + "，当前 " + BUFFER.size() + " 个");
                    NOT_EMPTY.signalAll();         // 只叫醒消费者
                    id++;
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    return;
                } finally {
                    LOCK.unlock();                 // 必须在 finally 里解锁
                }
                sleep(300);
            }
        }
    }

    static class Consumer implements Runnable {
        @Override
        public void run() {
            while (true) {
                LOCK.lock();
                try {
                    while (BUFFER.isEmpty()) {
                        NOT_EMPTY.await();         // 只等"不空"
                    }
                    Integer item = BUFFER.poll();
                    System.out.println("消费 " + item + "，当前 " + BUFFER.size() + " 个");
                    NOT_FULL.signalAll();          // 只叫醒生产者
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    return;
                } finally {
                    LOCK.unlock();
                }
                sleep(700);
            }
        }
    }

    public static void main(String[] args) {
        new Thread(new Producer(), "生产者").start();
        new Thread(new Consumer(), "消费者").start();
    }

    static void sleep(long ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }
}
// 输出：与 wait/notify 版一致
```

**`Condition` 的核心价值**：用 `wait/notify` 时只有**一个**等待队列，生产者叫醒时可能叫到另一个生产者（无效唤醒）；用两个 `Condition` 就能**精确唤醒对方那一边**。

### `tryLock`：`synchronized` 做不到的事

```java
// 文件：TryLockDemo.java
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.ReentrantLock;

public class TryLockDemo {

    static final ReentrantLock LOCK = new ReentrantLock();

    public static void main(String[] args) throws InterruptedException {
        LOCK.lock();                       // 主线程先占住锁
        try {
            Thread t = new Thread(() -> {
                try {
                    // 最多等 1 秒，拿不到就返回 false —— 不会傻等
                    boolean got = LOCK.tryLock(1, TimeUnit.SECONDS);
                    System.out.println("子线程是否拿到锁：" + got);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            });
            t.start();
            t.join();
        } finally {
            LOCK.unlock();
        }
    }
}
// 输出：
// 子线程是否拿到锁：false
```

## 7. `synchronized` vs `Lock`

| 维度 | `synchronized` | `ReentrantLock` |
| --- | --- | --- |
| 加锁/解锁 | 自动（JVM 管） | 手动 `lock()`/`unlock()`，必须放 `finally` |
| 能否超时放弃 | 不能，死等 | 能，`tryLock(timeout)` |
| 能否中断等待 | 不能 | 能，`lockInterruptibly()` |
| 多个等待队列 | 只有一个 | 多个 `Condition` |
| 公平锁 | 非公平 | 可 `new ReentrantLock(true)` |
| 出异常会不会漏解锁 | 不会 | **会**（忘了 `finally` 就永久锁死） |

**选择原则**：能用 `synchronized` 就用 `synchronized`（简单、不会漏解锁）；需要超时、中断、多条件队列时才用 `Lock`。

## 8. 动手任务 A–D（55 分钟）

### A. 先看"睡"和"等"的区别

跑 `WaitNotifyBasicDemo`，把 `LOCK.wait()` **换成** `Thread.sleep(2000)`，再跑一次。观察并回答：

| 问题 | 现象 |
| --- | --- |
| 换成 sleep 后，主线程能不能进 `synchronized (LOCK)` 块？ | |
| 如果能/不能，为什么？ | |

> 这一步就是"`wait` 释放锁、`sleep` 不释放"的物理证据。你昨天那道判断题，今天亲眼验证。

### B. 把 `while` 改回 `if`，看会发生什么

跑 `FalseWakeupDemo`，把 `while (count == 0)` 改成 `if (count == 0)`。多跑几次，记录是否出现"消费 0"或"消费 -1"。

> 虚假唤醒不一定每次都能复现（JVM 实现不同），但**这个坑必须按会发生来写代码**。

### C. 生产者-消费者：先跑通，再制造越界

跑 `ProducerConsumerWaitNotify`，观察 30 秒，确认缓冲区始终在 0–5 之间。

然后把**生产者的 `while` 改成 `if`**，再跑，看是否能跑出 `当前 6 个` 或 `当前 -1 个`。

### D. 对比两种实现

跑 `ProducerConsumerLock`，观察它和 `wait/notify` 版的行为差异（尤其在被唤醒的"精确性"上）。然后在纸上回答：如果把 `NOT_EMPTY.signalAll()` 改成 `LOCK.newCondition().signalAll()`（新建一个 Condition 再唤醒），还能正常工作吗？为什么？

## 9. 练习（先写，再看答案）

### A. 判断（5 题）

**A1.** `wait()` 和 `sleep()` 都会让线程暂停，两者都会释放锁。

> 答：

**A2.** `wait()` 必须在 `synchronized` 块里调用，否则抛 `IllegalMonitorStateException`。

> 答：

**A3.** `notify()` 被调用后，等待的线程会立刻继续执行 `wait()` 之后的代码。

> 答：

**A4.** `wait()` 应该包在 `while` 循环里，而不是 `if` 里。

> 答：

**A5.** `ReentrantLock` 的 `unlock()` 必须放在 `finally` 里。

> 答：

### B. 选择（5 题）

**B1.** 线程调用 `obj.wait()` 时，被释放的是：
A. 所有锁　B. `obj` 这个对象上的监视器锁　C. CPU 时间片　D. 不需要释放

> 答：

**B2.** 生产者-消费者里用 `notify()` 而不用 `notifyAll()`，可能造成的问题是：
A. 死锁必然发生　B. 唤醒丢失（叫醒了不需要的线程）　C. 抛异常　D. 缓冲区溢出

> 答：

**B3.** 下面哪个方法**不能**被中断（不会抛 `InterruptedException`）？
A. `Thread.sleep()`　B. `Object.wait()`　C. `Condition.await()`　D. `synchronized` 获取锁

> 答：

**B4.** `Condition` 相比 `wait/notify` 的最大优势是：
A. 性能更高　B. 可以建多个等待队列精确唤醒　C. 不需要加锁　D. 自动释放锁

> 答：

**B5.** `ReentrantLock(true)` 中的 `true` 表示：
A. 可重入　B. 公平锁（按等待顺序获取）　C. 可中断　D. 超时

> 答：

### C. 简答（3 题）

**C1.** 为什么 `wait()` 要释放锁，而 `sleep()` 不释放？从"如果不释放会怎样"的角度回答。

> 答：

**C2.** 生产者-消费者代码里，生产者 `notifyAll()` 之后，消费者被唤醒。请描述消费者从"被唤醒"到"真正执行 `poll()`"之间经历了什么。

> 答：

**C3.** `synchronized` 和 `ReentrantLock` 各自适合什么场景？各说一个 `synchronized` 做不到、只有 `Lock` 能做的事。

> 答：

<details>
<summary>参考答案</summary>

**A 判断**：A1 **错**（`wait` 释放锁，`sleep` 不释放）｜A2 **对**｜A3 **错**（先变 `BLOCKED` 抢锁，抢到才继续；`notify` 只是"叫醒"，不是"执行"）｜A4 **对**（防虚假唤醒、防条件被改）｜A5 **对**（否则一旦抛异常，锁永不释放）

**B 选择**：B1 **B**（只释放该对象上的监视器）｜B2 **B**｜B3 **D**（`synchronized` 等锁不可中断；`ReentrantLock.lockInterruptibly()` 才可以）｜B4 **B**｜B5 **B**

**C1**：`wait()` 的目的是"等某个条件成立"，而条件是由别的线程改变的——不释放锁，别的线程进不了同步块，条件永远无法改变，双方互相等死。`sleep()` 只是"暂停一段时间"，不依赖别人改条件，所以不需要释放锁，而且保持锁还能防止它睡觉期间数据被别人改。

**C2**：①从 `WAITING` 变为 `BLOCKED`，进入 `LOCK` 的等待队列抢锁；②抢到锁后，从 `wait()` 调用处之后继续执行；③回到 `while (BUFFER.isEmpty())` 重新判断（这就是为什么必须用 `while`：被唤醒时缓冲区可能又被别的消费者取空了）；④条件满足才执行 `poll()`，否则再次 `await()`。

**C3**：`synchronized` 适合绝大多数简单场景（自动释放、不会漏解锁）；`ReentrantLock` 适合需要**超时放弃**（`tryLock(1, SECONDS)`）、**可中断等待**、**多个条件队列**（生产者/消费者各一个 `Condition`）的场景。`synchronized` 做不到、`Lock` 能做到的两个：拿不到锁时超时返回，而不是死等；等待锁的过程中被 `interrupt()` 打断。

</details>

## 10. 高频坑

| 坑 | 说明 |
| --- | --- |
| `wait` 用 `if` | 虚假唤醒 + 条件被改，醒来直接往下执行导致越界/负数 |
| 用 `notify()` 不用 `notifyAll()` | 可能唤醒同类线程，真正的目标没被叫醒，导致假死 |
| `wait()` 不在 `synchronized` 里 | `IllegalMonitorStateException` |
| `Lock` 忘了 `unlock()` | 线程永久阻塞，且没有任何异常提示，只能 jstack 查 |
| 在锁里 `sleep` | 锁被占着白白浪费，临界区应尽量短 |
| `sleep` 唤醒后以为条件成立 | `sleep` 期间条件可能已变，醒来要重新判断 |

## 11. 30 秒卡

- **`wait` 释放锁，`sleep` 不释放**——这是两者最本质的区别。
- **`wait` 必须在 `synchronized` 里，且必须包在 `while` 里**（防虚假唤醒）。
- **被 `notify` 后不是立刻执行**，而是从 `WAITING` → `BLOCKED` → 抢到锁再继续。
- **优先 `notifyAll`**，`notify` 可能造成唤醒丢失。
- **`Lock` 三件套**：`lock()` / `unlock()`（放 `finally`）/ 多 `Condition` 精确唤醒。

## 12. 今天要记的 3 个术语（写进术语表）

- `wait/notify/notifyAll` / 虚假唤醒 / `IllegalMonitorStateException`
- 生产者-消费者 / 缓冲区 / 唤醒丢失
- `ReentrantLock` / `Condition.await/signal` / `tryLock` / 公平锁

## 13. 面试一句话

> **`wait()` 和 `sleep()` 有什么区别？**
> "`wait()` 属于 `Object`，必须在 `synchronized` 块里调用，调用时**释放锁**，进入 `WAITING`，需要别的线程 `notify` 唤醒，醒来后还要重新抢锁；`sleep()` 属于 `Thread`，任何地方都能调，**不释放锁**，进入 `TIMED_WAITING`，时间到自动醒。本质区别是：`wait` 是为了等条件（条件要别人来改变，所以必须让出锁），`sleep` 只是暂停（不依赖别人，所以保持锁更安全）。另外 `wait` 醒来后必须重新判断条件，所以要包在 `while` 里防虚假唤醒。"

## 14. 明天预告

**W5 D4（11/26）：线程池**——这周信息密度最高的一天。七个参数、四种拒绝策略、任务在池子里的流转顺序，还有 `ExecutorService.submit()` 返回的 `Future` 怎么用。建议早上先翻一遍标题，心里有个数。
