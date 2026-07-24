# 后端agent学习计划 — Week 11

## 本周目标
- Agent 概念与 LLM 基础：理解 LLM、API 调用、prompt 设计与安全性注意。
- 在 Java 后端调用 LLM（通过 HTTP REST API）实现基础问答功能。

## 每日任务（每日约 2 小时）
- Day 1：了解 LLM 与 Agent 的基础概念（对话管理、工具调用）。
- Day 2：阅读主流 LLM 的 API 文档（OpenAI / 其他），注册并获取 API key（可选）。
- Day 3：在 Spring Boot 中实现一个调用外部 HTTP API 的 Service（示例代码）。
- Day 4：实现简单的 Chat 接口：接收用户问题，调用 LLM，返回结果并保存对话历史到 DB。
- Day 5：实现简单的“指令解析”流程：LLM 返回结构化命令（如 JSON），后端解析并执行简单查询。
- Day 6：添加日志与审计：对所有 LLM 调用做记录（输入/输出/成本估算）。
- Day 7：复盘并写安全注意事项（不要把敏感数据发给第三方模型等）。

## 推荐哔哩哔哩视频（可作为入门了解资料）
- LLM 与 Agent 概念入门搜索：
  https://www.bilibili.com/search?keyword=LLM%20Agent%20入门
- OpenAI API 使用教程（中文）：
  https://www.bilibili.com/search?keyword=OpenAI%20API%20教程

