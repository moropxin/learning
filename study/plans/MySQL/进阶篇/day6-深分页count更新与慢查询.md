# SQL 优化（二）：深分页、count、update 与慢查询（进阶篇 · Day 6）

> 一句话：**深分页慢在"扫了再丢"；count 慢在"只能数"；update 慢/锁全表是因为条件列没索引；慢查询日志的作用是让"哪条 SQL 慢"从猜变成查。**
> 对应计划：W1 D6｜前置：Day 4（EXPLAIN）、Day 5（插入/排序/分组）｜学完能答：`limit 100000,10` 怎么优化、`count(*)` 和 `count(列)` 差在哪、无索引 update 为什么锁全表、怎么打开慢查询日志。

## 0. 今天 3 小时怎么用

| 时间 | 做什么 | 产出 |
| --- | --- | --- |
| 25 min | 读 §1–§4 | 记住「分页三种改法 + count 三个结论 + 锁的边界」 |
| 45 min | 看黑马进阶篇「SQL 优化-limit / count / update」章节 | 跟完课程 |
| 65 min | §5 实验：三种分页计时、count 对比、锁实验、开慢日志 | 优化前后耗时对比表 |
| 35 min | §6 练习 | 答题区写满 |
| 10 min | 笔记 + commit | 今天的产出 |

## 1. 深分页：慢在"扫了再丢"

```mermaid
flowchart LR
    A["limit 90000,10"] --> B["按 id 扫 90010 行"]
    B --> C["丢掉前 90000 行"]
    C --> D["只返回 10 行"]
```

`limit offset, n` 不是"跳到第 offset 行"，而是**从头扫、数到 offset 再取 n 行**。offset 越大，白扫的行越多，回表也越多。

| 写法 | SQL | 原理 | 适用 |
| --- | --- | --- | --- |
| 原始 ❌ | `select * from t order by id limit 90000,10` | 扫 90010 行 + 回表 90010 次 | — |
| ① 延迟关联 ✅ | `select o.* from t o join (select id from t order by id limit 90000,10) x on o.id = x.id` | 子查询只在**索引**里扫（覆盖索引，不回表），拿到 10 个 id 再回表 10 次 | 可跳页，最常用 |
| ② 书签/游标 ✅ | `select * from t where id > 90000 order by id limit 10` | 直接定位起点，扫 10 行 | 只能"下一页"，不能跳页 |
| ③ 子查询 in ⚠️ | `select * from t where id in (select id from t order by id limit 90000,10)` | 效果接近 ①，但优化器改写不稳定 | 兼容老版本 |

**代价对比（10 万行、`limit 90000,10`）**：原始 ≈ 扫 9 万行 + 9 万次回表；延迟关联 ≈ 扫 9 万行索引项（无回表）+ 10 次回表；书签 ≈ 10 行。

> 注意：延迟关联里子查询的 `order by` 必须能被索引满足（比如主键 `id`），否则子查询自己就要 filesort。

## 2. count 优化：InnoDB 只数不存

| 写法 | 语义 | 说明 |
| --- | --- | --- |
| `count(*)` | 行数 | **InnoDB 会挑最小的二级索引**来扫，不读整行 |
| `count(1)` | 行数 | 和 `count(*)` 等价（8.0 优化器已同化） |
| `count(主键)` | 行数 | 走聚簇索引，通常比 `count(*)` 慢 |
| `count(列)` | 该列非 NULL 的个数 | 语义不同；列可空时要逐行判断，最慢 |

**为什么 MyISAM 快、InnoDB 慢？** MyISAM 把总行数存在表里，直接读；InnoDB 因为 **MVCC**，同一时刻不同事务看到的行数不同，"总数"没法缓存成一个固定值，只能扫索引数。

**优化手段（按场景）**：

| 场景 | 做法 |
| --- | --- |
| 只要"大概多少" | `SHOW TABLE STATUS` 的 `Rows`（估算值）或 `information_schema.TABLES.TABLE_ROWS` |
| 分页显示总数 | 缓存计数 / 用"是否有下一页"替代精确总数 |
| 频繁统计 | 建汇总表，写时 +1，读时直接查 |
| 必须精确且量大 | 减小扫描范围：`where` 先过滤 + 覆盖索引；分批 `count` 后相加 |

## 3. update 优化：没有索引就是锁全表

InnoDB 的行锁**加在索引上**，不是加在行上。

| 更新条件 | 锁的范围 | 后果 |
| --- | --- | --- |
| 主键 / 唯一索引等值 | 只锁命中的行 | 并发好 |
| 普通索引等值 | 命中行 + 间隙（RR 下） | 可接受 |
| **没有索引** | 扫到的**所有行**都加锁（实为锁全表） | 别人改任何一行都被阻塞，极易死锁 |

**update 四条建议**：

1. 更新条件列**必须有索引**，否则先加索引；
2. 不要更新主键（要挪动聚簇索引里的行，代价大）；
3. 少更新大字段（`TEXT`/`BLOB` 走行溢出页）；
4. 大批量更新**拆小批**（每批几千行），避免长事务持锁。

## 4. 慢查询日志：把"猜"变成"查"

| 参数 | 作用 | 建议值 |
| --- | --- | --- |
| `slow_query_log` | 开关 | `ON` |
| `long_query_time` | 超过多少秒记录 | 生产 1s，学习 0.5s |
| `log_queries_not_using_indexes` | 记录没走索引的 SQL | 临时开，长期开会爆日志 |
| `slow_query_log_file` | 日志路径 | 默认在数据目录 |
| `log_output` | `FILE` / `TABLE` | FILE 方便 `mysqldumpslow` 分析 |

**三种定位手段**：

1. **慢日志文件**：`mysqldumpslow -s t -t 10 slow.log` 看 Top10；
2. **`performance_schema`**：按"SQL 指纹"聚合，不用装工具就能排序；
3. **`EXPLAIN ANALYZE`**：对可疑 SQL 看真实执行耗时。

> `SHOW PROFILE` 在 8.0.35 起已废弃，新版本用 `performance_schema.events_statements_summary_by_digest`。

## 5. 动手实验（65 分钟，代码带注释）

> 沿用 Day 5 的 `lab.t_opt`（10 万行）。如果重建过库，先跑 Day 5 §5 的建表 + 造数。

```sql
USE lab;
SELECT COUNT(*) FROM t_opt;   -- 确认 100000 行
```

```sql
-- ---------- 实验 A：三种分页写法 ----------
-- ① 原始写法：扫 90010 行 + 回表 90010 次
EXPLAIN SELECT * FROM t_opt ORDER BY id LIMIT 90000, 10;

-- ② 延迟关联：子查询只查 id（覆盖索引，不回表），外层 join 只回表 10 次
--    外层 o.* 需要整行，所以要 join 回 t_opt；派生表 x 只含 id
EXPLAIN SELECT o.* FROM t_opt o
JOIN (SELECT id FROM t_opt ORDER BY id LIMIT 90000, 10) x ON o.id = x.id;

-- ③ 书签：记住上一页最后一个 id，从它之后取
--    只能"下一页"，不能跳页，但代价最小
EXPLAIN SELECT * FROM t_opt WHERE id > 90000 ORDER BY id LIMIT 10;

-- ④ 对照：小 offset 时三者差别不大
EXPLAIN SELECT * FROM t_opt ORDER BY id LIMIT 10;
```

```sql
-- ---------- 实验 B：count 的四种写法 ----------
EXPLAIN SELECT COUNT(*) FROM t_opt;        -- key 往往是某个二级索引（最小的那棵）
EXPLAIN SELECT COUNT(1) FROM t_opt;        -- 与 count(*) 等价
EXPLAIN SELECT COUNT(id) FROM t_opt;       -- 走聚簇索引
EXPLAIN SELECT COUNT(memo) FROM t_opt;     -- memo 可空：要逐行判 NULL，最慢且语义不同

-- 近似值：不扫表，直接读统计信息（误差可能较大）
SHOW TABLE STATUS WHERE Name = 't_opt';
SELECT TABLE_ROWS FROM information_schema.TABLES
WHERE TABLE_SCHEMA = 'lab' AND TABLE_NAME = 't_opt';
```

```sql
-- ---------- 实验 C：update 的锁边界（需要两个窗口）----------
-- 【窗口 A】
START TRANSACTION;
UPDATE t_opt SET memo = 'lock-test' WHERE name = 'user500';   -- name 有索引 → 行锁

-- 【窗口 B】看锁：performance_schema.data_locks 是 8.0 查看行锁/间隙锁的表
SELECT ENGINE_TRANSACTION_ID, OBJECT_NAME, INDEX_NAME, LOCK_TYPE, LOCK_MODE, LOCK_DATA
FROM performance_schema.data_locks
WHERE OBJECT_NAME = 't_opt';

-- 【窗口 A】
ROLLBACK;

-- 【窗口 A】对比：条件列无索引 → 全表扫描 + 锁所有扫描到的行
START TRANSACTION;
UPDATE t_opt SET memo = 'lock-test2' WHERE memo = 'memo-1';   -- memo 无索引！

-- 【窗口 B】此时 LOCK_DATA 会列出大量行，其他事务改任何一行都可能被阻塞
SELECT COUNT(*) FROM performance_schema.data_locks WHERE OBJECT_NAME = 't_opt';

-- 【窗口 A】
ROLLBACK;
```

```sql
-- ---------- 实验 D：慢查询日志 ----------
-- 打开慢日志（需要 SYSTEM_VARIABLES_ADMIN 权限，本机 root 可以）
SET GLOBAL slow_query_log = ON;
SET GLOBAL long_query_time = 0.5;              -- 超过 0.5 秒就记录（默认 10 秒，太粗）
SHOW VARIABLES LIKE 'slow_query_log%';
SHOW VARIABLES LIKE 'long_query_time';

-- 造一条必然慢的 SQL：前导模糊 + 大表，故意不走索引
SELECT COUNT(*) FROM t_opt WHERE memo LIKE '%memo-1%';
SELECT SLEEP(1);                                -- SLEEP 也会被记进慢日志，方便验证

-- 看日志文件路径，然后用文本编辑器打开最后几行
SHOW VARIABLES LIKE 'slow_query_log_file';
```

```sql
-- ---------- 实验 E：按"指纹"找最耗时的 SQL（不装任何工具）----------
-- events_statements_summary_by_digest：把结构相同的 SQL 归并成一条，按总耗时排序
-- TIMER 单位是皮秒（ps），除以 1e9 得到毫秒
SELECT
  LEFT(DIGEST_TEXT, 60)                     AS sql_digest,   -- 只截前 60 个字符看
  COUNT_STAR                                AS exec_count,   -- 执行次数
  ROUND(AVG_TIMER_WAIT / 1e9, 2)            AS avg_ms,       -- 平均耗时（毫秒）
  ROUND(SUM_TIMER_WAIT / 1e12, 2)           AS total_s       -- 总耗时（秒）
FROM performance_schema.events_statements_summary_by_digest
ORDER BY SUM_TIMER_WAIT DESC
LIMIT 10;
```

**记录表（优化前后各跑一次，把数字填进来 —— 这就是今天的产出）**

| 场景 | 写法 | type / key | 耗时（ms） | 备注 |
| --- | --- | --- | --- | --- |
| 深分页 | 原始 `limit 90000,10` | | | |
| 深分页 | 延迟关联 | | | |
| 深分页 | 书签 `id > 90000` | | | |
| count | `count(*)` | | | |
| count | `count(memo)` | | | |
| update | `where name`（有索引） | | | 锁了几行 |
| update | `where memo`（无索引） | | | 锁了几行 |

## 6. 练习（35 分钟，直接写答题区）

### A. 判断题（6 题）

**1.** `limit 100000, 10` 会直接跳到第 10 万行开始读。
> 答：

**2.** 延迟关联之所以快，是因为子查询只读索引、不用回表。
> 答：

**3.** `count(*)` 比 `count(列)` 慢，因为要读整行。
> 答：

**4.** InnoDB 不缓存总行数，是因为 MVCC 让不同事务看到不同行数。
> 答：

**5.** `update ... where 无索引列` 会锁全表。
> 答：

**6.** `long_query_time` 默认是 1 秒。
> 答：

### B. 选择题（4 题）

**1. 深分页最适合的通用改法是？**
A. 加 `limit` 调大　B. 延迟关联（先用覆盖索引取 id 再 join）　C. 加 `order by rand()`　D. 改 `count(*)`
> 答：

**2. 想"大概知道表有多少行"，最快的办法是？**
A. `count(*)`　B. `SHOW TABLE STATUS` 的 `Rows`　C. `count(主键)`　D. 遍历全表
> 答：

**3. `update` 时最危险的条件是？**
A. 主键等值　B. 唯一索引等值　C. 无索引列　D. 普通索引等值
> 答：

**4. 8.0 里查"哪条 SQL 总耗时最高"，推荐用？**
A. `SHOW PROFILE`　B. `performance_schema.events_statements_summary_by_digest`　C. `SHOW PROCESSLIST`　D. `EXPLAIN`
> 答：

### C. 简答题（3 题）

**1. 深分页为什么慢？写出两种优化写法并说明各自的限制。**
> 答：
>
>

**2. 为什么 InnoDB 的 `count(*)` 不能像 MyISAM 一样 O(1)？**
> 答：
>
>

**3. 一条 update 把线上锁住了，你怎么排查？**
> 答：
>
>

### D. 改写题（2 题）

**1.** `select * from t_opt order by id limit 500000, 20;`
> 答：

**2.** `update t_opt set memo = 'x' where memo = 'memo-1';`
> 答：

<details><summary><b>参考答案（先写满答题区再点开）</b></summary>

**A. 判断题**

1. ❌ 错：要**从头扫**到 offset 再取，前面 10 万行都会被读出来再丢掉。
2. ✅ 对：子查询只查 `id`，走覆盖索引不回表；外层只对 10 行回表。
3. ❌ 错：`count(*)` 反而更快，InnoDB 会挑最小的二级索引扫，不读整行；`count(可空列)` 才慢。
4. ✅ 对：MVCC 下不同事务的快照可见行数不同，"总数"没有唯一值可缓存。
5. ✅ 对：没有索引就只能全表扫描，扫到的行全部加锁（RR 下还有间隙锁），表现为锁全表。
6. ❌ 错：默认是 **10 秒**，所以很多慢 SQL 根本没被记录，通常要调到 1 秒甚至 0.5 秒。

**B. 选择题**

1. **B**：延迟关联通用且能跳页；书签更便宜但不能跳页。
2. **B**：`SHOW TABLE STATUS` 读的是统计信息，不扫表（估算值，可能偏差大）。
3. **C**：无索引 → 全表扫描 → 所有行加锁。
4. **B**：按 SQL 指纹聚合，自带次数与耗时；`SHOW PROFILE` 已废弃。

**C. 简答题（要点）**

1. 慢在"扫了再丢"：`limit` 不是跳转，而是扫描 `offset + n` 行并丢弃前 offset 行，回表次数也随 offset 增长。①**延迟关联**：子查询用覆盖索引只取 id，外层 join 回表 10 次 —— 可以跳页，但子查询的排序必须走索引；②**书签/游标**：`where id > last_id limit n` —— 代价最小，但只能顺序翻页、不能跳页，且要求排序键唯一有序。
2. InnoDB 支持 MVCC，每行有隐藏的事务版本信息，不同事务/不同时刻看到的行数不同，无法像 MyISAM 那样把总行数存成静态值；只能扫描索引实时统计（优化器会选最小的二级索引以减少 IO）。
3. ①`SHOW PROCESSLIST` 看是否有长事务/大量 `Waiting for lock`；②`performance_schema.data_locks` / `data_lock_waits` 看锁了哪些行、谁在等谁；③`EXPLAIN` 该 update 的 where 条件，确认是否走索引；④确认事务隔离级别（RR 间隙锁更容易冲突）；⑤`information_schema.innodb_trx` 看事务年龄，必要时 kill 长事务（慎）。

**D. 改写题**

1. 延迟关联：`select o.* from t_opt o join (select id from t_opt order by id limit 500000, 20) x on o.id = x.id;`（或业务上改书签翻页 `where id > 500000 order by id limit 20`）
2. 先给条件列建索引再更新：`ALTER TABLE t_opt ADD INDEX idx_memo (memo(50));`（前缀索引即可，`memo` 太长不做全列索引），再执行原 update —— 锁范围从"全表"缩到"命中的行"。

</details>

## 7. 高频坑

| 坑 | 说明 | 正确做法 |
| --- | --- | --- |
| 以为 `limit` 能跳页 | 它是"扫完再丢" | 大 offset 用延迟关联或书签 |
| 延迟关联忘了排序走索引 | 子查询里 `order by` 无索引 → 自己 filesort，白忙 | 确认子查询 `type=index` |
| 用 `count(列)` 当 `count(*)` | 列可空时语义都变了 | 数行数就用 `count(*)` |
| 长期开 `log_queries_not_using_indexes` | 日志暴涨，磁盘写满 | 排查期临时开 |
| `long_query_time` 用默认 10s | 大部分慢 SQL 抓不到 | 调到 1s 或 0.5s |
| 无索引 update 只在测试库试 | 生产直接锁表 | 上线前用 `EXPLAIN` 验证 update 的 where |

## 8. 30 秒复习卡

```text
深分页：扫了再丢 → 延迟关联（覆盖索引取 id 再 join）/ 书签 where id > last
count：count(*) 走最小二级索引；MyISAM 存行数，InnoDB 因 MVCC 只能数
update：行锁加在索引上，无索引 = 锁全表
慢日志：slow_query_log=ON、long_query_time=1、log_queries_not_using_indexes 临时开
排 Top SQL：events_statements_summary_by_digest 按 SUM_TIMER_WAIT 排序
```

## 9. 面试一句话

> **`limit 100000,10` 怎么优化？**
> 它是"先扫 10 万行再丢掉"，所以要做"少扫"：延迟关联——子查询用覆盖索引只取 10 个 id，再 join 回表；或者书签翻页 `where id > last_id limit 10`，代价最小但不能跳页。
>
> **update 为什么会锁表？**
> InnoDB 的行锁加在索引上，`where` 列没有索引时只能全表扫描，扫描到的所有行都会加锁（RR 下还有间隙锁），表现为锁全表并引发阻塞与死锁；所以 update 的条件列必须先有索引。
