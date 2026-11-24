# 质量策略 · mgraph

| 闸门 | 脚本 |
| --- | --- |
| 格式 / 类型 / 头文件 / 分层 | `format.sh`、`check-types.py`、`check-headers.py`、`check-layers.sh` |
| 无直接输出 / 无 throw | `check-no-printf.py`、`check-no-throw.py` |
| 具名常量 | `check-magic-numbers.py`（移植文件 allow-file） |
| 追溯 / API 覆盖 / 断言 | `check-traceability.py`、`check-api-coverage.py`、`check-test-assertions.py` |
| 零告警 / 单元测试 / 安装包 | `build.sh test`、`ctest`、`package-test.sh` |

新增代码用完整严格警告集；移植的 `graph.cpp` 用 ncnn 上游同级警告集。
clang-tidy 只分析 `src/log.cpp` 与公共头。
