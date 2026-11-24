# 代码风格 · mgraph

* 类型 `PascalCase`，函数/变量 `snake_case`，常量 `kPascalCase`，宏 `MGRAPH_*`。
* 固定宽度类型；`std::size_t` 表示大小。
* `src/graph.cpp` 是 ncnn param 解析的行为移植，使用上游同级警告集；
  文本/二进制魔数与上限用 `check-magic-numbers: allow-file`。
* 新增代码（`src/log.cpp`、公共头、测试）使用完整严格集。
* 诊断走 `mgraph/log.hpp`，默认静默。
