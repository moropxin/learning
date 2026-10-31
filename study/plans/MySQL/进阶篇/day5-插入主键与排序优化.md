# SQL 优化（一）：插入、主键、排序、分组（进阶篇 · Day 5）

> 一句话：**写快靠"少提交、顺序写、少索引"；读快靠"一个索引同时伺候 where、order by、group by"**。
> 对应计划：W1 D5｜前置：Day 4（EXPLAIN 五列 + 索引失效）｜学完能答：批量导入怎么提速、主键为什么用自增、`Using filesort` 怎么消、`group by` 的临时表怎么省。

## 0. 今天 3 小时怎么用

| 时间 | 做什么 | 产出 |
| --- | --- | --- |
| 20 min | 读 §1–§4 | 记住「插入 4 招 + 排序 3 条件」 |
| 50 min | 看黑马进阶篇「SQL 优化-插入 / 主键 / order by / group by」 | 跟完课程 |
| 60 min | §5 实验：三种插入对比、顺序 vs UUID、排序/分组 EXPLAIN | 一张计时表 + 6 条 EXPLAIN |
| 35 min | §6 练习 | 答题区写满 |
| 15 min | 笔记 + commit | 今天的产出 |

## 1. 插入优化：把 1000 次提交压成 1 次

| # | 招 | 做法 | 为什么快 |
| --- | --- | --- | --- |
| 1 | **批量插** | 一条 `INSERT` 带多个 `values`，每批 500–1000 条 | 少解析、少网络往返 |
| 2 | **手动事务** | `SET autocommit = 0` 后批量插，最后 `COMMIT` | 每次提交都要刷 redo log（fsync），1000 次提交 = 1000 次刷盘 |
| 3 | **顺序主键** | 用自增 / 雪花 ID，别用随机值 | 只在最后一页追加，不触发页分裂 |
| 4 | **临时关校验** | 导入前 `SET UNIQUE_CHECKS=0; SET FOREIGN_KEY_CHECKS=0;`，导入后改回 `1` | 省掉逐行唯一性 / 外键检查 |
| 5 | **文件导入** | `LOAD DATA LOCAL INFILE`（需 `local_infile=1`） | 跳过 SQL 解析层，比 `INSERT` 快一个数量级 |
| 6 | **少建索引** | 导入前可先删非必需索引，导完再建 | 每个二级索引都要跟着写一遍 B+ 树 |

> 批次不是越大越好：超过 `max_allowed_packet` 会直接报错（8.0 默认 64MB，1000 行远远够用）。

## 2. 主键优化：页分裂是乱序插入的代价

```mermaid
flowchart LR
    A["页 P1<br/>10 20 30"] --> B["插 15"]
    B --> C["拆成两页<br/>P1: 10 15"]
    C --> D["P2: 20 30"]
    D --> E["父页加指针<br/>P1 → P2"]
```

- **顺序插入**：自增值永远落在最右页，满了就从新页继续 —— 只追加，不挪数据。
- **乱序插入**（UUID、随机数）：值落在中间页，页满了要**把一半数据搬到新页**，再改父页指针 → 一次插入触发多次页复制。
- **页合并**：删除大量数据后，相邻页利用率过低时 InnoDB 会合并，同样有代价。

| 主键选型 | 长度 | 顺序性 | 结论 |
| --- | --- | --- | --- |
| `BIGINT UNSIGNED AUTO_INCREMENT` | 8B | ✅ | 单库单表首选 |
| `INT UNSIGNED AUTO_INCREMENT` | 4B | ✅ | 够用且更省，注意 42 亿上限 |
| `UUID`（`CHAR(36)`） | 36B | ❌ | 页分裂 + 索引变大，别当聚簇索引 |
| 雪花 ID（`BIGINT`） | 8B | 近似顺序 | 分库分表 / 多写入点时的替代 |
| 业务字符串（订单号） | 变长 | ❌ | 二级索引里还要存一份，代价翻倍 |

**主键五条原则**：尽量短 → 顺序递增 → 不修改 → 不用随机值 → 用无符号整型。

## 3. order by 优化：什么情况下排序不用额外做

`Extra` 里出现 **`Using filesort`** = 索引没帮上排序的忙，MySQL 要把结果捞进 `sort_buffer` 排一遍（数据量大就落磁盘）。

**能用索引排序的三个条件（缺一不可）**：

1. `where` 里对索引**前面的列都是等值**（不能是范围）；
2. `order by` 的列顺序与索引**从左到右一致**；
3. 所有列的排序方向**一致**（8.0 支持 `index (a ASC, b DESC)` 建降序索引，混排也能走）。

```text
索引 (status, create_time)

where status = 1 order by create_time        ✅ Using index
where status = 1 order by name               ❌ filesort（name 不在索引里）
where status > 1 order by create_time        ❌ filesort（范围之后无序）
order by create_time                         ❌ filesort（跳过最左列）
order by status, create_time                 ✅ 全索引扫描，天然有序
```

**优化手段**（按代价从低到高）：

| 手段 | 做法 |
| --- | --- |
| 改索引 | 把 `order by` 的列并进联合索引，如 `(status, name)` |
| 缩小排序量 | `where` 先过滤，`limit` 提前截断 |
| 覆盖索引 | 只 `select` 索引里有的列，排序不必回表 |
| 调参数 | `sort_buffer_size` 调大，避免落磁盘（治标） |

## 4. group by 优化：别让它建临时表

`Extra` 里 **`Using temporary`** = 分组用上了临时表（内存放不下就落磁盘），通常还伴随 `Using filesort`。

**能走索引的条件**：`group by` 的列必须是索引的**最左前缀**；若 `where` 里已对最左列做了等值，`group by` 可以直接用后面的列。

```text
索引 (status, create_time)

group by status                        ✅ Using index for group-by（松散索引扫描）
where status = 1 group by create_time  ✅ 等值 + 后续列，天然有序
group by create_time                   ❌ temporary（跳过最左列）
group by DATE(create_time)             ❌ temporary + filesort（列上有函数）
```

> **8.0 的变化**：`group by` 不再隐式排序，网上"加 `order by null` 消除排序"的老技巧已经没用了 —— 这点面试常考。

## 5. 动手实验（60 分钟，代码带注释）

```sql
CREATE DATABASE IF NOT EXISTS lab DEFAULT CHARSET utf8mb4;
USE lab;

-- ---------- 建表：10 万行 ----------
DROP TABLE IF EXISTS t_opt;
CREATE TABLE t_opt (
  id          BIGINT PRIMARY KEY AUTO_INCREMENT,
  name        VARCHAR(50)  NOT NULL,
  status      TINYINT      NOT NULL,
  create_time DATETIME     NOT NULL,
  memo        VARCHAR(200),
  KEY idx_status_ct (status, create_time),   -- 联合索引：等值 + 排序/分组都用它
  KEY idx_name (name)
) ENGINE = InnoDB;

-- 递归 CTE 造 10 万行：一条 INSERT 就完成，本身就是"批量插入"的示范
SET SESSION cte_max_recursion_depth = 100000;
INSERT INTO t_opt (name, status, create_time, memo)
WITH RECURSIVE seq(n) AS (
  SELECT 1
  UNION ALL
  SELECT n + 1 FROM seq WHERE n < 100000
)
SELECT
  CONCAT('user', n),
  n % 5,                                        -- status 只有 0~4，区分度低
  NOW() - INTERVAL (n % 365) DAY,               -- INTERVAL 做日期加减
  CONCAT('memo-', n)
FROM seq;

SELECT COUNT(*) FROM t_opt;   -- 应为 100000
```

```sql
-- ---------- 实验 A：三种插入方式 ----------
DROP TABLE IF EXISTS t_ins; CREATE TABLE t_ins LIKE t_opt;

-- ① 单条 + 自动提交：客户端循环 1000 次（这里示范 2 条），记下总耗时
SET autocommit = 1;
INSERT INTO t_ins (name, status, create_time) VALUES ('a', 1, NOW());

-- ② 手动事务 + 批量 values：1000 行进 1 个事务（示意，实际用程序或脚本拼）
SET autocommit = 0;
INSERT INTO t_ins (name, status, create_time) VALUES
  ('b', 1, NOW()), ('c', 1, NOW()), ('d', 1, NOW());
COMMIT;
SET autocommit = 1;     -- 用完记得改回来，否则后续每条都不提交

-- ③ INSERT ... SELECT：一条语句完成"查 + 插"，没有 1000 次往返
INSERT INTO t_ins (name, status, create_time, memo)
SELECT name, status, create_time, memo FROM t_opt LIMIT 10000;
```

```sql
-- ---------- 实验 B：顺序主键 vs UUID ----------
DROP TABLE IF EXISTS t_pk_seq, t_pk_uuid;
CREATE TABLE t_pk_seq  (id BIGINT    PRIMARY KEY, payload VARCHAR(50)) ENGINE = InnoDB;
CREATE TABLE t_pk_uuid (id CHAR(36)  PRIMARY KEY, payload VARCHAR(50)) ENGINE = InnoDB;

-- 顺序插入：始终在最后一页追加
INSERT INTO t_pk_seq (id, payload)
WITH RECURSIVE seq(n) AS (SELECT 1 UNION ALL SELECT n + 1 FROM seq WHERE n < 100000)
SELECT n, CONCAT('p', n) FROM seq;

-- 随机 UUID 插入：值散落在所有页中间
INSERT INTO t_pk_uuid (id, payload)
WITH RECURSIVE seq(n) AS (SELECT 1 UNION ALL SELECT n + 1 FROM seq WHERE n < 100000)
SELECT UUID(), CONCAT('p', n) FROM seq;

-- 对比：Data_free 是页内空闲字节，越大说明碎片越多（页分裂的痕迹）
SHOW TABLE STATUS WHERE Name IN ('t_pk_seq', 't_pk_uuid');
```

```sql
-- ---------- 实验 C：order by 能不能用索引 ----------
EXPLAIN SELECT * FROM t_opt WHERE status = 1 ORDER BY create_time;             -- Using index（等值 + 后续列）
EXPLAIN SELECT * FROM t_opt WHERE status = 1 ORDER BY name;                    -- Using filesort
EXPLAIN SELECT * FROM t_opt WHERE status > 1 ORDER BY create_time;             -- Using filesort（范围后无序）
EXPLAIN SELECT * FROM t_opt ORDER BY status, create_time;                      -- 全索引扫描，天然有序
EXPLAIN SELECT id, status, create_time FROM t_opt WHERE status = 1 ORDER BY create_time;  -- 覆盖索引
EXPLAIN SELECT * FROM t_opt WHERE status = 1 ORDER BY create_time DESC;        -- 反向扫描，仍 Using index
```

```sql
-- ---------- 实验 D：group by 有没有临时表 ----------
EXPLAIN SELECT status, COUNT(*) FROM t_opt GROUP BY status;                    -- Using index for group-by
EXPLAIN SELECT status, create_time, COUNT(*) FROM t_opt GROUP BY status, create_time;
EXPLAIN SELECT status, COUNT(*) FROM t_opt WHERE status = 1 GROUP BY create_time;  -- 等值 + 后续列
EXPLAIN SELECT create_time, COUNT(*) FROM t_opt GROUP BY create_time;          -- temporary（跳最左）
EXPLAIN SELECT DATE(create_time), COUNT(*) FROM t_opt GROUP BY DATE(create_time);  -- temporary + filesort
```

**记录表（把实测结果填进来）**

| 实验 | SQL | type | key | Extra | 耗时 |
| --- | --- | --- | --- | --- | --- |
| A① | 单条插入 | all | null | null | 362ms|
| A③ | INSERT…SELECT 1 万行 | all | null | null | 341ms |
| B | 顺序主键 10 万行 | all | null | null | 353ms|
| B | UUID 主键 10 万行 | all | null | null | 360ms |
| C | `order by create_time` | ref | idx_status_ct | null | 355ms |
| C | `order by name` | ref | idx_status_ct | Using filesort | 353ms |
| D | `group by status` | index | idx_status_ct | Using index | 351ms |
| D | `group by DATE(...)` | index | idx_status_ct | Using index & Using temporary | 375ms |

## 6. 练习（35 分钟，直接写答题区）

### A. 判断题（6 题）

**1.** 一条 `INSERT` 带 1000 个 `values`，通常比 1000 条单行 `INSERT` 快。
> 答：对，无须1000次往返和解析

**2.** 主键用 UUID 更好，因为全局唯一、不冲突。
> 答：错，值落在页中，会频繁页分裂，表长更长，应该用雪花ID

**3.** 只要 `order by` 的列上建了索引，就一定能消除 `Using filesort`。
> 答：错，需要where与前序列等值，order by后和索引方向、顺序一致

**4.** MySQL 8.0 里 `group by` 还会隐式排序，所以要加 `order by null` 优化。
> 答：错，8.0已经没有隐式排序了，5.7版本取消了此优化

**5.** 大批量导入时可以临时关闭唯一性检查，导完再打开。
> 答：对，把`SET UNIQUE_CHECKS=0; SET FOREIGN_KEY_CHECKS=0;`执行可省略多次效验，导完要将其重新置为1

**6.** 二级索引建得越多，插入速度越快。
> 答：错，二级索引跟着B+TREE创建，数据会糅杂

### B. 选择题（4 题）

**1. 导入 100 万行数据，最快的是？**
A. 循环单条 INSERT　B. 批量 INSERT + 手动事务　C. `LOAD DATA LOCAL INFILE`　D. 建满索引再插
> 答：C

**2. `Extra: Using filesort` 表示？**
A. 排序走了索引　B. 需要额外排序，索引没帮上忙　C. 用了临时表　D. 覆盖索引
> 答：B

**3. 下列**最不适合**做主键的是？**
A. `BIGINT AUTO_INCREMENT`　B. `UUID` 字符串　C. 雪花 ID　D. `INT UNSIGNED AUTO_INCREMENT`
> 答：B

**4. `group by` 出现 `Using temporary` 说明？**
A. 分组走了索引　B. 分组用上了临时表　C. 用了覆盖索引　D. 没有索引
> 答：B

### C. 简答题（3 题）

**1. 批量导入数据，你会做哪 4 件事提速？**
> 答：
> N1.批量"values" N2.关闭自动事务、手动提交，减少校验
> N3.导入期间将数据唯一性、外键关联性检查关闭 N4.数据量大用‘load data’

**2. `order by` 想用上索引排序，需要满足哪 3 个条件？**
> 答：
> where条件要为等值条件，order by后置索引要跟前置顺序、排序方向一致
>

**3. 主键为什么建议「短 + 顺序」？分别从索引大小和页分裂两个角度说。**
> 答：
>短值索引小，聚簇索引存短值索引可以防止二级索引“ 索引列+主键 ”因存入长值索引导致行变长，存放索引列变短导致树深增加
>顺序索引自增值只在尾端插入，不会出现页分裂；随机值插到中间页，页满要搬一半数据，插入变慢且碎片多


### D. 改写题（2 题）

**1.** `select * from t_opt where date(create_time) = '2026-10-30' order by create_time;`
> 答：select * from t_opt where create_time >= '2026-10-30' and create_time < '2026-10-31' order by create_time

**2.** `select id, name from t_opt where status = 1 order by name;`（当前只有 `idx_status_ct`，出现 filesort）
> 答：alter table t_opt add index idx_status_name (status,name);建联合索引和id主       键一起走覆盖查询

<details><summary><b>参考答案（先写满答题区再点开）</b></summary>

**A. 判断题**

1. ✅ 对：少了解析和往返，`values` 越多越明显（单批 500–1000 最稳）。
2. ❌ 错：UUID 无序，会频繁页分裂；36B 也比 8B 长，索引更大。全局唯一的需求用雪花 ID 更合适。
3. ❌ 错：还要 `where` 对前序列等值、`order by` 顺序与索引一致、方向一致。
4. ❌ 错：8.0 已移除 `group by` 的隐式排序，`order by null` 是 5.7 时代的老技巧。
5. ✅ 对：`SET UNIQUE_CHECKS=0; SET FOREIGN_KEY_CHECKS=0;` 省校验，导完必须恢复为 1。
6. ❌ 错：每个二级索引都要跟着写 B+ 树，索引越多写越慢。

**B. 选择题**

1. **C**：`LOAD DATA` 跳过 SQL 解析与逐行往返；B 次之；A 最慢。
2. **B**：索引没提供有序性，需要额外排序（内存 `sort_buffer`，装不下落磁盘）。
3. **B**：UUID 无序 + 36B，直接踩页分裂和索引膨胀两个坑。
4. **B**：分组需要临时表；通常还会伴随 `Using filesort`。

**C. 简答题（要点）**

1. ①批量 `values`（500–1000/批）；②手动事务/关 `autocommit`，减少提交刷盘；③顺序主键；④导入期临时关唯一性与外键检查，或先删索引后重建；⑤数据量大用 `LOAD DATA`。
2. ①`where` 对索引前列都是等值；②`order by` 列与索引从左到右一致；③排序方向一致（或用 8.0 降序索引）。
3. **短**：聚簇索引叶子存整行，二级索引叶子存"索引列 + 主键"，主键越长，每个二级索引都跟着变胖，一页能放的行变少、树变高。**顺序**：自增值总在最后一页追加，不触发页分裂；随机值插到中间页，页满要搬一半数据，插入变慢且碎片多。

**D. 改写题**

1. 去掉函数、改成范围：`select * from t_opt where create_time >= '2026-10-30' and create_time < '2026-10-31' order by create_time;`（`create_time` 上有索引时可直接 `range` + 有序）
2. 建一个能同时伺候 `where` 和 `order by` 的联合索引：`ALTER TABLE t_opt ADD INDEX idx_status_name (status, name);` —— `status` 等值在前、`name` 在后，排序即可复用索引顺序（`id` 是主键，`select id, name` 走覆盖索引，不回表）。

</details>

## 7. 高频坑

| 坑 | 说明 | 正确做法 |
| --- | --- | --- |
| 批量太大 | 超过 `max_allowed_packet` 直接报错 | 每批 500–1000，或调大该参数 |
| `autocommit=0` 忘了改回 | 后面的语句都没提交，一断线全丢 | 实验结束 `SET autocommit = 1` |
| `UNIQUE_CHECKS=0` 忘了恢复 | 之后插入脏数据也不报错 | 导入完立刻改回 1 |
| 见 `filesort` 就慌 | 结果集很小（几十几百行）时内存排序几乎无感 | 看 `rows` 和真实耗时再判断 |
| 以为 `group by` 还能靠 `order by null` | 8.0 已无隐式排序 | 直接用 `EXPLAIN` 看有没有 `temporary` |
| 主键用业务字符串 | 二级索引里要重复存，代价翻倍 | 用整型自增 / 雪花 ID |

## 8. 30 秒复习卡

```text
插入 4 招：批量 values、手动事务、顺序主键、临时关校验（LOAD DATA 更快）
主键：短 + 顺序 + 不改；UUID 会页分裂
order by 走索引：where 等值 + 顺序一致 + 方向一致 → Using index
group by：列是最左前缀（或 where 等值后紧跟）→ 无 temporary；8.0 不再隐式排序
```

## 9. 面试一句话

> **插入 100 万条数据怎么优化？**
> 批量 `values` 每批几百到一千条、放在一个事务里减少提交刷盘、用自增主键保证顺序插入，导入期临时关闭唯一性和外键检查，数据量大时直接上 `LOAD DATA LOCAL INFILE`，并把非必需索引延后创建。
>
> **怎么消除 `Using filesort` 和 `Using temporary`？**
> 前者要满足 `where` 等值 + `order by` 与索引同序同向，必要时把排序列并进联合索引；后者要求 `group by` 的列是索引最左前缀，或 `where` 已对最左列等值、`group by` 用后续列。
