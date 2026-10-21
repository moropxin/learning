# W7 · Spring 全家桶速通：IOC · AOP · SpringMVC · SpringBoot

> 2026-12-07（一）– 12-13（日）｜档位：C 框架课｜里程碑：**M4 SpringBoot 最小 CRUD**

> 这周是「从 Servlet 手写到框架开发」的转折点。目标不是把 SSM 学完，而是**能独立搭一个 SpringBoot + MyBatis 的接口工程**，因为 W8 苍穹外卖第一天就要用。

## 一、本周目标

1. 讲清 IOC/DI 解决了什么问题，会用注解管理 Bean。
2. 讲清 AOP 的用途，能写一个日志切面；知道 `@Transactional` 什么情况下失效。
3. 会用 SpringMVC 注解写 RESTful 接口、统一异常处理。
4. 能用 SpringBoot 搭起「MyBatis + 统一返回 + 拦截器」的最小 CRUD 工程。

## 二、时间节奏（C 档：看课 90 + 动手 60 + 算法 25 + 记录 5 = 180 分钟）

| 时间块 | 分钟 | 做什么 |
| --- | --- | --- |
| 看课 | 90 | 跳过 XML 配置、AOP 源码、Maven 高级 |
| 动手 | 60 | 当天必须有一个能启动的接口 |
| 算法 | 25 | 递归/树 1 题 |
| 记录 | 5 | commit + 3 行日志 |

## 三、每日任务

| 天 | 今日目标 | 看课章节（SSM 教程） | 动手任务 | 验收标准 |
| --- | --- | --- | --- | --- |
| 一 12/07 | IOC / DI | Spring：IOC、DI、Bean 管理（注解） | 用注解把 Service 注入 Controller；对比「自己 new」和「容器给」 | 能说清 IOC 解决了什么（解耦、可测试、统一管理生命周期） |
| 二 12/08 | AOP + 事务 | AOP、事务管理 | 写一个 `@Around` 日志切面打印方法耗时；用 `@Transactional` 演示回滚；复现「同类方法自调用导致事务失效」 | 能说出 `@Transactional` 失效的 3 种常见场景 |
| 三 12/09 | SpringMVC | SpringMVC：注解、RESTful、参数绑定、统一异常 | 写 4 个 RESTful 接口（GET/POST/PUT/DELETE），加 `@RestControllerAdvice` 全局异常 | Postman 4 个请求全通；异常返回统一 JSON |
| 四 12/10 | SpringBoot 起步 | SpringBoot：起步依赖、自动配置、YAML、多环境 | 建 SpringBoot 工程，配 `application.yml` 数据源，切 dev/prod 两套配置 | 能说清「自动配置」大致怎么生效（`@EnableAutoConfiguration` + starter） |
| 五 12/11 | 最小 CRUD | 无（综合） | 用 SpringBoot + MyBatis 做「员工」CRUD：统一返回 `Result`、全局异常、登录拦截器、JWT | 5 个接口全通；拦截器生效；返回结构统一 |
| 六 12/12 | 文档与测试 | 接口文档（Knife4j / Apifox） | 接入 Knife4j 或用 Apifox 建接口集合；给 Service 写 2 个单测 | 打开文档页能看到全部接口并能调试 |
| 日 12/13 | M4 验收 | 本周笔记 | 口述 3 问；算法 30 分钟；把工程整理成可复用模板 | 30 分钟内能从零新建一个带 CRUD 的 SpringBoot 工程 |

## 四、本周产出

- `study/projects/springboot-demo/`：可复用的最小 CRUD 模板（统一返回、全局异常、拦截器、JWT、接口文档）
- `study/notes/spring.md`：IOC/DI、AOP、`@Transactional` 失效场景、自动配置、常用注解清单
- 提交信息建议：`week07-d2: 日志切面`、`week07-d5: 最小CRUD工程`

## 五、自查 3 问（周日口述）

1. IOC 和 DI 是什么？不用 Spring 会怎样？
2. `@Transactional` 什么情况下会失效？事务的传播行为有哪些常见值？
3. SpringBoot 的自动配置原理，用一句话怎么说清？

## 六、高频坑 / 减法清单

- 陷进 Spring 源码：这周只看「怎么用 + 一句话原理」，源码留到面试前。
- 注解不生效：先查包扫描路径、`@Configuration`、代理方式（JDK/CGLIB）。
- **减法顺序**：先跳 AOP 注解细节 → 再跳多环境 → 再跳 Knife4j；IOC、`@Transactional`、RESTful、自动配置不能跳。

## 七、面试一句话

> `@Transactional` 为什么会失效？
> 因为它基于 AOP 代理：同类内部方法自调用不走代理、方法不是 `public`、异常被自己 catch 掉、抛的是受检异常而默认只回滚 `RuntimeException`、或者数据库引擎不支持事务——这几种都会失效。
