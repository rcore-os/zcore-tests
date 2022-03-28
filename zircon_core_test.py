import sys
import argparse
from utils.log import Logger
from utils.test import TestRunner, TestStatus, load_testcases

parser = argparse.ArgumentParser()
parser.add_argument("-l", "--libos", action="store_true", help="test on libos mode (otherwise bare-metal mode)")
parser.add_argument("-f", "--fast", action="store_true", help="do not test known failed and timeout testcases")
parser.add_argument("-t", "--test", help="run only one test")
parser.add_argument("--no-failed", action="store_true", help="exit with calling exit(0), never call exit(-1)")
args = parser.parse_args()


ZBI_PATH = "../prebuilt/zircon/x64/core-tests.zbi"
TEST_DIR = "testcases/zircon_core_test"
TEST_NAME = "%s_%s" % ("x86_64", "libos" if args.libos else "bare")
TEST_FILE = "%s/%s.txt" % (TEST_DIR, TEST_NAME)
LOG_OUTPUT = "zircon_core_test_%s.log" % TEST_NAME

TIMEOUT = 10
CMDLINE_BASE = "LOG=error:userboot=test/core-standalone-test:userboot.shutdown:core-tests="
OK_PATTERN = [
    "[       OK ]",
    "[  PASSED  ]",
    "userboot: finished!",
]
FAILED_PATTERN = [
    "[  FAILED  ]",
    "ERROR",
]


class ZirconTestRunner(TestRunner):
    BASE_CMD = "cd ../zCore && make MODE=release ZBI=core-tests TEST=1"

    def build_cmdline(self) -> str:
        return self.BASE_CMD + (" LIBOS=1" if args.libos else "")

    def run_cmdline(self, name: str) -> str:
        if args.libos:
            return "../target/release/zcore %s '%s'" % (ZBI_PATH, CMDLINE_BASE + name)
        else:
            return self.BASE_CMD + " CMDLINE='%s' justrun" % (CMDLINE_BASE + name)

    def check_output(self, output: str) -> TestStatus:
        for pattern in FAILED_PATTERN:
            if pattern in output:
                return TestStatus.FAILED
        for pattern in OK_PATTERN:
            if not pattern in output:
                return TestStatus.FAILED
        return TestStatus.OK


if __name__ == "__main__":
    runner = ZirconTestRunner()
    runner.build()

    if args.test:
        res = runner.run_one(args.test, args.fast, TIMEOUT)
        ok = res == TestStatus.OK
    else:
        runner.set_logger(Logger(LOG_OUTPUT))
        testcases = load_testcases(TEST_FILE)
        ok = runner.run_all(testcases, args.fast, TIMEOUT)

    if not ok and not args.no_failed:
        sys.exit(-1)
    else:
        sys.exit(0)
