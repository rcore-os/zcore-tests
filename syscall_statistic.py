import os
import codecs
import re
import signal
import argparse

# 运行shell命令
def exec_cmd(cmd):
    text = os.popen(cmd)
    return text

# 超时计时
class TimeOutException(Exception):
    def __init__(self, *args, **kwargs):
        pass

def timeout_handler(signum, frame):
    print("timeout signal received")
    # raise TimeOutException()

# 获得libc_test结果列表
def gen_test_list(passed_test, failed_test, arch):
    filename = './testcases/linux_libc_test/' + arch + '_bare.txt'
    with codecs.open(filename, 'r', 'utf-8') as f:
        text = f.readlines()
        for i in text:
            if 'OK' in i:
                passed_test.append(i.split()[0])
            if 'FAILED' in i or 'TIMEOUT' in i or 'PARTIAL' in i:
                failed_test.append(i.split()[0])


# 统计通过的syscall
def get_passed_syscall(passed_test, arch):

    passed_syscall = []
    prefix = ''

    if arch == 'x86_64':
        prefix = 'strace -c ../rootfs/x86_64'
    if arch == 'riscv64':
        prefix = 'strace -c ../ignored/target/riscv64/prebuild'
        
    for i in passed_test:
        if arch == 'riscv64':
            i = re.sub("src/", "", i)
        cmd = prefix + i + ' 2>&1'
        result = exec_cmd(cmd)
        signal.signal(signal.SIGALRM, timeout_handler)
        signal.alarm(8)
        try:
            for line in result.readlines():
                s = ['-', 'time', 'src' , 'Segmentation', 'total']
                if any(re.findall('|'.join(s), line)):
                    continue
                for text in line.split():
                    if (re.search('[a-zA-Z]', text)) and text not in passed_syscall:
                        passed_syscall.append(text)
        except TimeOutException as ex:
            print('timeout ', i)

    return passed_syscall


# 统计未通过的syscall
def get_failed_syscall(failed_test, arch):

    failed_syscall = []
    prefix = ''

    if arch == 'x86_64':
        prefix = 'x86_64'
    if arch == 'riscv64':
        prefix = 'riscv64'

    filename = prefix + '_failed_test.log'

    with codecs.open(filename,'w','utf-8') as f1:
        for i in failed_test:
            cmd = 'python3 linux_libc_test.py --arch ' + prefix + ' -t ' + i
            result = exec_cmd(cmd)
            f1.write(result.read())
    
    f = codecs.open(filename,'r','utf-8')
    text = f.readlines()
    for i in text:
        x = re.findall("unknown syscall: [a-zA-Z].*. ", i)
        if x != []:
            tmp = re.sub("unknown syscall: ", "", x[0])
            tmp = re.sub('. ', '', tmp)
            if tmp not in failed_syscall:
                failed_syscall.append(tmp)

    return failed_syscall

def main(args):

    if args.arch == 'x86_64':
        passed_test = []
        failed_test = []
        passed_syscall = []
        failed_syscall = []

        gen_test_list(passed_test, failed_test, 'x86_64')

        passed_syscall = get_passed_syscall(passed_test, 'x86_64')
        with codecs.open('x86_64_passed_syscall.txt', 'w', 'utf-8') as f:
            for i in passed_syscall:
                f.write(i + '\n')

        # failed syscall
        failed_syscall = get_failed_syscall(failed_test, 'x86_64')
        with codecs.open("x86_64_failed_syscall.txt", 'w', 'utf-8') as f:
            for i in failed_syscall:
                f.write(i + '\n')
    
    if args.arch == 'riscv64':
        passed_test = []
        failed_test = []
        passed_syscall = []
        failed_syscall = []

        gen_test_list(passed_test, failed_test, 'riscv64')

        passed_syscall = get_passed_syscall(passed_test, 'riscv64')
        with codecs.open('riscv64_passed_syscall.txt', 'w', 'utf-8') as f:
            for i in passed_syscall:
                f.write(i + '\n')

        failed_syscall = get_failed_syscall(failed_test, 'riscv64')
        with codecs.open("riscv64_failed_syscall.txt", 'w', 'utf-8') as f:
            for i in failed_syscall:
                f.write(i + '\n')
    
if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('--arch', type=str, default='x86_64', help='--arch = x86_64 or riscv64')
    args = parser.parse_args()
    main(args)