import re
import sys
import argparse
from utils.log import Logger
from utils.test import TestRunner, TestStatus, load_testcases

parser = argparse.ArgumentParser()
parser.add_argument("-l", "--libos", action="store_true", help="test on libos mode")
parser.add_argument("-a", "--arch", choices=["x86_64", "riscv64"], default="x86_64", help="target architecture")
parser.add_argument("-f", "--fast", action="store_true", help="do not test known failed and timeout testcases")
parser.add_argument("-t", "--test", help="run only one test")
args = parser.parse_args()
if args.libos and args.arch != "x86_64":
    raise Exception("target architecture must be 'x86_64' for LibOS mode")

TEST_DIR = "testcases/linux_libc_test"
TEST_NAME = "%s_%s" % (args.arch, "libos" if args.libos else "bare")
TEST_FILE = "%s/%s.txt" % (TEST_DIR, TEST_NAME)
LOG_OUTPUT = "linux_libc_test_%s.log" % TEST_NAME

TIMEOUT = 10
CMDLINE_BASE = "LOG=error:ROOTPROC="
FAILED_PATTERN = [
    "failed",
    "panicked at",
    "ERROR",
]


class LinuxTestRunner(TestRunner):
    BASE_CMD = "cd ../zCore && make MODE=release LINUX=1 TEST=1 ARCH=%s" % args.arch

    def build_cmdline(self) -> str:
        return self.BASE_CMD + (" LIBOS=1" if args.libos else "")

    def run_cmdline(self, name: str) -> str:
        if args.libos:
            return "cd .. && LOG=error ./target/release/zcore %s" % name
        else:
            return self.BASE_CMD + " CMDLINE='%s' justrun" % (CMDLINE_BASE + name)

    def check_output(self, output: str) -> TestStatus:
        if not args.libos:
            output = re.sub("BdsDxe:.*", "", output)
        for pattern in FAILED_PATTERN:
            if pattern in output:
                return TestStatus.FAILED
        return TestStatus.OK


if __name__ == "__main__":
    runner = LinuxTestRunner()
    runner.build()

    if args.test:
        res = runner.run_one(args.test, args.fast, TIMEOUT)
        ok = res == TestStatus.OK
    else:
        runner.set_logger(Logger(LOG_OUTPUT))
        testcases = load_testcases(TEST_FILE)
        ok = runner.run_all(testcases, args.fast, TIMEOUT)

    if not ok:
        sys.exit(-1)
    else:
        sys.exit(0)
