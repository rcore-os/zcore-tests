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
