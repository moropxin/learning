# MySQL 基础篇自检清单



***

## 0. SQL 四大分类



| 分类  | 全称     | 作用        | 代表关键字                                          |
| --- | ------ | --------- | ---------------------------------------------- |
| DDL | 数据定义语言 | 建库、建表、改结构 | `create / alter / drop / show / use`           |
| DML | 数据操作语言 | 增删改表里的数据  | `insert / update / delete`                     |
| DQL | 数据查询语言 | 查数据       | `select / where / group by / order by / limit` |
| DCL | 数据控制语言 | 权限、用户     | `grant / revoke / create user`                 |


***

## 1. DDL —— 库操作



```sql
SHOW DATABASES;                       -- 看所有库
CREATE DATABASE IF NOT EXISTS dbname  -- 建库（不存在才建）
  DEFAULT CHARSET utf8mb4;
USE dbname;                            -- 切库
DROP DATABASE IF EXISTS dbname;       -- 删库
```




***

## 2. DDL —— 表操作 + 常用数据类型



```sql
CREATE TABLE student (
  id      INT,
  name    VARCHAR(20),      -- 变长字符串，括号里是最大字符数
  gender  CHAR(1),          -- 定长字符串
  age     TINYINT UNSIGNED, -- 无符号小整数
  score   DECIMAL(5,2),     -- 定点数：总共5位，小数2位（最大999.99）
  birth   DATE,             -- 只有日期
  join_time DATETIME        -- 日期+时间
);

SHOW TABLES;                  -- 看当前库有哪些表
DESC student;                -- 看表结构
DROP TABLE IF EXISTS student; -- 删表

ALTER TABLE student ADD COLUMN phone VARCHAR(11);   -- 加列
ALTER TABLE student MODIFY COLUMN name VARCHAR(30); -- 改类型
ALTER TABLE student CHANGE COLUMN phone tel VARCHAR(11); -- 改列名
ALTER TABLE student DROP COLUMN tel;                -- 删列
RENAME TABLE student TO stu;                       -- 改表名
```




***

## 3. DML —— 增删改数据



```sql
-- 插入
INSERT INTO student (id, name, age) VALUES (1, '张三', 20);
INSERT INTO student VALUES (2, '李四', '男', 21, 88.5, '2005-01-01', NOW()); -- 全列写法
INSERT INTO student (name, age) VALUES ('王五',22), ('赵六',23);             -- 批量

-- 修改
UPDATE student SET age = 21, name = '张三三' WHERE id = 1;
-- ⚠️ 不写 WHERE 会改全表！

-- 删除
DELETE FROM student WHERE id = 1;
-- ⚠️ TRUNCATE TABLE student; 是清空整张表（DDL，不可回滚，速度更快）
```



***

## 4. DQL —— 基础查询



```sql
SELECT * FROM student;                       -- 查所有列（生产环境别用 *）
SELECT name, age FROM student;               -- 指定列
SELECT name AS 姓名, age AS 年龄 FROM student;-- 起别名（AS 可省略）
SELECT DISTINCT age FROM student;            -- 去重
```



***

## 5. DQL —— 条件查询 WHERE



```sql
-- 比较：=  >  <  >=  <=  <>  !=
-- 逻辑：AND  OR  NOT
-- 区间：BETWEEN 20 AND 22   （含两端）
-- 集合：IN (20, 22, 25)
-- 模糊：LIKE  _代表一个字符  %代表任意多个字符
-- 空值：IS NULL / IS NOT NULL   （不能用 = NULL）
```



```
SELECT * FROM student WHERE age = 20;
SELECT * FROM student WHERE age BETWEEN 20 AND 22;
SELECT * FROM student WHERE age IN (20, 22);
SELECT * FROM student WHERE name LIKE '张%';     -- 姓张
SELECT * FROM student WHERE name LIKE '_三';     -- 两个字、第二个字是"三"
SELECT * FROM student WHERE phone IS NULL;
```


***

## 6. DQL —— 聚合函数



| 函数                  | 作用                                |
| ------------------- | --------------------------------- |
| `COUNT(列名)`         | 统计行数（**不计 NULL**）；`COUNT(*)` 计所有行 |
| `MAX(列)` / `MIN(列)` | 最大 / 最小                           |
| `AVG(列)`            | 平均值                               |
| `SUM(列)`            | 求和                                |



```sql
SELECT COUNT(*) FROM student;          -- 总人数
SELECT AVG(score) FROM student;        -- 平均分
SELECT MAX(score), MIN(score) FROM student;
```



***

## 7. DQL —— 分组查询 GROUP BY



```sql
-- 按性别分组，看每组人数和平均分
SELECT gender, COUNT(*), AVG(score)
FROM student
WHERE age > 18          -- 分组前过滤（过滤行）
GROUP BY gender
HAVING AVG(score) > 60; -- 分组后过滤（过滤组）
```

**WHERE vs HAVING 区别（高频）**



* `WHERE`：分组**前**过滤行，不能用聚合函数。

* `HAVING`：分组**后**过滤组，可以用聚合函数。


***

## 8. DQL —— 排序 ORDER BY



```sql
SELECT * FROM student ORDER BY score ASC;   -- 升序（默认）
SELECT * FROM student ORDER BY score DESC;  -- 降序
SELECT * FROM student ORDER BY score DESC, age ASC; -- 先按成绩降序，成绩相同按年龄升序
```



***

## 9. DQL —— 分页 LIMIT



```sql
-- 起始下标从 0 开始；每页 pageSize 条，第 pageNum 页：
--   LIMIT [ (pageNum-1)*pageSize, pageSize ];
SELECT * FROM student LIMIT 0, 10;   -- 第1页，每页10条
SELECT * FROM student LIMIT 10, 10;  -- 第2页
```




***

## 10. 约束（建表时加在列后面）



| 约束 | 关键字                                | 作用                |
| -- | ---------------------------------- | ----------------- |
| 主键 | `PRIMARY KEY`                      | 唯一 + 非空，一张表一个     |
| 自增 | `AUTO_INCREMENT`                   | 主键常用搭配，自动 +1      |
| 非空 | `NOT NULL`                         | 该列不能为 NULL        |
| 唯一 | `UNIQUE`                           | 值不能重复（NULL 可重复）   |
| 默认 | `DEFAULT 值`                        | 不填时给默认值           |
| 外键 | `FOREIGN KEY (a) REFERENCES 主表(b)` | 保证两表数据一致（多表那块再细抠） |



```sql
CREATE TABLE user (
  id    INT PRIMARY KEY AUTO_INCREMENT,  -- 主键 + 自增
  name  VARCHAR(20) NOT NULL,
  phone VARCHAR(11) UNIQUE,
  age   INT DEFAULT 18
);
```



***

## 11. 数据库设计 —— 三大范式



| 范式  | 要求                                          | 反例 → 正例                                  |
| --- | ------------------------------------------- | ---------------------------------------- |
| 1NF | 每列**原子不可再分**                                | 一个 "地址" 列拆成省 / 市 / 区                     |
| 2NF | 在 1NF 基础上，**非主键列必须完全依赖整个主键**（针对联合主键，消除部分依赖） | 选课表 (学号，课程号，学生名，课程名) → 学生名只依赖学号，应拆出去     |
| 3NF | 在 2NF 基础上，**非主键列不能传递依赖主键**（消除传递依赖）          | 学生表里有 "所在学院名"，学院名依赖 "学院编号" 而不是学号 → 拆出学院表 |




***



```
FROM  →  WHERE  →  GROUP BY  →  聚合函数  →  HAVING  →  SELECT  →  ORDER BY  →  LIMIT
```
