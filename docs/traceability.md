# 追溯矩阵 · mgraph

> `check-traceability.py --check` 校验：REQ-001..010 无缺号、每条有 `●`、
> 引用文件存在、`Suite.Case` 真实存在、**每个 TEST 都被本文提及**。

## 1. 需求

| 需求 | 设计 | 实现 | 测试 |
| --- | --- | --- | --- |
| **REQ-001** 文本 param 解析 | DES §2 | ● `include/mgraph/graph.hpp`<br>● `src/graph.cpp` | ● `Graph.ParsesTextTopologyAndParams` |
| **REQ-002** 二进制 param 解析 | DES §2 | ● `src/graph.cpp` | ● `Graph.ParsesBinaryTopology` |
| **REQ-003** Blob 拓扑 | DES §2 | ● `include/mgraph/graph.hpp`<br>● `src/graph.cpp` | ● `Graph.ParsesTextTopologyAndParams`<br>● `Graph.ParsesBinaryTopology` |
| **REQ-004** 每层参数 | DES §2 | ● `include/mgraph/graph.hpp` | ● `Graph.ParsesTextTopologyAndParams`<br>● `Graph.ParsesBinaryTopology` |
| **REQ-005** 输入校验 | DES §2 | ● `src/graph.cpp` | ● `Graph.RejectsOldMagic`
● `Graph.BuildsExecutionPlan`
● `Layer.DefaultFlagsAndLifecycle`
● `Layer.InplaceForwardClonesAndCallsInplace`
● `LayerFactory.RegistersAndCreatesCustomLayer`
● `LayerFactory.RegistersByIndexAndReportsEntries`
● `LayerFactory.OverwritesBuiltinLayer`
● `Executor.RunsLayersInDependencyOrder`
● `Executor.RejectsInvalidLayerAndMissingProducer`
● `Executor.BackendMasksOptionWithoutFeatmask` |
| **REQ-006** 清空 | DES §2 | ● `src/graph.cpp` | ● `Graph.ParsesTextTopologyAndParams` |
| **REQ-007** 日志静默 | DES §3 | ● `src/log.cpp` | ● `Log.IsSilentWhenNoHandlerInstalled` |
| **REQ-008** 日志转发 | DES §3 | ● `src/log.cpp` | ● `Log.ForwardsLevelMessageAndUserData`<br>● `Log.CanBeCleared` |
| **REQ-011** Layer 基类解耦 | DES §3 | ● `include/mgraph/layer.hpp` | ● `Layer.DefaultFlagsAndLifecycle`<br>● `Layer.InplaceForwardClonesAndCallsInplace` |
| **REQ-012** Layer 注册工厂 | DES §3 | ● `include/mgraph/layer_factory.hpp` | ● `LayerFactory.RegistersAndCreatesCustomLayer`<br>● `LayerFactory.RegistersByIndexAndReportsEntries`<br>● `LayerFactory.OverwritesBuiltinLayer`
● `Executor.RunsLayersInDependencyOrder`
● `Executor.RejectsInvalidLayerAndMissingProducer`
● `Executor.BackendMasksOptionWithoutFeatmask` |
| **REQ-014** 通用执行循环 | DES §5 | ● `include/mgraph/executor.hpp` | ● `Executor.RunsLayersInDependencyOrder`<br>● `Executor.RejectsInvalidLayerAndMissingProducer`<br>● `Executor.BackendMasksOptionWithoutFeatmask` |
| **REQ-013** 执行计划 | DES §2 | ● `include/mgraph/graph.hpp`<br>● `src/graph.cpp` | ● `Graph.BuildsExecutionPlan` |
| **REQ-009** 可机械验证质量 | QUALITY | ● `scripts/check-all.sh` | ● `scripts/check-all.sh`（全绿） |
| **REQ-010** ncnn 集成 | DES §4 | ● `src/graph.cpp` | ● `Graph.ParsesTextTopologyAndParams`<br>● `Graph.ParsesBinaryTopology` |

## 2. 用例清单（反向检查）

● `Graph.ParsesTextTopologyAndParams`
● `Graph.ParsesBinaryTopology`
● `Graph.RejectsOldMagic`
● `Graph.BuildsExecutionPlan`
● `Layer.DefaultFlagsAndLifecycle`
● `Layer.InplaceForwardClonesAndCallsInplace`
● `LayerFactory.RegistersAndCreatesCustomLayer`
● `LayerFactory.RegistersByIndexAndReportsEntries`
● `LayerFactory.OverwritesBuiltinLayer`
● `Executor.RunsLayersInDependencyOrder`
● `Executor.RejectsInvalidLayerAndMissingProducer`
● `Executor.BackendMasksOptionWithoutFeatmask`
● `Log.IsSilentWhenNoHandlerInstalled`
● `Log.ForwardsLevelMessageAndUserData`
● `Log.CanBeCleared`

## 3. 未覆盖项

| 项 | 原因 |
| --- | --- |
| Layer 创建/注册/执行 | 下一里程碑 |
| 全模型 zoo | 需要外部模型与数据集 |
| 非 x86 | 只验证 x86_64 |
| OOM 注入 | 未做 RLIMIT_AS 子进程 |
