# 验证 · M1

环境：Linux x86_64 Fedora 44，GCC 16.2.1，Clang 22.1.8，CMake 4.3.0，GTest 1.17.0。

| 项 | 结果 |
| --- | --- |
| mgraph check-all | 全部通过 |
| 单元测试 | 12/12（graph/layer/log） |
| 追溯矩阵 | 13 条需求、12 个用例 |
| API 覆盖 | 22/22 |
| GCC / Clang | 零告警 |
| ASan + UBSan / TSan | 通过 |
| clang-tidy | 新代码 + 公共头通过 |
| valgrind | 无 definite leak |
| mgraph 安装包消费方 | 通过 |
| ncnn 全量 ctest | **189/189** |

未验证：全模型 zoo、非 x86、OOM 注入。
