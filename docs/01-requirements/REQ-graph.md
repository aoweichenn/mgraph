# 需求 · 图拓扑

## REQ-001 · 文本 param 解析

**契约**：解析 magic、layer/blob 计数、层类型/名称、bottoms/tops 与每层参数。

**验收**：`Graph.ParsesTextTopologyAndParams`。

## REQ-002 · 二进制 param 解析

**契约**：解析 magic、层类型索引、counts、blob 索引与二进制 ParamDict。

**验收**：`Graph.ParsesBinaryTopology`。

## REQ-003 · Blob 拓扑

**契约**：每个 blob 记录 name（文本）、producer、consumer；文本 bottoms 按名字
复用已有 blob，tops 使用新槽位。

**验收**：`Graph.ParsesTextTopologyAndParams`、`Graph.ParsesBinaryTopology`。

## REQ-004 · 每层参数

**契约**：`LayerSpec` 持有 `mformat::ParamDict`，文本/二进制分别用对应加载器。

**验收**：`Graph.ParsesTextTopologyAndParams`、`Graph.ParsesBinaryTopology`。

## REQ-005 · 输入校验

**契约**：旧 magic、非法 counts、越界 blob 索引返回错误，不保留半成品图。

**验收**：`Graph.RejectsOldMagic`。

## REQ-006 · 清空

**契约**：`clear()` 清空 layers/blobs，允许重复加载。

**验收**：`Graph.ParsesTextTopologyAndParams`。

## REQ-007 · 日志静默

**契约**：无 handler 时完全静默，错误通过返回值表达。

**验收**：`Log.IsSilentWhenNoHandlerInstalled`。

## REQ-008 · 日志转发

**契约**：handler 收到 level/message/user_data，可替换/清除。

**验收**：`Log.ForwardsLevelMessageAndUserData`、`Log.CanBeCleared`。

## REQ-009 · 可机械验证质量

**契约**：格式、类型、头文件、具名常量、API 覆盖、断言、零告警、测试、
安装包纳入 `check-all.sh`。

**验收**：`scripts/check-all.sh`。

## REQ-010 · ncnn 集成

**契约**：`Net::load_param` / `load_param_bin` 使用 `mgraph::Graph` 解析，
再按 spec 创建 layer；189 个 ncnn 测试保持通过。

**验收**：`Graph.ParsesTextTopologyAndParams`、`Graph.ParsesBinaryTopology`。

## REQ-011 · Layer 基类解耦

**契约**：`mgraph::Layer<MatT, OptionT>` 提供标志、参数、生命周期与默认
forward 实现，不依赖 ncnn/Vulkan 类型；ncnn `Layer` 继承它。

**验收**：`Layer.DefaultFlagsAndLifecycle`、`Layer.InplaceForwardClonesAndCallsInplace`。

## REQ-012 · Layer 注册工厂

**契约**：`mgraph::LayerFactory<LayerT>` 管理 custom/overwrite builtin 注册、
创建、销毁、custom index 查找；custom bit 可配置。

**验收**：`LayerFactory.RegistersAndCreatesCustomLayer`、
`LayerFactory.RegistersByIndexAndReportsEntries`、
`LayerFactory.OverwritesBuiltinLayer`。

## REQ-013 · 执行计划

**契约**：`Graph::build_execution_plan` 返回层顺序、每个 blob 的最后使用层、
网络输入/输出 blob 集合。

**验收**：`Graph.BuildsExecutionPlan`。
