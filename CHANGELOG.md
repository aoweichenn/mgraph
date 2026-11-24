# Changelog

## 0.1.0 — 2026-11-25

首个独立版本。

* `Graph`：ncnn param 文本/二进制拓扑解析。
* `Blob` producer/consumer 记录。
* `LayerSpec`：类型名/类型索引、bottoms/tops、`ParamDict`。
* `mgraph::Layer<MatT, OptionT>` 与 `LayerFactory<LayerT>`。
* `ExecutionPlan`：层顺序、blob 最后使用层、输入/输出 blob。
* `Executor` / `ExecutionBackend`：递归依赖执行循环。
* `Extractor`：blob 存储、input 与按需 ensure。
* 可注入日志接口。
