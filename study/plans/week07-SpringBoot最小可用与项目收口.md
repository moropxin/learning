# W7 · SpringBoot 最小可用 + 项目收口 + 简历 v0 + 投递启动

> 2026-12-07（一）– 12-13（日）｜档位：C 框架课 + D 项目｜里程碑：**M3 项目可演示 / M5 简历 v0 + 首批投递（12/13）**

> **本周是整个计划的分水岭。** 七天里你要同时完成四件事：Maven → SpringBoot 最小可用 → MyBatis 打通数据库 → 简历 v0 与投递表。
> 学框架的标准是"**能写三层、能连库、能返回统一 JSON**"，不是"懂自动配置原理"。原理留给春招前的八股阶段。

## 一、本周目标

1. 会用 Maven 加依赖、看懂 `pom.xml` 的坐标。
2. 能独立建 SpringBoot 工程，写出 Controller/Service/Mapper 三层并跑通。
3. 会用 MyBatis（注解 + XML 两种）连 MySQL，`#{}` 和 `${}` 的区别能讲清。
4. 会做登录 + 拦截器 + 全局异常处理（`@RestControllerAdvice`）。
5. **12/12 简历 v0 一页纸完成；12/13 建立深圳 30 家岗位池并开始投递。**

## 二、时间节奏（C 档：看课 90 + 敲代码 60 + 算法 25 + 记录 5 = 180 分钟）

| 时间块 | 分钟 | 做什么 |
| --- | --- | --- |
| 看课 | 90 | 黑马「Maven」「SpringBoot3 快速入门」「MyBatis」，1.25 倍速，只看到"能写 CRUD"为止 |
| 敲代码 | 60 | 每天把项目往前推一个模块，当天 Postman 验通 |
| 算法 | 25 | 力扣 1 题（Hot100 简单） |
| 记录 | 5 | 日志 + commit（12/12、12/13 各留 1 小时给简历与投递） |

## 三、每日任务

| 天 | 今日目标 | 看课章节 | 动手任务 | 验收标准 |
| --- | --- | --- | --- | --- |
| 一 12/07 | Maven + SpringBoot 起步 | Maven 坐标/依赖/生命周期、start.spring.io | 建工程，写 `/hello` 返回 JSON；看启动日志里的 Tomcat 端口 | `mvn spring-boot:run` 能起 |
| 二 12/08 | 三层结构 + 参数接收 | `@RestController`、`@RequestBody`、`@PathVariable` | 把昨天 Servlet 的 3 个接口改成 SpringBoot 版 | 接口行为不变 |
| 三 12/09 | MyBatis 连库 | 注解 Mapper、XML Mapper、`#{}` vs `${}`、PageHelper | 用 MyBatis 查出商品列表，加分页 | 能查到数据且分页正确 |
| 四 12/10 | 登录 + 拦截器 + 全局异常 | `HandlerInterceptor`、`@RestControllerAdvice` | 登录接口发 token（简易 JWT 或 UUID+Redis 先不做，用 Session 也行）；拦截器校验 | 未登录访问返回 401 |
| 五 12/11 | 项目收口 | — | 补齐改/查详情/条件查询；`mvn package` 打成 jar 跑一遍 | 8 个接口全部可用 |
| 六 12/12 | **简历 v0 + 仓库打磨** | — | 一页简历（技能/项目/C 语言与数据结构/蓝桥杯）；GitHub README 加架构图与接口表 | 简历能让陌生人 30 秒看懂你做过什么 |
| 日 12/13 | **投递表 + 演示录屏** | — | BOSS 直聘筛"深圳 Java 实习"30 家入表；录 3 分钟项目演示 | 投递表有 30 行；演示能讲清"我做了什么" |

## 四、本周产出

- `study/projects/web-crud/`：SpringBoot 版项目（三层 + MyBatis + 登录 + 统一返回 + 全局异常），可 `java -jar` 启动
- `study/resume/resume-v0.md`：一页简历（Markdown 版，导出 PDF）
- `study/plans/投递记录.md`：深圳 30 家岗位池（公司/岗位/JD 关键词/投递日期/状态/复盘）
- `PROGRESS.md`：W7 打卡 + M3/M5 结论

## 五、自查 3 问 + 1 个术语关（周日口述）

1. SpringBoot 相比 SSM 省了什么？（至少说 3 点：起步依赖、自动配置、内嵌 Tomcat）
2. `#{}` 和 `${}` 的区别？为什么 `${}` 有 SQL 注入风险？
3. 你的项目里，一个请求从 Controller 到数据库经过了哪些层？各层职责是什么？

- [ ] 4. 本周新名词能一句话讲清 80% 吗？（Bean/依赖注入/自动配置/切面）

## 六、高频坑 / 减法清单

| 坑 | 说明 |
| --- | --- |
| 陷进"自动配置原理" | 本周只要"知道它帮你配好了"；`@EnableAutoConfiguration` 源码不追 |
| Mapper 接口没加 `@Mapper` | 启动报找不到 Bean；或在启动类加 `@MapperScan` |
| `application.yml` 缩进错 | YAML 对空格敏感，报错信息通常很含糊 |
| 端口被占用 | `server.port` 改 8081，别再折腾杀进程 |
| 简历写成"技术栈罗列" | 每条技能后面必须挂一个"用它做过什么"，否则等于没写 |
| 投递只投大厂 | 本周目标是小厂/外包/创业公司，要求"能到岗、能干活" |

**本周减法（已执行）**：Spring 手写 IOC/AOP 原理 → 跳过；`Bean` 生命周期 → 只记四个阶段名；MyBatis 一级/二级缓存 → 跳过。
**不能砍**：三层结构、MyBatis 连库、统一返回、登录拦截、简历与投递表。

## 七、面试一句话

> **SpringBoot 为什么能"开箱即用"？**
> 靠两个机制：**起步依赖**（`spring-boot-starter-web` 一次性带入 Tomcat、SpringMVC、Jackson 等，版本由父 POM 统一管理，不用自己解决冲突）和**自动配置**（`@EnableAutoConfiguration` 通过 `spring.factories`/`AutoConfiguration.imports` 加载候选配置类，再用 `@ConditionalOnClass`、`@ConditionalOnMissingBean` 等条件注解决定"类路径上有这个类、且你没自己配，我才给你配一个默认的"）。所以它本质是"**约定优于配置 + 条件装配**"，不是魔法：你一旦自己定义了 `DataSource`，默认的就不生效。
