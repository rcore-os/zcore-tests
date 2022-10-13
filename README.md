# zcore-tests

Test scripts for [zCore OS](https://github.com/rcore-os/zCore).

## Setup

**You MUST put this repository under the root directory of zCore.**

## Linux libc tests on qemu

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

## Linux libc test on boards

```shell
# usage:
python linux_libc_test-board.py [-a <arch>] [-f] [-b <board>] [-D <dev>]
  # options:
  -a <arch>, --arch <arch>	# target architecture, can be : x86_64(default), riscv64, aarch64
  -f, --fast	# do not test known failed and timeout testcases
  -b <board>, --board <board>	# board name, can be : nezha(default), unmatched, visionfive, light, cr1825
  -D <dev>, --device <dev>	# specify the device. default : /dev/ttyUSB0
  
python linux_libc_test-board.py -t <testcase> [-a <arch>] [-b <board>] [-D <dev>]
  # run only one test <testcase>
```
