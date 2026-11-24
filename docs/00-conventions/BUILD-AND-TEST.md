# 构建与测试 · mgraph

依赖 `../mformat`（会引入 `../mmat`、`../mmemory`、`../mplatform`）。

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -Dmgraph_build_tests=ON
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
```

脚本：`build.sh test`、`sanitize.sh`、`tsan.sh`、`analyze.sh`、
`package-test.sh`、`check-all.sh`。
