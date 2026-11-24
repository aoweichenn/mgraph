# 实现 · M1

```
include/mgraph/graph.hpp     Blob / LayerSpec / Graph
include/mgraph/log.hpp       可注入日志
src/graph.cpp                文本/二进制拓扑解析
src/log.cpp
```

`graph.cpp` 的扫描逻辑来自 ncnn `net.cpp`：

* `scan_net_value` 的整数/字符串边界检查；
* 文本 bottoms 名称查找与 tops 新槽位；
* 二进制 typeindex / blob 索引 / ParamDict。

`Net` 侧改动：

* `NetPrivate` 增加 `graph`；
* 文本/二进制 load 先用 graph 解析，再创建 layer；
* 原解析循环的层类型、bottoms/tops、ParamDict 读取被 spec 访问替代；
* Vulkan fallback、shape hint、featmask、layer `load_param` 保持原逻辑。
