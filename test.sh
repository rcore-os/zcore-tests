#!/bin/bash -x
# Note : this script should run on zCore, to execute testcases.
#   to run : sh test.sh <arch>
# Note : this script can also run on Linux, to generate files needed on zCore.
#   to run : ./test.sh linux

TESTCASE_PATH=testcases/linux_libc_test

if [ $1 == linux ]; then
  for arch in riscv64 x86_64 aarch64
  do
    echo ${arch}
    awk '/OK/ {print $1}' ${TESTCASE_PATH}/${arch}_bare.txt | tee ${arch}.txt
    for type in functional math regression
    do
      grep \/$type ${arch}.txt | tee ${arch}_${type}.txt
    done
    sed -i "s/^.*functional\///" ${arch}_functional.txt
    sed -i "s/^.*math\///" ${arch}_math.txt
    sed -i "s/^.*regression\///" ${arch}_regression.txt
  done
else
  for type in functional math regression
  do
    cat $1_${type}.txt | while read basename
    do
      testcase=/libc-test/src/${type}/${basename}
      echo "========== START $basename =========="
      eval $testcase
      if [ $? = 0 ]; then
        echo "========== END $basename =========="
      fi
    done
  done
fi
