# 索引分类：聚簇索引 · 二级索引 · 回表 · 覆盖 · 最左前缀（进阶篇 · Day 3）

> 一句话：**InnoDB 里其实有两棵树——聚簇索引（主键 → 整行）和二级索引（索引列 → 主键）；查非索引列要「回表」，用覆盖索引能省掉这一步，联合索引则要守「最左前缀」。**
> 对应计划：W1 D3｜前置：Day 2 的 B+ 树（非叶子存键、叶子存数据）｜学完能回答：回表是什么、怎么避免、联合索引为什么讲究顺序。

## 0. 今天 3 小时怎么用

| 时间 | 做什么 | 产出 |
| --- | --- | --- |
| 30 min | 读 §1–§5：聚簇/二级/回表/覆盖/最左前缀 | 能画「回表」路径 |
| 50 min | 看黑马进阶篇「索引-分类 / 索引-使用」章节 | 跟完课程 |
| 40 min | §6 动手：8 条 EXPLAIN，标出 type / key / Extra | 一张对比表 |
| 45 min | §7 巩固练习：8 选择 + 5 判断 + 3 简答 + 4 SQL 分析 | 正确率 ≥ 80% |
| 15 min | 写笔记、`git commit` | 今天的产出 |

> 原则：**今天只干一件事——把「回表」画出来**。画清楚"二级索引叶子 → 主键 → 聚簇索引叶子"，覆盖索引和最左前缀都是它的推论。

## 1. 先看全景：一张表有两棵树

```mermaid
flowchart LR
    Q["查询：where name = 'user500'"] --> SI["二级索引 idx_name<br/>叶子：name + 主键 id"]
    SI -->|"拿到 id = 500"| CI["聚簇索引（主键）<br/>叶子：整行数据"]
    CI --> R["返回整行 → 这一步叫「回表」"]
```

- **聚簇索引**：主键索引，叶子存**整行**。表的数据就长在这棵树上。
- **二级索引**：你自己建的普通索引，叶子只存**索引列 + 主键值**，不存整行。

所以：`select *` 走二级索引，一定要回表；只查索引列，就不用回表。

## 2. 聚簇索引（Clustered Index）

| 问题 | 答案 |
| --- | --- |
| 谁是聚簇索引？ | InnoDB 的**主键索引** |
| 叶子存什么？ | **一整行数据**（所有列） |
| 一张表有几个？ | **只能有 1 个**（数据只能按一种顺序组织） |
| 没有主键怎么办？ | 选第一个 `UNIQUE NOT NULL` 索引代替 |
| 连唯一索引也没有？ | InnoDB 生成隐藏的 `row_id`（6 字节）当聚簇索引 |
| 好处 | 主键查询一次 B+ 树就拿到整行；范围查询顺着叶子链表扫 |

> 记忆：**聚簇索引 = 表本身**。按主键查，等于顺着目录直接翻到那一页。

## 3. 二级索引（Secondary Index）

| 对比项 | 聚簇索引 | 二级索引 |
| --- | --- | --- |
| 建在什么上 | 主键 | 你指定的列（name/age…） |
| 叶子存什么 | 整行数据 | 索引列 + 主键值 |
| 数量 | 1 个 | 多个 |
| 查整行 | 一次到位 | 先查二级索引，再回表 |
| 典型用途 | 主键查询、范围扫描 | 按业务字段查 |

**二级索引叶子为什么存主键，不存行地址？**
因为行会移动（页分裂、数据更新），存地址会失效；存主键，行怎么动都能通过聚簇索引找回。

## 4. 回表：多一次 B+ 树查找

```text
where name = 'user500'
  → 走 idx_name：找到 name='user500' 的叶子，拿到主键 id=500
  → 拿 id=500 去聚簇索引再查一次：拿到整行
  → 返回
```

| 问题 | 说明 |
| --- | --- |
| 回表几次 IO | 二级索引 2–3 次 + 聚簇索引 2–3 次，比只走主键多一倍 |
| 回表为什么慢 | 第二次查的是**随机 IO**（主键可能分散在不同页） |
| 什么时候必须回表 | 查询列里有**不在二级索引里**的列（比如 `select *`） |
| 怎么避免 | ①覆盖索引 ②把常用列放进联合索引 ③尽量别 `select *` |

## 5. 覆盖索引：要的列，索引里都有

**定义**：查询涉及的列，全部包含在某个索引里（二级索引叶子 = 索引列 + 主键），就不用回表。

| SQL（表有 `idx_name(name)`，主键 id） | 是否覆盖 | 原因 |
| --- | --- | --- |
| `select name from t where name='x'` | ✅ 覆盖 | 只要 name，索引里有 |
| `select id, name from t where name='x'` | ✅ 覆盖 | 二级索引叶子自带主键 id |
| `select id from t where name='x'` | ✅ 覆盖 | 同上 |
| `select * from t where name='x'` | ❌ 回表 | 要 age/city 等索引外的列 |
| `select age from t where name='x'` | ❌ 回表 | age 不在 idx_name 里 |

> EXPLAIN 里看到 **`Extra: Using index`** 就是覆盖索引（注意：`Using index` ≠ `Using index condition`，后者是索引下推）。

**怎么用**：把「查询频繁 + 列不多」的字段做成联合索引，让常见查询刚好覆盖。

## 6. 联合索引与最左前缀

联合索引 `(a, b, c)` 的排序规则：

```text
先按 a 排序 → a 相同再按 b → b 相同再按 c
```

所以它天然支持「从最左列开始、连续匹配」的条件：

| 查询条件 | 能否用上 `(a,b,c)` | 实际用到的列 |
| --- | --- | --- |
| `a = 1` | ✅ | a |
| `a = 1 and b = 2` | ✅ | a, b |
| `a = 1 and b = 2 and c = 3` | ✅ | a, b, c |
| `b = 2` | ❌ | 无（跳过最左列） |
| `b = 2 and c = 3` | ❌ | 无 |
| `a = 1 and c = 3` | ⚠️ 部分 | a（c 用不上定位） |
| `a > 1 and b = 2` | ⚠️ 部分 | a（范围之后，b 失效） |
| `a = 1 and b > 2 and c = 3` | ⚠️ 部分 | a, b（c 失效） |

**两条规律：**

1. **必须从最左列开始**，中间不能断（断了，后面的列都只能当过滤条件）。
2. **遇到范围查询（>、<、between、like 'x%'）就停**，范围列后面的列不能用于定位。

> 补充：MySQL 5.6+ 有**索引下推（ICP）**，范围列后面的条件虽然不能定位，但可以在索引层先过滤（`Extra: Using index condition`），减少回表次数——但**不等于能用上**，排序/定位仍然用不了。

### 6.1 联合索引列顺序怎么定

| 原则 | 说明 |
| --- | --- |
| 等值列在前，范围列在后 | 范围一出现，后面的列就失去定位能力 |
| 区分度高的列在前 | 更快缩小范围（但也要结合查询频率） |
| 把 `order by` 的列接在后面 | 索引天然有序，可省掉 filesort |
| 尽量让高频查询被覆盖 | 覆盖索引比回表快一个量级 |

## 7. 动手实验（40 分钟）

```sql
CREATE DATABASE IF NOT EXISTS idx_lab2 DEFAULT CHARSET utf8mb4;
USE idx_lab2;
DROP TABLE IF EXISTS t_user;

CREATE TABLE t_user (
  id   BIGINT PRIMARY KEY AUTO_INCREMENT,
  name VARCHAR(50),
  age  INT,
  city VARCHAR(20),
  memo VARCHAR(100)
) ENGINE = InnoDB;

SET SESSION cte_max_recursion_depth = 10000;
INSERT INTO t_user (name, age, city, memo)
WITH RECURSIVE seq(n) AS (
  SELECT 1 UNION ALL SELECT n + 1 FROM seq WHERE n < 10000
)
SELECT CONCAT('user', n), 18 + n % 50, ELT(1 + n % 4, '北京','上海','广州','深圳'), 'memo'
FROM seq;

CREATE INDEX idx_name     ON t_user(name);
CREATE INDEX idx_age_city ON t_user(age, city);
```

逐条跑，把 `type / key / key_len / Extra` 填进表里：

| # | SQL | 关注点 |
| --- | --- | --- |
| 1 | `EXPLAIN SELECT * FROM t_user WHERE name='user500';` | 回表（Extra 无 Using index） |
| 2 | `EXPLAIN SELECT name FROM t_user WHERE name='user500';` | 覆盖（Using index） |
| 3 | `EXPLAIN SELECT id, name FROM t_user WHERE name='user500';` | 覆盖（id 在叶子里） |
| 4 | `EXPLAIN SELECT * FROM t_user WHERE age=20;` | 联合索引最左列 |
| 5 | `EXPLAIN SELECT * FROM t_user WHERE city='北京';` | 跳过最左列 → ALL |
| 6 | `EXPLAIN SELECT * FROM t_user WHERE age=20 AND city='北京';` | 两列都用上（key_len 变长） |
| 7 | `EXPLAIN SELECT * FROM t_user WHERE age>20 AND city='北京';` | range + 索引下推 |
| 8 | `EXPLAIN SELECT age, city FROM t_user WHERE age=20;` | 覆盖（联合索引叶子含 age/city/id，不用回表） |

**要记录的三件事**：
1. 第 2、3 条和第 1 条的 `Extra` 差在哪；
2. 第 5 条为什么 `key` 是 NULL；
3. 第 6 条和第 4 条的 `key_len` 差多少（说明两列都用上了）。

## 8. 巩固练习（45 分钟，做完再看答案）

### A. 选择题（8 题）

**1. InnoDB 聚簇索引的叶子节点存什么？**
A. 主键值　B. 整行数据　C. 行地址　D. 索引列

**2. 二级索引的叶子节点存什么？**
A. 整行数据　B. 索引列 + 主键值　C. 行地址　D. 页号

**3. 一张 InnoDB 表最多有几个聚簇索引？**
A. 0 个　B. 1 个　C. 2 个　D. 每个索引一个

**4. 下列哪条一定是覆盖索引？**（表有 `idx_name(name)`，主键 id）
A. `select * from t where name='x'`
B. `select name from t where name='x'`
C. `select age from t where name='x'`
D. `select * from t`

**5. 联合索引 `(a,b,c)`，`where b=2` 能走索引吗？**
A. 能，走全索引　B. 不能，跳过最左列　C. 只能用 c　D. 只能全表扫

**6. 联合索引 `(a,b,c)`，`where a=1 and c=3` 能用上几列定位？**
A. 0　B. 1　C. 2　D. 3

**7. 回表发生在什么之后？**
A. 聚簇索引查完　B. 二级索引查到主键之后　C. 全表扫描之后　D. 覆盖索引之后

**8. `Extra: Using index` 表示？**
A. 用了索引下推　B. 覆盖索引，不用回表　C. 用了主键　D. 索引失效

### B. 判断题（5 题）

1. 二级索引的叶子存的是行地址。（　）
2. 一张表可以有多个二级索引。（　）
3. 联合索引 `(a,b,c)` 查 `c=3` 也能用上索引。（　）
4. 覆盖索引可以避免回表。（　）
5. 主键越长，二级索引占的空间越大。（　）

### C. 简答题（3 题）

1. 什么是回表？有哪两种常见方式避免？
2. 最左前缀原则是什么？举一个「用不上」的例子。
3. 为什么建议用自增主键，而不是 UUID？

### D. SQL 分析题（4 题，表结构同上）

> 表 `t_user(id 主键, name, age, city, memo)`，索引：`idx_name(name)`、`idx_age_city(age, city)`。判断 type、是否回表、Extra 大概是什么。

1. `select * from t_user where name = 'user500';`
2. `select id, name from t_user where name = 'user500';`
3. `select * from t_user where city = '北京';`
4. `select * from t_user where age = 20 and city = '北京';`

<details><summary><b>答案与解析（先做完再点开）</b></summary>

**A. 选择题**

1. **B**。聚簇索引叶子存整行，所以主键查询一次到位。
2. **B**。二级索引叶子存「索引列 + 主键值」，要整行就回表。
3. **B**。数据只能按一种顺序组织，聚簇索引只有一个。
4. **B**。只查 name，二级索引叶子就有；A、C 要回表，D 是全表扫。
5. **B**。B+ 树先按 a 排，跳过 a 就无法二分定位。
6. **B**。只用 a 定位；c 不能定位（只能当过滤条件）。
7. **B**。二级索引拿到主键后，再查聚簇索引 = 回表。
8. **B**。`Using index` 是覆盖索引；`Using index condition` 才是索引下推。

**B. 判断题**

1. **错**。存主键值（行地址会失效）。
2. **对**。一张表可以有任意多个二级索引。
3. **错**。跳过最左列 a，索引无法定位。
4. **对**。查询列全在索引里，不用回表。
5. **对**。二级索引叶子每个条目都要带主键，主键越宽，索引越大。

**C. 简答题（要点）**

1. 二级索引叶子只有「索引列 + 主键」，查整行时要拿主键再查聚簇索引，这一次额外查找叫回表。避免：①覆盖索引（只查索引里的列）；②把高频查询列放进联合索引；③少用 `select *`。
2. 联合索引按「最左列 → 次左列」排序，条件必须从最左列开始且连续。例：索引 `(a,b,c)`，`where b=2` 或 `where c=3` 用不上。
3. 自增主键有序递增，插入总在 B+ 树最右侧，页分裂少、随机 IO 少；UUID 无序，插入位置随机，页分裂多、索引膨胀、二级索引也更大。

**D. SQL 分析题**

| # | type | 回表 | Extra（预期） | 说明 |
| --- | --- | --- | --- | --- |
| 1 | `ref` | ✅ 回表 | NULL | 走 idx_name 拿 id，再查聚簇索引取整行 |
| 2 | `ref` | ❌ 覆盖 | `Using index` | id + name 都在二级索引叶子 |
| 3 | `ALL` | 全表扫 | NULL | 跳过最左列 age，idx_age_city 用不上 |
| 4 | `ref` | ✅ 回表 | NULL | age、city 两列都用上，取整行仍要回表 |

</details>

## 9. 高频坑

| 坑 | 说明 | 正确理解 |
| --- | --- | --- |
| 以为 `select *` 也能覆盖 | `*` 包含所有列 | 只有索引里有的列才能覆盖 |
| 把 `Using index condition` 当成覆盖 | 它是索引下推 | 覆盖看 `Using index`（没有 condition） |
| 联合索引随便排顺序 | 顺序决定能用哪些查询 | 等值在前、范围在后、高频在前 |
| 以为范围条件后面的列「完全没用」 | ICP 仍可过滤 | 能过滤 ≠ 能定位/排序 |
| 用 UUID 当主键 | 随机插入 → 页分裂 | 自增、短、有序 |
| 在二级索引上 `order by` 非索引列 | 触发 filesort | 排序字段要进索引且守最左前缀 |

## 10. 30 秒复习卡

```text
聚簇索引 = 主键索引，叶子存整行，一张表 1 个
二级索引 = 叶子存「索引列 + 主键」，查整行要回表
回表 = 二级索引拿主键 → 再查聚簇索引（多一次随机 IO）
覆盖索引 = 查询列全在索引里 → Extra: Using index，不用回表
联合索引 (a,b,c) = 先 a 再 b 再 c；从最左开始、不能断、遇范围就停
```

## 11. 面试一句话

> **什么是回表？怎么优化？**
> InnoDB 的二级索引叶子只存索引列和主键值，查询需要其他列时，要拿主键再去聚簇索引查一次整行，这就是回表。优化方式：用覆盖索引（只查索引里有的列）、把常用查询列做进联合索引、避免 `select *`。
>
> **最左前缀原则？**
> 联合索引按定义时的列顺序排序，查询条件必须从最左列开始并且连续，才能用索引定位；中间跳过一列，后面的列就只能做过滤，不能用于定位和排序。
