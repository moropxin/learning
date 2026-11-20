# W6 · JavaWeb 核心速通（Servlet + 会话 + JSON）

> 2026-11-30（一）– 12-06（日）｜档位：C 框架课｜里程碑：**M3 项目第一批接口跑通**

> 这周只为一件事服务：**12 月 13 日之前，你能端着电脑演示一个"带登录、能增删改查"的 Web 项目。**
> 所以 JSP、EL、JSTL 全部跳过（黑马新版本身也在弱化），只学 HTTP → Servlet → 会话 → Filter → JSON 这一条最短路径。

## 一、本周目标

1. 能说清一次 HTTP 请求的组成（请求行/头/体），会看 F12 的 Network。
2. 会写 Servlet（注解方式 `@WebServlet`），处理 GET/POST、拿参数、返回 JSON。
3. 会区分转发/重定向，会用 Cookie/Session 做登录态。
4. 会写 Filter，做一个"未登录跳登录页"的拦截。
5. 12/05 起开工自选项目：3 张表、三层包结构、第一天出第一个查询接口。

## 二、时间节奏（C 档：看课 90 + 敲代码 60 + 算法 25 + 记录 5 = 180 分钟）

| 时间块 | 分钟 | 做什么 |
| --- | --- | --- |
| 看课 | 90 | 黑马「JavaWeb 基础」HTTP/Tomcat/Servlet/会话/Filter，1.25 倍速；JSP 章节跳过 |
| 敲代码 | 60 | 每天一个能跑的功能点，当天用 Postman 验证 |
| 算法 | 25 | 力扣 1 题 |
| 记录 | 5 | 日志 + commit（术语在周日记 3 个） |

## 三、每日任务

| 天 | 今日目标 | 看课章节 | 动手任务 | 验收标准 |
| --- | --- | --- | --- | --- |
| 一 11/30 | HTTP + Tomcat + 第一个 Servlet | HTTP 协议、Tomcat 目录、`@WebServlet` | IDEA 建 Maven Web 工程，写 `/hello` 返回一段中文 | 浏览器访问不乱码 |
| 二 12/01 | Servlet 生命周期 + 参数 + 响应 | `init/service/destroy`、`getParameter`、`setContentType` | 写 `/login` 接收用户名密码，返回 JSON 字符串 | Postman 能看到 JSON |
| 三 12/02 | 转发重定向 + 四大作用域 + 会话 | `forward`/`sendRedirect`、request/session/application | 登录成功后把用户存 Session，`/me` 读出来 | 能说清转发与重定向的 3 点区别 |
| 四 12/03 | Filter + 登录校验 | `Filter` 生命周期、`chain.doFilter` | 写登录校验 Filter，未登录访问 `/me` 跳登录 | 能说出过滤器与拦截器的区别（先有个印象） |
| 五 12/04 | JSON + 前后端分离 | Jackson、统一返回体、跨域概念 | 用 Jackson 把对象转 JSON；封装 `Result{code,msg,data}` | 三个接口返回格式统一 |
| 六 12/05 | **项目开工**：需求 + 建库 + 三层 | — | 定需求（3 张表，如用户/商品/订单）、建库建表、建 `controller/service/mapper` 包 | 表建好，能连上数据库 |
| 日 12/06 | 项目第一批接口 | — | 列表 + 新增 + 删除 三个接口，Postman 全部跑通 | 6 个接口可用（含登录、详情） |

## 四、本周产出

- `study/projects/web-crud/`：Servlet 版项目骨架（能跑、有登录、有 3 个 CRUD 接口）
- `study/projects/web-crud/README.md`：需求说明 + 表结构 + 接口列表 + 启动步骤
- `study/notes/javaweb.md`：HTTP 请求组成、转发 vs 重定向、Cookie vs Session、Filter 执行顺序
- `PROGRESS.md`：W6 打卡 + 日志

## 五、自查 3 问 + 1 个术语关（周日口述）

1. 转发和重定向的区别？什么时候必须用重定向？
2. Cookie 和 Session 的区别？Session 存在哪、为什么需要 JSESSIONID？
3. Filter 的执行顺序和生命周期？它为什么能统一处理编码问题？

- [ ] 4. 本周新名词能一句话讲清 80% 吗？

## 六、高频坑 / 减法清单

| 坑 | 说明 |
| --- | --- |
| 响应中文乱码 | `response.setContentType("text/html;charset=utf-8")` 必须写在 `getWriter()` 之前 |
| 请求参数乱码 | POST 用 `request.setCharacterEncoding("utf-8")`（Tomcat 8+ 默认 UTF-8，先确认再改） |
| 一个 Servlet 一个 `@WebServlet` 路径写重 | 启动直接报 `IllegalArgumentException`，两个类不能映射同一路径 |
| 转发后还继续写响应 | `forward` 之后 `return`，否则报 `response already committed` |
| 把 JSP 当必修 | **本周直接跳过**，前后端分离项目用不上 |

**本周减法（已执行）**：JSP/EL/JSTL 全跳；`Listener` 只看不写；`Servlet 3.0 注解` 之外的老式 `web.xml` 配置只认不背。
**不能砍**：HTTP 请求结构、Servlet 生命周期、Session 登录态、Filter —— 12 月面试必问其中至少两个。

## 七、面试一句话

> **Cookie 和 Session 的区别？**
> Cookie 存在**浏览器端**，是服务端 `Set-Cookie` 下发、之后每次请求自动带上的小段文本，不安全、容量小（4KB）；Session 存在**服务端**（默认内存），浏览器只保存一个 `JSESSIONID` 的 Cookie 作为"取件码"。所以 Session 更安全、能存对象，但占服务端内存、集群下需要共享（Redis 存 Session 就是这个原因）。"记住我"这类要跨会话保存的，通常把 token 放 Cookie 里。
