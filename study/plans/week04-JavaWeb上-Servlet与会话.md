# W4 · JavaWeb（上）：前端快过 · HTTP · Servlet · 会话

> 2026-11-16（一）– 11-22（日）｜档位：C 框架课｜里程碑：—

## 一、本周目标

1. 前端只求「看得懂、改得动」：HTML/CSS/JS 三天内过完，不陷细节。
2. 讲清 HTTP 请求/响应、状态码、GET/POST 区别。
3. 能独立写出第一个 Servlet 接口，并说清生命周期。
4. 用 Cookie/Session 做出「登录后能拿到当前用户」的最小闭环。

## 二、时间节奏（C 档：看课 90 + 动手 60 + 算法 25 + 记录 5 = 180 分钟）

| 时间块 | 分钟 | 做什么 |
| --- | --- | --- |
| 看课 | 90 | 1.25–1.5 倍速，跳过 CSS 美化/Vue 部分 |
| 动手 | 60 | 当天代码必须跑起来，用浏览器/Postman 验证 |
| 算法 | 25 | 栈/队列主题 1 题 |
| 记录 | 5 | commit + 3 行日志 |

## 三、每日任务

| 天 | 今日目标 | 看课章节（JavaWeb 教程） | 动手任务 | 验收标准 |
| --- | --- | --- | --- | --- |
| 一 11/16 | 前端能看懂 | HTML 常用标签 / CSS 基础 / 表单 | 手写登录表单页（原生 HTML+CSS，不追求好看） | 页面能提交表单；知道 label、input、button 各自作用 |
| 二 11/17 | JS 能改 | JS 基础：变量/函数/对象/DOM/事件/JSON | 给登录页加校验：用户名非空、密码 ≥6 位；控制台打印 JSON 对象 | 校验不通过时页面有提示；能说清「事件绑定」是什么 |
| 三 11/18 | HTTP + 容器 | HTTP 协议（请求/响应、状态码、GET/POST）＋ Tomcat 使用 | 部署 Tomcat，访问静态页；浏览器 F12 → Network 看请求头、响应码 | 能说出 200/302/404/500 分别意味着什么 |
| 四 11/19 | 第一个 Servlet | Servlet 入门（生命周期、`@WebServlet`、执行流程） | 写 `/hello` 接口，返回 JSON 字符串；用 Postman 调通 | 能画出 Servlet 生命周期（init/service/destroy）并说出调用次数 |
| 五 11/20 | 请求与响应 | Request / Response（参数获取、中文乱码、转发与重定向） | 登录接口：POST 表单 → 参数校验 → 成功重定向、失败转发回页面；解决中文乱码 | 转发与重定向各实现一次，能说出 URL 与请求次数差异 |
| 六 11/21 | 会话 | Cookie / Session | 登录成功后写 Session；`/me` 接口返回当前用户名；未登录返回 401 | 关闭浏览器重开仍能保持登录（Session 未过期） |
| 日 11/22 | 复盘 + 验收 | 本周笔记 | 口述 3 问；算法 30 分钟；把本周代码整理进 `study/projects/web-crud/` | 3 问能讲清；项目目录结构清楚 |

## 四、本周产出

- `study/projects/web-crud/`：登录页 + `/hello` + `/login` + `/me` 四个可运行入口
- `study/notes/javaweb-http-servlet.md`：HTTP 要点、Servlet 生命周期、转发 vs 重定向、Cookie vs Session
- 提交信息建议：`week04-d4: 第一个Servlet接口`、`week04-d6: Session登录态`

## 五、自查 3 问（周日口述）

1. GET 和 POST 的区别？什么时候必须用 POST？
2. 转发（forward）和重定向（redirect）的区别？
3. Cookie 和 Session 的区别？Session 存在哪、什么时候失效？

## 六、高频坑 / 减法清单

- **前端陷太深**：CSS 布局不好看直接放过，后端才是主线。
- 没配好 Tomcat/端口就写代码：先跑通一个静态页再写 Servlet。
- 乱码只设一处：请求、响应、数据库三处都要检查。
- **减法顺序**：先跳 CSS 美化 → 再跳 JS 高级特性 → 再跳 Listener；Servlet 生命周期、会话不能跳。

## 七、面试一句话

> Cookie 和 Session 的区别？
> Cookie 存在浏览器，Session 存在服务端；Cookie 里一般只放 SessionId，服务端靠它找回 Session。Cookie 可被禁用/篡改，Session 更安全，但服务端要存，集群需要共享（Redis 或 Session 复制）。
