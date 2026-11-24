# 实现 · M1

```
include/mgraph/graph.hpp          Blob / LayerSpec / Graph / ExecutionPlan
include/mgraph/layer.hpp          Layer<MatT, OptionT>
include/mgraph/layer_factory.hpp  LayerFactory<LayerT>
include/mgraph/executor.hpp       Executor / ExecutionBackend
include/mgraph/extractor.hpp      Extractor 编排
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

## 执行循环迁移

`NetPrivate::forward_layer` 不再包含递归/依赖拉取逻辑；它构造
`mgraph::Executor<Layer, Mat, Option>`。`NetPrivate` 实现 `ExecutionBackend`：

* `is_input_layer`：`typeindex == LayerType::Input`；
* `mask_option`：`get_masked_option`；
* `invoke`：`do_forward_layer` + benchmark 包装。

## Extractor 编排迁移

ncnn `Extractor::input` 与 CPU `extract` 的懒执行部分改为构造
`mgraph::Extractor<Layer, Mat, Option>` 并调用 `input` / `ensure`。
输出后处理（packing/fp16/batch）、Vulkan 路径和本地 allocator 仍在 ncnn。
