#!/bin/bash
# 作用：统计在zCore上通过的系统调用和libc-test测例
# 使用方法：
# ./statistic -e <dir> : 提取每个测例所依赖的系统调用，包含系统调用的文件和测例同名，放在$SYSCALL_DIR目录
#     <dir> : 测例所在的目录，一般来说这个目录的名字就是libc-test
# ./statistic -a <arch> : 把已通过的测例所包含的系统调用合并到一个文件，前提是要求$SYSCALL_DIR目录存在
#    <arch> : 测例的架构，比如x86_64、riscv64等
# ./statistic -a <arch> -e <dir> : 先从测例提取系统调用，再合并所需的系统调用，就是上面的两步合而为一

SYSCALL_DIR="libc_syscall/"

# If something goes wrong, print help message.
function help() {
  echo -e "USAGE:
    ./statistic -a <arch> : count passed libc testcases and syscalls\n\
    ./statistic -e <dir> : only extract syscalls\n \
      Options:\n \
      -a <arch> : which arch is libc-test built on\n \
      -e <dir> : extract syscalls for libc excutable file"
}

# extract passed testcases from ./testcases/linux_libc_test/<arch>_bare.txt
# $1 : ARCH
function extract_testcase() {
  src_name="./testcases/linux_libc_test/"$1"_bare.txt"
  dst_name="passed_testcases_"$1".txt"
  grep OK $src_name > $dst_name 
  sed -i "s/ *OK//" $dst_name
}

# extract syscalls for libc excutable file
# $1 : libc-test dir
function extract_syscall() {
  echo "extract syscall $1"
  mkdir -p $SYSCALL_DIR
  for i in `find $1 -type f -name "*exe"`; do
    testcase_name=${i##*/}
    strace -c $i 2>&1 | tee $SYSCALL_DIR$testcase_name;
  done
}

# merge and sort syscalls
# $1 : ARCH
function merge_syscall() {
  dst_name="passed_testcases_"$1".txt"
  syscall_file="syscall_"$1".txt"

  > $syscall_file
  for i in `cat $dst_name`; do
    testcase_name=${i##*/}
    cat $SYSCALL_DIR$testcase_name >> $syscall_file
  done

  sed -i "/^X/d" $syscall_file
  sed -i "/total/d" $syscall_file
  sed -i "/\%/d" $syscall_file
  sed -i "/--/d" $syscall_file
  sed -i "/Error/d" $syscall_file
  sed -i "s/.\{51\}//" $syscall_file
  cat $syscall_file | sort | uniq > tmptmp
  mv tmptmp $syscall_file
}

function main() {
  while getopts "a:e:" OPT; do
    case $OPT in
      a)
        ARCH="$OPTARG"
        MERGE_SYS="yes";;
      e)
        DIR_SYSCALL="$OPTARG"
        EXTRACT_SYS="yes";;
      ?)
        help
        exit;;
    esac
  done

  if [ "$EXTRACT_SYS"x == "yes"x ]; then
    extract_syscall $DIR_SYSCALL
  fi
  if [ "$MERGE_SYS"x == "yes"x ]; then
    extract_testcase $ARCH
    merge_syscall $ARCH
  fi
}

main $*
