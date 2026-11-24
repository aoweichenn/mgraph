# 复审记录 · mgraph

## RV-01 · Graph 解析签名应为 const（已修）

`Net::load_param(const DataReader&)` 传入 const reader；Graph 最初用非 const
引用导致编译失败。改为 `const mformat::DataReader&`。

## RV-02 · 文本 param 不能全部写在一行（测试问题）

ParamDict 以换行结束一层参数；测试最初把所有层写在一行，导致参数解析失败。
测试改为每层一行，与真实 param 一致。

## RV-03 · 二进制测试布局

构造二进制 param 时把 top_count 写错，Graph 正确拒绝。修正测试布局后通过。

## RV-04 · ncnn 集成边界（决定）

本轮只迁拓扑与参数解析；layer 创建/注册、执行、内存复用仍在 ncnn。这样
189 测试可验证行为不变，同时为下一步 `LayerFactory`/`ExecutionPlan` 留出接口。

## RV-05 · 移植代码警告/常量基线（决定）

`graph.cpp` 使用 ncnn 上游同级警告集；魔数/上限用 allow-file。新增 log/头/
测试走严格集；clang-tidy 只分析新代码与公共头。
