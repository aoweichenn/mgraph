# 设计 · mgraph

## 边界

负责：param 拓扑、blob producer/consumer、每层 `ParamDict`。
不负责：layer 创建/注册、权重加载、图执行、内存复用、Vulkan。

## 数据模型

```
Blob       name, producer, consumer
LayerSpec  type_index | type name, name, bottoms, tops, ParamDict
Graph      blobs, layers + load_param / load_param_bin / clear
```

文本解析与 ncnn 一致：bottoms 按名称查找，未找到则分配新 blob；tops 始终
使用新槽位。二进制解析直接使用文件中的 blob 索引。

类型解析：

* 文本保留 `type` 名称，`type_index = -1`，由消费方注册表解析；
* 二进制保留 `type_index`，`type` 为空。

## ncnn 接入

`NetPrivate` 持有 `mgraph::Graph`。`Net::load_param*` 先解析图，再把 blobs
复制到 `NetPrivate::blobs`，按 `LayerSpec` 创建 layer、设置 bottoms/tops 和
`ParamDict`。layer 注册、Vulkan fallback、shape hint、执行逻辑仍在 ncnn。

## Layer 与注册

`mgraph::Layer<MatT, OptionT>` 是类型参数化基类：标志、bottoms/tops、形状、
`load_param/load_model/create/destroy/forward` 默认实现都在 mgraph。
`ncnn::Layer` 继承它，只补 Vulkan 方法与 `vkdev`。

`mgraph::LayerFactory<LayerT>` 管理 custom/overwrite builtin 注册、创建、
销毁与 custom index；ncnn `NetPrivate` 持有一个实例。

## ExecutionPlan

`Graph::build_execution_plan` 产生层顺序、blob 最后使用层、网络输入/输出
blob。ncnn 已用它生成 `output_blob_indexes`；更完整的生命周期复用仍由
Extractor 的动态 refcount 逻辑负责。

## 执行循环

`mgraph::Executor<LayerT, MatT, OptionT>` 实现递归执行：

1. 校验 layer index；
2. Input layer 直接返回；
3. 对每个 bottom，若 `blob_mats[bottom].dims == 0`，递归执行 producer；
4. 按 featmask 通过 backend 生成 option；
5. 调用 backend `invoke`。

`ExecutionBackend` 由消费方实现：判断 Input、mask option、执行具体
`do_forward_layer`。ncnn `NetPrivate` 实现该 backend，`forward_layer` 只构造
Executor 并调用 `forward`。

## 后续

下一里程碑把 layer 注册接口与执行循环也迁到 mgraph，定义 `LayerFactory` /
`ExecutionPlan`，ncnn 只保留具体 layer 与 CPU kernel。
