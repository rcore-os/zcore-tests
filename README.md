# zcore-tests

Test scripts for [zCore OS](https://github.com/rcore-os/zCore).

## Setup

**You MUST put this repository under the root directory of zCore.**

## Linux libc tests

```
usage: python3 linux_libc_test.py [-h] [-l] [-a {x86_64,riscv64}] [-f] [-t TEST]

optional arguments:
  -h, --help            show this help message and exit
  -l, --libos           test on libos mode (otherwise bare-metal mode, x86_64 only)
  -a {x86_64,riscv64}, --arch {x86_64,riscv64}
                        target architecture
  -f, --fast            do not test known failed and timeout testcases
  -t TEST, --test TEST  run only one test
```

## Zircon core tests (x86_64 only)

```
usage: python3 zircon_core_test.py [-h] [-l] [-f] [-t TEST]

optional arguments:
  -h, --help            show this help message and exit
  -l, --libos           test on libos mode (otherwise bare-metal mode)
  -f, --fast            do not test known failed and timeout testcases
  -t TEST, --test TEST  run only one test
```

## d1 libc test

```
# usage:
python linux_libc_test-board.py -a riscv64 -f -b d1 -D /dev/ttyUSB0
  # 运行所在testcases/linux_libc_test/riscv64_d1.txt里状态为OK的测例
python linux_libc_test-board.py -a riscv64 -t <testcase>
  # 运行单个测例<testcase>

# TODO:
1. utils/test_d1.py里正则表达式的调试
2. testcases/linux_libc_test/riscv64_d1.txt里测例状态的修改
3. 把oscomp里通过的测例加到testcases/linux_libc_test/riscv64_d1.txt里
```
