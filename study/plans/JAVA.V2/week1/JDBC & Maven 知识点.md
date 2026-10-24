# JDBC 与 Maven 知识点速通讲义

# 快速学习路线

这份讲义的目标是让你**最快掌握 JDBC 与 Maven 的核心概念和考点**：先用一张路线图建立整体框架，再按「懂概念 → 会流程 → 能刷题」三阶段推进，最后用练习自查。JDBC 重点在**六步流程、PreparedStatement、事务**；Maven 重点在**坐标、依赖 scope、生命周期与常用命令**。

# JDBC 核心知识

## 一句话理解 JDBC

JDBC（Java DataBase Connectivity）是 Java 提供的**访问数据库的标准接口**。它像一座桥：Java 程序用统一的 JDBC API 操作不同数据库（MySQL、Oracle 等），数据库厂商只需提供对应的驱动（Driver）。

## 核心接口一览

|接口 / 类|作用|关键方法|
|---|---|---|
|**DriverManager**|驱动管理器，负责建立连接|getConnection\(url, user, pwd\)|
|**Connection**|数据库连接（会话）|prepareStatement、setAutoCommit、commit、rollback|
|**PreparedStatement**|执行 SQL 的预编译语句对象|executeQuery（查询）、executeUpdate（增删改）、setXxx|
|**ResultSet**|查询结果集|next\(\) 移动行、getXxx\(列名\) 取值|

## 六步使用流程

所有 JDBC 程序的骨架都是下面六步，先看流程图记住顺序，再对照代码理解每一步。

完整最小示例（查询）：

```java
String url = "jdbc:mysql://localhost:3306/test";
String sql = "SELECT name FROM user WHERE id = ?";
try (Connection conn = DriverManager.getConnection(url, "root", "123456");
     PreparedStatement ps = conn.prepareStatement(sql)) {
    ps.setInt(1, 1001);
    try (ResultSet rs = ps.executeQuery()) {
        while (rs.next()) {
            System.out.println(rs.getString("name"));
        }
    }
}  // try-with-resources 自动释放资源
```

## Statement 与 PreparedStatement 对比

|对比项|Statement|PreparedStatement|
|---|---|---|
|传参方式|字符串拼接 SQL|? 占位符 \+ setXxx\(\) 绑定|
|SQL 注入|有风险|可防止|
|预编译|无|有，执行效率更高|
|使用建议|少用|推荐，开发中首选|

## 事务与连接池

**事务**：JDBC 默认每条 SQL 自动提交。手动事务三步：先 conn\.setAutoCommit\(false\)，再执行多条 SQL，最后成功 conn\.commit\(\)、失败 conn\.rollback\(\)。

**连接池**：反复创建、销毁连接开销很大。连接池（Druid、HikariCP）提前创建一批连接并复用，显著提升性能。考点：连接池的作用是**复用连接、提高性能**。

## JDBC 易错点速记

- 释放资源顺序：先 ResultSet → Statement → Connection（先开后关），推荐 try\-with\-resources 自动关闭。

- executeQuery 只用于 SELECT；增删改用 executeUpdate，返回受影响行数。

- URL 格式：jdbc:mysql://主机:端口/数据库名。

- 驱动类名：MySQL 8 是 com\.mysql\.cj\.jdbc\.Driver；JDBC 4\.0 后 Class\.forName 可省略。

- rs\.next\(\) 必须调用后才能取到第一行数据。

# Maven 核心知识

## 一句话理解 Maven

Maven 是 Java 项目的**构建 \+ 依赖管理工具**：一条命令完成编译、测试、打包，并用 pom\.xml 统一管理第三方库和插件。

## 坐标、仓库与依赖范围

依赖定位与获取机制见下图（坐标三要素唯一确定依赖，仓库按顺序查找）：

依赖 scope（生效范围）对照：

|scope|生效范围|典型例子|
|---|---|---|
|**compile**（默认）|编译、测试、运行全程有效|通用业务依赖|
|**provided**|编译、测试有效，运行时不打包|servlet\-api（容器提供）|
|**runtime**|运行、测试有效，编译时不需要|MySQL 驱动|
|**test**|仅测试有效，不进生产包|JUnit|

## 生命周期与常用命令

Maven 有三套独立生命周期（clean、default、site），其中 default 最常用。执行靠后阶段会自动执行它之前的所有阶段，例如 mvn package 会依次执行 compile → test → package。常用命令对照：

|命令|作用|
|---|---|
|**mvn clean**|清理 target 目录|
|**mvn compile**|编译源码|
|**mvn test**|运行测试|
|**mvn package**|打包 jar / war（自动执行之前的阶段）|
|**mvn install**|安装到本地仓库|
|**mvn deploy**|发布到远程仓库|

## 依赖管理要点

- **传递依赖**：A 依赖 B、B 依赖 C 时，A 自动获得 C，无需重复声明。

- **依赖冲突**：路径最近优先，其次先声明优先；必要时用 \<exclusions\> 排除。

- **下载慢**：可在 settings\.xml 配置阿里云镜像仓库加速。

## Maven 易错点速记

- 坐标三要素 groupId \+ artifactId \+ version 缺一不可。

- mvn package 只打包，mvn install 才装进本地仓库，两者顺序是 install 在后。

- test / provided 范围的依赖不会打进生产包。

- 修改 pom\.xml 后，IDE 需要刷新或重新导入才会生效。

# 练习题库（含答案）

## 选择题

**1\.** JDBC 中，用于建立数据库连接的核心类是（B）

- A\. Statement

- B\. DriverManager

- C\. ResultSet

- D\. Connection

**2\.** 执行 SELECT 查询语句应使用（A）

- A\. executeQuery\(\)

- B\. executeUpdate\(\)

- C\. next\(\)

- D\. getConnection\(\)

**3\.** 关于 PreparedStatement，下列说法**错误**的是（D）

- A\. 使用 ? 占位符绑定参数

- B\. 可以防止 SQL 注入

- C\. 语句会被预编译，执行效率更高

- D\. 只能执行查询语句

**4\.** JDBC 手动控制事务时，正确的操作顺序是（C）

- A\. commit\(\) → setAutoCommit\(false\) → 执行 SQL

- B\. setAutoCommit\(true\) → 执行 SQL → rollback\(\)

- C\. setAutoCommit\(false\) → 执行 SQL → commit\(\) 或 rollback\(\)

- D\. 执行 SQL → setAutoCommit\(false\) → commit\(\)

**5\.** Maven 中唯一确定一个依赖的是（A）

- A\. groupId \+ artifactId \+ version

- B\. groupId \+ version

- C\. artifactId \+ version

- D\. 仅 version

**6\.** JUnit 依赖一般使用的 scope 是（　）

- A\. compile

- B\. provided

- C\. test

- D\. runtime

**7\.** 执行 mvn package 时，**不会**被触发的阶段是（　）

- A\. validate

- B\. compile

- C\. test

- D\. install

**8\.** Maven 查找依赖的顺序是（B）

- A\. 中央仓库 → 本地仓库 → 远程仓库

- B\. 本地仓库 → 中央仓库 → 远程仓库

- C\. 远程仓库 → 本地仓库 → 中央仓库

- D\. 中央仓库 → 远程仓库 → 本地仓库

## 判断题

**1\.** JDBC 释放资源时应先关闭 Connection，再关闭 Statement、ResultSet。（F）

**2\.** PreparedStatement 使用参数化查询，可有效防止 SQL 注入。（R）

**3\.** Maven 中 provided 范围的依赖在打包时不会打入最终包。（　）

**4\.** 执行 mvn install 会把项目安装到远程仓库。（　）

**5\.** JDBC URL 的标准格式为 jdbc:mysql://主机:端口/数据库名。（R）

## 代码补全题

**1\.** 补全 JDBC 查询代码（填类名或方法名）：

```java
String url = "jdbc:mysql://localhost:3306/student";
String sql = "SELECT name FROM user WHERE id = ?";
try (Connection conn = ____①____.getConnection(url, "root", "123456");
     PreparedStatement ps = conn.____②____(sql);
     ResultSet rs = ps.____③____()) {
    ps.setInt(1, 1);
    while (rs.____④____()) {
        System.out.println(rs.____⑤____("name"));
    }
}
```

**2\.** 补全 pom\.xml 中的依赖声明（MySQL 驱动）：

```xml
<dependency>
    <groupId>mysql</groupId>
    <artifactId>____①____</artifactId>
    <version>____②____</version>
    <scope>____③____</scope>  <!-- 运行期才需要，编译期用不到 -->
</dependency>
```

## 简答题

**1\.** 简述 JDBC 访问数据库的六步流程。

**2\.** 为什么推荐使用 PreparedStatement 而不是 Statement？

**3\.** Maven 的 scope 有哪些常见取值？分别表示什么？

## 参考答案与解析

**选择题**

|题号|答案|解析|
|---|---|---|
|1|**B**|DriverManager\.getConnection\(\) 负责建立连接并返回 Connection 对象。|
|2|**A**|executeQuery\(\) 用于 SELECT，返回 ResultSet；executeUpdate\(\) 用于增删改。|
|3|**D**|PreparedStatement 可执行增删改查所有语句，不只是查询。|
|4|**C**|先关掉自动提交 setAutoCommit\(false\)，执行完 SQL 后按结果 commit\(\) 或 rollback\(\)。|
|5|**A**|坐标三要素 groupId \+ artifactId \+ version 共同唯一确定依赖。|
|6|**C**|JUnit 只在测试阶段使用，scope 为 test，不进生产包。|
|7|**D**|package 触发它之前的所有阶段（validate、compile、test），install 在其后，不会触发。|
|8|**B**|先查本地仓库，没有再去中央仓库，再没有去远程仓库（含镜像）。|

**判断题**

|题号|答案|解析|
|---|---|---|
|1|**×**|应按先开后关原则：先关 ResultSet，再 Statement，最后 Connection。|
|2|**√**|参数化查询把值作为数据而非 SQL 代码处理，可防注入。|
|3|**√**|provided 运行期由容器提供，不打进最终包。|
|4|**×**|mvn install 安装到本地仓库，deploy 才发布到远程仓库。|
|5|**√**|格式为 jdbc:mysql://主机:端口/数据库名。|

**代码补全题**

**1\.** ① DriverManager　② prepareStatement　③ executeQuery　④ next　⑤ getString

**2\.** ① mysql\-connector\-java　② 8\.0\.33（任一真实版本号）　③ runtime

**简答题**

**1\.** ① 加载驱动（JDBC 4\.0\+ 可省略）② 通过 DriverManager 获取连接 ③ 创建 Statement / PreparedStatement ④ 执行 SQL ⑤ 处理 ResultSet 结果集 ⑥ 关闭资源（先开后关）。

**2\.** ① 使用 ? 占位符参数化传参，可防止 SQL 注入，更安全；② 语句预编译后可复用执行计划，效率更高；③ 代码更清晰，避免手工拼接字符串。

**3\.** compile（默认，全程有效）、provided（编译测试有效，运行不打进包）、runtime（运行测试有效，编译不需要）、test（仅测试有效）。

