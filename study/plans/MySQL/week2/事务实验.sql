-- ============================================================
-- MySQL 事务实验脚本
-- 配套笔记：study/plans/MySQL/week1/基础篇-事务.md
-- 环境：MySQL 8.0.16+（CHECK 约束才会真正生效）
-- 用法：开两个客户端（窗口 A / 窗口 B），按注释顺序交替执行
-- 注意：带【窗口 A】【窗口 B】的段落必须分两个连接跑，同一个连接看不出并发问题
-- ============================================================


-- ------------------------------------------------------------
-- 00 准备：建库、建表、造数据（只跑一次）
-- ------------------------------------------------------------
CREATE DATABASE IF NOT EXISTS tx_lab DEFAULT CHARSET utf8mb4;
USE tx_lab;

DROP TABLE IF EXISTS account;
CREATE TABLE account (
  id      INT PRIMARY KEY,
  name    VARCHAR(20)    NOT NULL,
  balance DECIMAL(10,2)  NOT NULL CHECK (balance >= 0)
) ENGINE = InnoDB;

INSERT INTO account (id, name, balance) VALUES
  (1, 'A', 1000),
  (2, 'B', 1000);

-- 初始状态：总额应为 2000
SELECT * FROM account;
SELECT SUM(balance) AS total FROM account;

-- 每个实验前都可以用这两行把数据重置回初始状态
-- UPDATE account SET balance = 1000;
-- DELETE FROM account WHERE id > 2;


-- ------------------------------------------------------------
-- 01 提交与回滚：转账两步
-- ------------------------------------------------------------
-- 【窗口 A】正常提交
START TRANSACTION;
UPDATE account SET balance = balance - 100 WHERE id = 1;
UPDATE account SET balance = balance + 100 WHERE id = 2;
-- 此时窗口 B 查询（默认 RR）看不到变化
COMMIT;
SELECT * FROM account;   -- A=900, B=1100

-- 【窗口 A】主动回滚
START TRANSACTION;
UPDATE account SET balance = balance - 100 WHERE id = 1;
ROLLBACK;
SELECT * FROM account;   -- A 仍是 900，钱没动

-- 【窗口 A】约束兜底：余额不能为负（会报错 3819）
START TRANSACTION;
UPDATE account SET balance = balance - 99999 WHERE id = 1;
ROLLBACK;                -- 报错后别忘了回滚，否则事务一直挂着

-- 查看当前隔离级别（MySQL 8；5.7 用 @@tx_isolation）
SELECT @@transaction_isolation;


-- ------------------------------------------------------------
-- 02 脏读：READ UNCOMMITTED
-- ------------------------------------------------------------
-- 【窗口 A】【窗口 B】都要先执行：
-- SET SESSION TRANSACTION ISOLATION LEVEL READ UNCOMMITTED;

-- 【窗口 A】
START TRANSACTION;
UPDATE account SET balance = 900 WHERE id = 1;
-- 不要提交！切到窗口 B

-- 【窗口 B】
SELECT balance FROM account WHERE id = 1;   -- 看到 900 → 脏读

-- 【窗口 A】
ROLLBACK;
-- 【窗口 B】
SELECT balance FROM account WHERE id = 1;   -- 回到 1000


-- ------------------------------------------------------------
-- 03 不可重复读：READ COMMITTED
-- ------------------------------------------------------------
-- 重置：UPDATE account SET balance = 1000;
-- 【窗口 A】【窗口 B】：
-- SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED;

-- 【窗口 A】
START TRANSACTION;
SELECT balance FROM account WHERE id = 1;   -- 1000

-- 【窗口 B】
UPDATE account SET balance = 900 WHERE id = 1;
COMMIT;

-- 【窗口 A】
SELECT balance FROM account WHERE id = 1;   -- 900 → 不可重复读
COMMIT;


-- ------------------------------------------------------------
-- 04 可重复读：REPEATABLE READ（MySQL 默认）
-- ------------------------------------------------------------
-- 重置：UPDATE account SET balance = 1000;
-- 【窗口 A】【窗口 B】：
-- SET SESSION TRANSACTION ISOLATION LEVEL REPEATABLE READ;

-- 【窗口 A】
START TRANSACTION;
SELECT balance FROM account WHERE id = 1;   -- 1000

-- 【窗口 B】
UPDATE account SET balance = 900 WHERE id = 1;
COMMIT;

-- 【窗口 A】
SELECT balance FROM account WHERE id = 1;   -- 仍是 1000（快照读）
COMMIT;
SELECT balance FROM account WHERE id = 1;   -- 新事务才看到 900


-- ------------------------------------------------------------
-- 05 幻读：REPEATABLE READ 下的快照读 vs 当前读
-- ------------------------------------------------------------
-- 重置：DELETE FROM account WHERE id > 2;
-- 【窗口 A】【窗口 B】：
-- SET SESSION TRANSACTION ISOLATION LEVEL REPEATABLE READ;

-- 【窗口 A】
START TRANSACTION;
SELECT COUNT(*) FROM account WHERE id > 10;   -- 0

-- 【窗口 B】
INSERT INTO account (id, name, balance) VALUES (11, 'C', 500);
COMMIT;

-- 【窗口 A】快照读：看不到新插入的行
SELECT COUNT(*) FROM account WHERE id > 10;   -- 仍是 0
-- 【窗口 A】当前读：能看到并锁住这条新行
UPDATE account SET balance = balance + 0 WHERE id > 10;   -- 影响 1 行
SELECT COUNT(*) FROM account WHERE id > 10;   -- 1 行
COMMIT;


-- ------------------------------------------------------------
-- 06 收尾（可选）
-- ------------------------------------------------------------
-- DROP DATABASE IF EXISTS tx_lab;
