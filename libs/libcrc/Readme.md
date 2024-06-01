# libcrc

Multi platform MIT licensed CRC library in C

<https://github.com/lammertb/libcrc>

## 修改的内容

1. 删除了不必要的测试文件
2. 使用 `precalc` 生成了 `gentab32.inc` 和 `gentab64.inc`, 其用于 `crc32` 和 `crc64` 的编译
