# 实现 · M1

```
include/mgraph/graph.hpp          Blob / LayerSpec / Graph / ExecutionPlan
include/mgraph/layer.hpp          Layer<MatT, OptionT>
include/mgraph/layer_factory.hpp  LayerFactory<LayerT>
include/mgraph/log.hpp            可注入日志
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

## Layer 解耦

ncnn `Layer` 改为 `mgraph::Layer<Mat, Option>` 的派生类：

* 标志/bottoms/tops/形状/默认 forward 从模板继承；
* ncnn 只保留 Vulkan 方法、`vkdev` 与构造；
* custom/overwrite 注册表改为 `mgraph::LayerFactory<Layer>`。

## ExecutionPlan

ncnn `NetPrivate` 在 load_param 后调用 `graph.build_execution_plan()`，
并用 `plan.output_blobs` 生成输出索引。
