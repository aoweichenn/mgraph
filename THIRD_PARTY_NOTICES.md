# 第三方声明

## ncnn 图拓扑解析

`src/graph.cpp` 的 param 文本/二进制拓扑解析来自 ncnn `src/net.cpp`
（Tencent，BSD-3-Clause，Copyright 2017 Tencent）。

本仓库的改动：

* namespace/类型改为 `mgraph` 与固定宽度；
* 参数解析复用 `mformat::ParamDict` / `DataReader`；
* 只保留拓扑与 layer spec，不创建/执行 layer；
* 诊断改为可注入的 `mgraph::LogHandler`；
* 增加独立测试、安装包与质量闸门。
