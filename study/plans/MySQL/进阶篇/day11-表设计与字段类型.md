# 表设计与字段类型（进阶篇 · Day 11 / W2 D4）

> 一句话：**范式管"不冗余、不改错"，反范式管"查得快"，字段类型管"省空间、索引短"。三件事的优先级是：先正确，再性能，最后省字节。**
> 排期：11/06｜今天不学新原理，是把 Day1–Day10 的知识**用在建表上**——这也是 M1 验收和后面苍穹外卖/黑马点评项目的地基。

## 0. 今天 3 小时怎么用

| 时间 | 做什么 | 产出 |
| --- | --- | --- |
| 15 min | 看 Day10 批改（尤其 D1 那处概念错误） | 分清"快照读 / 当前读" |
| 45 min | §2–§4 三大范式、反范式、字段类型 | 记住范式的三个反例 |
| 60 min | §5 建表实战（订单/券/用户）+ §6 验证实验 | 6 张表的 DDL 能跑通 |
| 25 min | 算法 | 哈希表 1 题 |
| 20 min | 记录 + 检查清单逐条打勾 | 今天结束 |
| 15 min | 填练习 | —— |

## 1. 先看 Day10 那道错题：当前读不看快照

D1 你答「读到 `v0`」，正确是 **`v1`**。这一类错误你已经犯第二次了（Day9 D1 也是把两种状态混在一起），所以给你一个**三句话判别法**，以后遇到"读的是谁"就套它：

```text
① 语句是普通 SELECT 吗？        → 是：快照读，按 ReadView 看历史版本
② 语句带 FOR UPDATE / 是写操作吗？→ 是：当前读，读最新已提交版本，并加锁
③ 我自己改过的行，我自己看得见吗？→ 看得见（规则① creator_trx_id）
```

所以那道题：`UPDATE` 是**当前读** → 读到 B 已提交的 **`v1`** → 把它改成 `v2`。跟你事务里的 ReadView 没关系。**记住：ReadView 只约束"普通 SELECT"，管不到 UPDATE。**

## 2. 三大范式：它们各自在防什么错

范式的目的是**消除冗余，从而消除"改一处漏一处"的更新异常**。不用背定义，记反例。

### 2.1 第一范式（1NF）：列不可再分

```text
❌ 反例：user 表里 address = '广东省深圳市南山区科技园'，或 tags = 'Java,MySQL,Redis'
   问题：想按城市统计 → 只能 LIKE，走不了索引
✅ 改法一：拆成 province / city / district 三列
✅ 改法二：tags 拆成独立表 user_tag(user_id, tag)，一对多
```

**判断口诀**：这一列里还能不能"再切一刀"？能切，就不满足 1NF。

### 2.2 第二范式（2NF）：非主键列要依赖"整个"主键

只对**联合主键**有意义——消除"部分依赖"。

```text
❌ 反例：order_item(order_id, product_id, product_name, price, num)
   主键 = (order_id, product_id)
   但 product_name、price 只依赖 product_id，不依赖 order_id
   问题：同一个商品在 100 个订单里存了 100 份名字，改名要改 100 处
✅ 改法：拆出 product 表，order_item 只留 (order_id, product_id, price, num)
```

> 注意：`price` 留在 order_item 里**不算违反**——订单要冻结下单时的价格，这属于"业务快照"，见 §3。

### 2.3 第三范式（3NF）：非主键列不能依赖另一个非主键列

消除"传递依赖"。

```text
❌ 反例：student(id, name, class_id, class_name, teacher_name)
   class_name 依赖 class_id，class_id 又依赖 id → 传递依赖
   问题：班级改名要改所有学生行
✅ 改法：class 表单独存 (id, name, teacher_name)，student 只留 class_id
```

### 2.4 BCNF（了解即可）

3NF 的加强版：连"主属性之间的依赖"也要消掉。实际业务里**做到 3NF 就够**，面试被问到答"BCNF 进一步消除了主属性对候选键的部分/传递依赖，一般业务 3NF 已足够"就行。

| 范式 | 消除什么 | 典型反例 |
| --- | --- | --- |
| 1NF | 列不可再分 | 逗号分隔的 tags、拼在一起的地址 |
| 2NF | 部分依赖（联合主键） | 订单明细里存商品名 |
| 3NF | 传递依赖 | 学生表里存班级名 |

## 3. 反范式：什么时候"故意冗余"

范式让表变多、查询要 JOIN。**读多写少**的场景可以故意冗余，但要有明确理由：

| 场景 | 怎么冗余 | 为什么可以 |
| --- | --- | --- |
| 列表页要显示用户名 | 订单表存 `user_name` | 用户名几乎不改；省一次 JOIN |
| 计数展示 | 帖子表存 `liked`、`comments` 计数 | 每次 `COUNT(*)` 太贵；允许短暂不准 |
| 报表/数仓 | 宽表，几十列 | 分析场景不关心写放大 |
| **业务快照** | 订单存下单时的 `price`、`voucher_title` | **这不是反范式，是业务要求**：历史价格必须冻结 |

判断标准三问：

1. 这个冗余字段**多久变一次**？（几乎不变才冗余）
2. 查询**真的**因此变快了吗？（有 JOIN 索引时未必要冗余）
3. 一致性延迟**业务能接受**吗？（计数类可以，金额类不行）

> 常见误区：把"商品快照"当成反范式来辩护。快照的本质是**记录历史事实**，和范式无关——哪怕完全规范化，订单也必须存下单价。

## 4. 字段类型：一张表选完

### 4.1 整数

| 类型 | 字节 | 范围（有符号） | 用途 |
| --- | --- | --- | --- |
| TINYINT | 1 | -128 ~ 127 | 状态、性别、布尔 |
| SMALLINT | 2 | ±3.2 万 | 小计数 |
| MEDIUMINT | 3 | ±838 万 | 少用 |
| INT | 4 | ±21 亿 | 常规 ID、数量 |
| BIGINT | 8 | ±9.2×10¹⁸ | 主键、时间戳、金额（分） |

- 8.0 起 `INT(11)` 的**显示宽度已废弃**（不再补零），别再写 `INT(11)`。
- 用 `UNSIGNED` 前想清楚：**两个 UNSIGNED 相减可能报错/回绕**，很多团队干脆全用有符号。

### 4.2 金额

```sql
-- ❌ 绝对不要
price FLOAT / DOUBLE          -- 二进制浮点，0.1+0.2 ≠ 0.3
-- ✅ 二选一
price DECIMAL(10,2)           -- 精确十进制，范围 ±9999 万，2 位小数
price BIGINT                  -- 存"分"，程序里除 100
```

### 4.3 字符串

| 类型 | 特点 | 用途 |
| --- | --- | --- |
| CHAR(n) | 定长，不足补空格，n ≤ 255 | 长度固定的码：手机号、身份证、MD5 |
| VARCHAR(n) | 变长，额外 1~2 字节存长度 | 绝大多数文本 |
| TEXT | 行外存储，不能整列建索引 | 长正文（正文建议拆到副表） |
| BLOB | 二进制 | 尽量别放数据库（用对象存储） |

三个必须知道的细节：

1. **`VARCHAR(n)` 的 n 是"字符数"不是字节数**。utf8mb4 下一个字符最多 4 字节，所以 `VARCHAR(32)` 的索引 `key_len` 最大 = 32×4 + 2 = **130**（这正好接上 Day4 的 key_len 公式）。
2. **CHAR 不一定省空间**：utf8mb4 下 CHAR(10) 也按最多 40 字节预留，存不满会浪费。
3. **排序规则**：8.0 默认 `utf8mb4_0900_ai_ci`（不区分大小写、不区分重音）。`ci` = case insensitive，做验证码/密码比对要小心。

### 4.4 时间

| 类型 | 字节 | 范围 | 说明 |
| --- | --- | --- | --- |
| DATE | 3 | 1000-01-01 ~ 9999-12-31 | 只到天 |
| DATETIME | 5（带微秒时 6~8） | 1000 ~ 9999 | **推荐**，不随时区变 |
| TIMESTAMP | 4 | 1970-01-01 ~ **2038-01-19** | 有时区转换，**有 2038 问题** |

结论：**业务时间用 `DATETIME`**（或 `BIGINT` 存毫秒），别用 `TIMESTAMP`，更别用 `VARCHAR` 存时间（排序、范围查询全废）。

### 4.5 状态与枚举

```sql
-- ❌ ENUM：加一个值要 ALTER TABLE，且顺序敏感
status ENUM('待支付','已支付','已取消')
-- ✅
status TINYINT NOT NULL DEFAULT 0 COMMENT '0待支付 1已支付 2已取消 3已退款'
```

### 4.6 NULL 与默认值

```sql
-- ❌ 能 NULL 的列：COUNT(col) 会漏、!= 会漏、索引统计更差
nickname VARCHAR(32)
-- ✅
nickname VARCHAR(32) NOT NULL DEFAULT '' COMMENT '昵称'
```

> 例外：真正"未知"的语义（比如 `pay_time` 未支付时）可以用 NULL，但要接受查询时的三值逻辑。

## 5. 建表实战：一套"券"业务（和苍穹外卖/黑马点评同构）

设计目标：用户 → 券（秒杀券）→ 订单 → 评价。先看结构，再逐条解释取舍。

```sql
CREATE DATABASE IF NOT EXISTS shop DEFAULT CHARSET utf8mb4 COLLATE utf8mb4_0900_ai_ci;
USE shop;

-- ① 用户表
CREATE TABLE t_user (
  id          BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键',
  phone       CHAR(11)        NOT NULL                COMMENT '手机号，定长用 CHAR',
  password    CHAR(64)        NOT NULL                COMMENT 'SHA-256 十六进制，64 位定长',
  nickname    VARCHAR(32)     NOT NULL DEFAULT ''     COMMENT '昵称，最长 32 字符',
  avatar      VARCHAR(255)    NOT NULL DEFAULT ''     COMMENT '头像 URL',
  status      TINYINT         NOT NULL DEFAULT 1      COMMENT '1正常 0禁用',
  create_time DATETIME        NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  update_time DATETIME        NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (id),
  UNIQUE KEY uk_phone (phone)                         COMMENT '登录靠手机号，必须唯一'
) ENGINE = InnoDB COMMENT = '用户表';
```

```sql
-- ② 店铺表（经纬度用 DECIMAL，别用 FLOAT）
CREATE TABLE t_shop (
  id          BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  name        VARCHAR(64)     NOT NULL COMMENT '店名',
  type_id     BIGINT UNSIGNED NOT NULL COMMENT '分类ID',
  address     VARCHAR(128)    NOT NULL DEFAULT '' COMMENT '地址（整体展示用，不拆）',
  x           DECIMAL(10,6)   NOT NULL DEFAULT 0  COMMENT '经度，6 位小数≈0.1m',
  y           DECIMAL(10,6)   NOT NULL DEFAULT 0  COMMENT '纬度',
  avg_price   DECIMAL(8,2)    NOT NULL DEFAULT 0  COMMENT '人均，2 位小数',
  score       DECIMAL(2,1)    NOT NULL DEFAULT 5.0 COMMENT '评分 0.0~9.9',
  status      TINYINT         NOT NULL DEFAULT 1  COMMENT '1营业 0停业',
  create_time DATETIME        NOT NULL DEFAULT CURRENT_TIMESTAMP,
  update_time DATETIME        NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  KEY idx_type (type_id) COMMENT '按分类列表页查询'
) ENGINE = InnoDB COMMENT = '店铺表';
```

```sql
-- ③ 优惠券（注意：秒杀库存单独拆表 → 一对一，把"高频写"隔离出去）
CREATE TABLE t_voucher (
  id           BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  shop_id      BIGINT UNSIGNED NOT NULL COMMENT '所属店铺',
  title        VARCHAR(64)     NOT NULL COMMENT '券标题',
  sub_title    VARCHAR(128)    NOT NULL DEFAULT '' COMMENT '副标题',
  rules        VARCHAR(1024)   NOT NULL DEFAULT '' COMMENT '使用规则（短文本，够用就用 VARCHAR 不用 TEXT）',
  pay_value    DECIMAL(10,2)   NOT NULL DEFAULT 0 COMMENT '支付金额',
  actual_value DECIMAL(10,2)   NOT NULL DEFAULT 0 COMMENT '抵扣金额',
  type         TINYINT         NOT NULL DEFAULT 0 COMMENT '0普通 1秒杀',
  stock        INT             NOT NULL DEFAULT 0 COMMENT '库存（普通券）',
  begin_time   DATETIME        NOT NULL COMMENT '生效时间',
  end_time     DATETIME        NOT NULL COMMENT '失效时间',
  status       TINYINT         NOT NULL DEFAULT 1 COMMENT '1上架 0下架',
  create_time  DATETIME        NOT NULL DEFAULT CURRENT_TIMESTAMP,
  update_time  DATETIME        NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  KEY idx_shop (shop_id),
  KEY idx_status_time (status, begin_time) COMMENT '查"上架且已开始"的券，联合索引顺序有讲究'
) ENGINE = InnoDB COMMENT = '优惠券表';

CREATE TABLE t_seckill_voucher (
  voucher_id BIGINT UNSIGNED NOT NULL COMMENT '主键=券ID，一对一',
  stock      INT             NOT NULL DEFAULT 0 COMMENT '秒杀库存（会被高频扣减）',
  begin_time DATETIME        NOT NULL,
  end_time   DATETIME        NOT NULL,
  PRIMARY KEY (voucher_id)
) ENGINE = InnoDB COMMENT = '秒杀券库存表';
```

```sql
-- ④ 订单表：状态机 + 多个时间字段 + 业务快照
CREATE TABLE t_voucher_order (
  id            BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '订单ID（也可用雪花）',
  voucher_id    BIGINT UNSIGNED NOT NULL COMMENT '券ID',
  user_id       BIGINT UNSIGNED NOT NULL COMMENT '下单用户',
  voucher_title VARCHAR(64)     NOT NULL COMMENT '券标题快照（券改名不影响历史订单）',
  pay_value     DECIMAL(10,2)   NOT NULL COMMENT '支付金额快照',
  pay_type      TINYINT         NOT NULL DEFAULT 0 COMMENT '0余额 1支付宝 2微信',
  status        TINYINT         NOT NULL DEFAULT 0 COMMENT '0未支付 1已支付 2已核销 3已取消 4已退款',
  create_time   DATETIME        NOT NULL DEFAULT CURRENT_TIMESTAMP,
  pay_time      DATETIME        DEFAULT NULL COMMENT '未支付时为 NULL（语义上就是"还没发生"）',
  use_time      DATETIME        DEFAULT NULL COMMENT '核销时间',
  refund_time   DATETIME        DEFAULT NULL,
  update_time   DATETIME        NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  UNIQUE KEY uk_user_voucher (user_id, voucher_id) COMMENT '同一人同一券限购 1 张（业务约束落在唯一索引上）',
  KEY idx_user_time (user_id, create_time) COMMENT '我的订单列表：按用户+时间倒序',
  KEY idx_status (status) COMMENT '后台按状态捞单'
) ENGINE = InnoDB COMMENT = '券订单表';
```

```sql
-- ⑤ 评价表：演示 1NF 的"多值拆分"与反范式的"计数冗余"
CREATE TABLE t_blog (
  id          BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  shop_id     BIGINT UNSIGNED NOT NULL,
  user_id     BIGINT UNSIGNED NOT NULL,
  title       VARCHAR(128)    NOT NULL DEFAULT '',
  content     TEXT            NOT NULL COMMENT '正文很长，单独放 TEXT（不在主表查询条件里用）',
  liked       INT             NOT NULL DEFAULT 0 COMMENT '点赞数：反范式冗余计数',
  comments    INT             NOT NULL DEFAULT 0 COMMENT '评论数：同上',
  create_time DATETIME        NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  KEY idx_shop_time (shop_id, create_time)
) ENGINE = InnoDB COMMENT = '探店评价';

CREATE TABLE t_blog_tag (
  blog_id BIGINT UNSIGNED NOT NULL,
  tag     VARCHAR(16)     NOT NULL COMMENT '标签拆表，替代 tags="美食,环境好"',
  PRIMARY KEY (blog_id, tag),
  KEY idx_tag (tag) COMMENT '按标签反查评价（1NF 拆表后才建得出来）'
) ENGINE = InnoDB COMMENT = '评价标签';
```

### 5.1 设计说明：每一处取舍都要能说出理由

| 设计点 | 选择 | 理由 |
| --- | --- | --- |
| 主键 | `BIGINT UNSIGNED AUTO_INCREMENT` | 8 字节、有序（避免 Day5 讲的页分裂）、不暴露业务量 |
| 手机号 | `CHAR(11)` + 唯一索引 | 定长，CHAR 更省；唯一约束由数据库保证 |
| 密码 | `CHAR(64)` | SHA-256 hex 固定 64 字符，定长最合适 |
| 金额 | `DECIMAL(10,2)` | 精确；浮点会丢精度 |
| 经纬度 | `DECIMAL(10,6)` | 6 位小数约 0.1 米，够用且精确 |
| 时间 | `DATETIME` | 不受 2038 限制、不做时区转换 |
| 状态 | `TINYINT` + 注释 | 扩展只改注释，不用 ALTER |
| 秒杀库存 | 拆 `t_seckill_voucher` | 高频写隔离到小表，行锁冲突面更小 |
| 订单存券名 | 快照字段 | 历史订单必须显示"当时买的什么价" |
| 限购 | 唯一索引 `(user_id, voucher_id)` | 幂等/防重复下单的最后一道闸 |
| 正文 | `TEXT` | 不用它做条件，避免行溢出影响主表 |
| 标签 | 拆 `t_blog_tag` | 1NF：多值列拆表才能建索引、才能反查 |
| 计数 | 冗余 `liked/comments` | 列表页每次 COUNT 太贵，允许短暂不准 |
| 外键 | **不建** | 互联网场景常用应用层保证：外键会加锁、影响分库分表与在线 DDL |

### 5.2 建表检查清单（每次建完逐条打勾）

```text
□ 有主键吗？主键是自增 BIGINT / 雪花，而不是业务字段或 UUID？
□ 主键之外的唯一约束（手机号、限购）用 UNIQUE KEY 落到数据库了吗？
□ 所有列都 NOT NULL + DEFAULT 了吗？（除"未发生"语义的时间列）
□ 金额是 DECIMAL 吗？时间是 DATETIME 吗？状态是 TINYINT 吗？
□ 每张表、每个字段都有 COMMENT 吗？
□ 索引命名规范：idx_ 普通索引、uk_ 唯一索引？
□ 字符集是 utf8mb4 吗？排序规则统一吗？
□ 有做"列表页查询"的联合索引吗？顺序符合最左前缀？
□ 长文本/大字段拆出去了吗？
□ 是否有冗余字段？每个冗余都能说出"为什么允许"吗？
```

## 6. 实验（60 分钟）

```sql
-- ---------- 实验 A：把 §5 的 6 张表建出来 ----------
-- 依次执行上面的 CREATE TABLE，然后确认
SHOW TABLES;
SHOW CREATE TABLE t_voucher_order\G    -- 看引擎、字符集、索引、注释是否都在
```

```sql
-- ---------- 实验 B：DECIMAL vs DOUBLE 的精度 ----------
DROP TABLE IF EXISTS t_num;
CREATE TABLE t_num (d DECIMAL(10,2), f DOUBLE);
INSERT INTO t_num VALUES (0.1, 0.1), (0.2, 0.2);

SELECT SUM(d) AS decimal_sum, SUM(f) AS double_sum FROM t_num;
-- 预期：0.30  vs  0.30000000000000004

SELECT CAST(0.1 AS DOUBLE) + CAST(0.2 AS DOUBLE) AS float_add;
-- 结论：金额用 DOUBLE，对账时会出现"差 1 分"
```

```sql
-- ---------- 实验 C：VARCHAR(n) 的 n 是字符，不是字节 ----------
INSERT INTO t_user (phone, password, nickname) VALUES ('13800000001', REPEAT('a', 64), '😀');
SELECT nickname,
       LENGTH(nickname)      AS bytes,    -- 4（utf8mb4 一个 emoji 4 字节）
       CHAR_LENGTH(nickname) AS chars     -- 1
FROM t_user WHERE phone = '13800000001';
-- 结论：VARCHAR(32) 能存 32 个 emoji（128 字节），所以索引 key_len 要按 ×4 算
```

```sql
-- ---------- 实验 D：验证索引长度与 CHAR/VARCHAR 差异 ----------
EXPLAIN SELECT * FROM t_user WHERE phone = '13800000001';
-- key: uk_phone, key_len: 44 → CHAR(11) utf8mb4 = 11×4 = 44
-- 对照：如果 phone 是 VARCHAR(11) 且 NOT NULL，key_len = 11×4+2 = 46（多 2 字节存长度）
```

```sql
-- ---------- 实验 E：看列的元数据（写文档/交接时很有用）----------
SELECT COLUMN_NAME, COLUMN_TYPE, IS_NULLABLE, COLUMN_DEFAULT, CHARACTER_MAXIMUM_LENGTH, COLUMN_COMMENT
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = 'shop' AND TABLE_NAME = 't_voucher_order'
ORDER BY ORDINAL_POSITION;
```

```sql
-- ---------- 实验 F：插入一条完整订单数据，验证约束 ----------
INSERT INTO t_voucher_order (voucher_id, user_id, voucher_title, pay_value, status)
VALUES (1, 1, '满 100 减 20', 80.00, 0);

-- 再插一条同用户同券 → 应报 1062 Duplicate entry
INSERT INTO t_voucher_order (voucher_id, user_id, voucher_title, pay_value, status)
VALUES (1, 1, '满 100 减 20', 80.00, 0);
```

**记录表（今天要填的 4 行）**

| 实验 | 看到的结果 | 结论 |
| --- | --- | --- |
| B DECIMAL vs DOUBLE | | |
| C emoji 字节数 / 字符数 | | |
| D key_len（CHAR 11） | | |
| F 重复插入的报错码 | | |

## 7. 练习

### A. 判断题（6）

**1.** 只要表满足第三范式，就不需要再考虑冗余字段了。
> 答：

**2.** `VARCHAR(20)` 表示最多存 20 个字节。
> 答：

**3.** 金额字段用 `DECIMAL(10,2)` 比 `DOUBLE` 更安全。
> 答：

**4.** `TIMESTAMP` 比 `DATETIME` 更省空间，所以业务时间应该优先用 `TIMESTAMP`。
> 答：

**5.** 订单表里存 `voucher_title`（券标题）违反了第三范式，应该删掉。
> 答：

**6.** 把 `tags = '美食,环境好'` 存在一列里，是符合第一范式的。
> 答：

### B. 选择题（4）

**1. 联合主键 `(order_id, product_id)` 的表里存了 `product_name`，违反了？**
A. 1NF　B. 2NF　C. 3NF　D. 不违反
> 答：

**2. 学生表有 `(id, name, class_id, class_name)`，`class_name` 的问题属于？**
A. 列可再分　B. 部分依赖　C. 传递依赖　D. 没问题
> 答：

**3. 存"用户余额"，最合适的类型是？**
A. `FLOAT`　B. `DOUBLE`　C. `DECIMAL(12,2)`　D. `VARCHAR(20)`
> 答：

**4. 下面哪个做法能避免"同一用户对同一张券重复下单"？**
A. 在代码里 `SELECT` 一次判断
B. 建 `UNIQUE KEY (user_id, voucher_id)`
C. 给 `user_id` 建普通索引
D. 用 `TEXT` 存订单信息
> 答：

### C. 简答题（3）

**1. 三大范式分别消除什么问题？各给一个反例。**
> 答：
>
>
>

**2. 什么情况下可以反范式？说出你的三条判断标准。**
> 答：
>
>
>

**3. 为什么互联网业务通常不建外键？**
> 答：
>
>

### D. 设计题（2）

**1.** 设计一张"点赞记录表"，要求：能记录"谁在什么时候赞了哪条评价"，同一个人对同一条评价只能赞一次，并且能快速查出"某条评价的所有点赞"和"某个用户赞过的所有评价"。给出 DDL 并说明索引。
> 答：
>
>

**2.** 有一张 `t_article(id, title, content, author_name, author_avatar, category_name, tags)`，其中 `tags='Java,MySQL'`、`author_avatar` 只由作者决定、`category_name` 只由分类决定。指出它违反了哪些范式，给出改造方案（写出表结构即可）。
> 答：
>
>

<details><summary><b>参考答案</b></summary>

**A. 判断题**

1. ❌ 错。范式解决冗余，但**读多写少**场景可以故意冗余（用户名、计数、业务快照），关键是能说出理由。
2. ❌ 错。`VARCHAR(n)` 的 n 是**字符数**；utf8mb4 下最多占 4n 字节。
3. ✅ 对。DECIMAL 是精确十进制，DOUBLE 是二进制浮点，会出现 `0.1+0.2≠0.3`。
4. ❌ 错。TIMESTAMP 只有 4 字节但**2038 年溢出**，且做时区转换；业务时间优先 `DATETIME`。
5. ❌ 错。存券标题是**业务快照**（历史订单要显示当时的券名与价格），不属于范式问题；券改名不能影响历史订单。
6. ❌ 错。逗号分隔的多值列违反 1NF（列不可再分），无法建索引、无法按标签查询。

**B. 选择题**

1. **B**。`product_name` 只依赖 `product_id`，属于联合主键的**部分依赖** → 2NF。
2. **C**。`class_name` 依赖 `class_id`，`class_id` 依赖 `id` → **传递依赖** → 3NF。
3. **C**。金额必须精确十进制；先排除浮点，VARCHAR 无法做数值运算与排序。
4. **B**。唯一索引把"业务约束"落到数据库，能天然防并发重复；代码判断有竞态。

**C. 简答题（要点）**

1. **1NF** 消除"列可再分"——反例：`tags='Java,MySQL'`、`address='广东省深圳市南山区'`；**2NF** 消除"部分依赖"（只对联合主键）——反例：`order_item(order_id, product_id, product_name)`；**3NF** 消除"传递依赖"——反例：`student(id, name, class_id, class_name)`。
2. ①冗余字段**几乎不变**（用户名、券标题快照）；②查询确实因此变快，且无法靠索引/JOIN 优化掉；③**一致性延迟业务能接受**（点赞数、评论数可以短暂不准，金额不行）。
3. ①外键在写入时会**加锁检查**，高并发下增加锁冲突；②影响**分库分表**（跨库无法维护外键）；③影响**在线 DDL**与数据迁移；④互联网业务更倾向"应用层保证 + 唯一索引兜底 + 定期对账"。

**D. 设计题**

1. 参考 DDL：

```sql
CREATE TABLE t_like (
  id          BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  blog_id     BIGINT UNSIGNED NOT NULL COMMENT '被赞的评价',
  user_id     BIGINT UNSIGNED NOT NULL COMMENT '点赞用户',
  create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  UNIQUE KEY uk_blog_user (blog_id, user_id) COMMENT '防重复点赞（幂等）',
  KEY idx_user_time (user_id, create_time)   COMMENT '查某用户赞过的（我的点赞列表）'
) ENGINE = InnoDB COMMENT = '点赞记录';
```

说明：`uk_blog_user` 天然满足"同人同评价只赞一次"，且以 `blog_id` 打头，能高效回答"某条评价的所有点赞"；`idx_user_time` 回答"某个用户赞过的所有评价"，且按时间倒序可直接走索引（避免额外排序）。

2. 违反情况：
- `tags` 逗号分隔 → **违反 1NF**（列可再分）；
- `author_avatar` 只依赖作者 → 与主键外的字段存在依赖，若作者信息在多篇文章重复 → **违反 3NF**（应拆 `t_author`）；
- `category_name` 只由分类决定 → **违反 3NF**（应拆 `t_category`）。

改造后：

```sql
t_article(id PK, title, content, author_id, category_id, create_time)
t_author(id PK, name, avatar)
t_category(id PK, name)
t_article_tag(article_id, tag, PRIMARY KEY(article_id, tag), KEY idx_tag(tag))
```

说明：`content` 可以留 TEXT 在主表（若不用于条件查询），否则拆 `t_article_content(article_id PK, content)`；`t_article_tag` 既满足 1NF 又能"按标签反查文章"。

</details>

## 8. 高频坑

| 坑 | 说明 |
| --- | --- |
| 用 FLOAT/DOUBLE 存金额 | 对账差 1 分；用 DECIMAL 或 BIGINT 存分 |
| `VARCHAR(255)` 一律拉满 | 索引变长、内存分配浪费；按真实长度定，如 32/64/128 |
| 用 `TIMESTAMP` 存业务时间 | 2038 溢出 + 时区转换，跨时区业务易错 |
| 状态用 ENUM 或字符串 | 加值要 ALTER；用 TINYINT + COMMENT |
| 列能 NULL 就 NULL | 索引统计差、`COUNT(col)` 漏行、三值逻辑坑 |
| 主键用 UUID | 随机插入导致页分裂（Day5 讲过），B+ 树碎片化 |
| 什么都建外键 | 高并发锁冲突、妨碍分库分表与在线 DDL |
| 正文 TEXT 放主表还拿来查 | 行溢出、不能整列索引、拖慢主表扫描 |
| 忘记建"列表页"联合索引 | 建表时就要想好 Top3 查询，按最左前缀建索引 |

## 9. 30 秒复习卡

```text
1NF 列不可再分（tags 拆表）；2NF 消除部分依赖（联合主键）；3NF 消除传递依赖（拆字典表）
反范式三问：变得频繁吗 / 真的更快吗 / 允许延迟吗
金额 DECIMAL 或 BIGINT 分；时间 DATETIME；状态 TINYINT；主键 BIGINT 自增
VARCHAR(n)：n 是字符数，utf8mb4 索引 key_len ≈ 4n+2
业务快照（订单存券名/价格）不是反范式，是历史事实
互联网不建外键：锁冲突、分库分表、在线 DDL
```

## 10. 面试一句话

> **你们建表有什么规范？**
> 主键用 BIGINT 自增或雪花、不用业务字段；所有列 NOT NULL 加默认值，金额用 DECIMAL、时间用 DATETIME、状态用 TINYINT 并写注释；唯一约束（手机号、限购）落到 UNIQUE KEY 上做幂等兜底；列表页查询在建模阶段就设计好联合索引；长文本拆到副表；一般不建外键，由应用层 + 唯一索引 + 对账保证。
>
> **什么时候会反范式？**
> 读多写少、冗余字段几乎不变、且业务能接受短暂不一致时——比如订单里存券标题和价格的快照、列表页冗余用户名、评价表冗余点赞数。但金额类一致性不能放松，而且快照字段本质是记录历史事实，不属于反范式。
