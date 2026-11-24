# mgraph

`mcnn` 的图拓扑层：解析 ncnn param 文本/二进制，输出 `Blob` 与 `LayerSpec`。
不创建 layer、不执行推理、不持有权重。

## 能力

| 项 | 状态 |
| --- | --- |
| 文本 param 拓扑解析 | ✅ |
| 二进制 param 拓扑解析 | ✅ |
| blob producer/consumer 记录 | ✅ |
| 每层 `ParamDict` | ✅ |
| Layer 基类模板 / LayerFactory | ✅ |
| ExecutionPlan（层顺序、blob 最后使用、输入输出） | ✅ |
| 通用执行循环 `Executor` / `ExecutionBackend` | ✅ |
| 可注入日志 | ✅ |

## 快速开始

```cpp
#include <mgraph/mgraph.hpp>

const char text[] =
    "7767517 1 1\n"
    "Input input 0 1 data\n";
const auto* mem = reinterpret_cast<const std::uint8_t*>(text);
mformat::DataReaderFromMemory reader(mem);

mgraph::Graph graph;
graph.load_param(reader);

const mgraph::LayerSpec& layer = graph.layers[0];
```

## 构建

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -Dmgraph_build_tests=ON
cmake --build build -j"$(nproc)"
ctest --test-dir build
```

## 与 mcnn 的关系

ncnn `Net::load_param` / `load_param_bin` 现在先用 `mgraph::Graph` 解析拓扑与
参数，再按 `LayerSpec` 创建 layer。layer 注册与执行仍在 ncnn，作为下一步
`mgraph` 的执行/注册接口扩展点。
