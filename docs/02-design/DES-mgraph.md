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

## 后续

下一里程碑把 layer 注册接口与执行循环也迁到 mgraph，定义 `LayerFactory` /
`ExecutionPlan`，ncnn 只保留具体 layer 与 CPU kernel。
