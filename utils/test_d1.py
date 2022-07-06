from __future__ import annotations

import time
import subprocess
from termcolor import colored
from typing import List, Tuple
from enum import Enum
from .log_d1 import Logger
import serial
import sys
import re

class TestStatus(Enum):
    OK = "OK"
    FAILED = "FAILED"
    TIMEOUT = "TIMEOUT"
    PARTIAL = "PARTIAL"
    SKIPPED = "SKIPPED"

    def from_str(s: str) -> TestStatus:
        if s == "OK":
            return TestStatus.OK
        elif s == "TIMEOUT":
            return TestStatus.TIMEOUT
        elif s == "FAILED":
            return TestStatus.FAILED
        elif s == "PARTIAL":
            return TestStatus.PARTIAL
        elif s == "SKIPPED":
            return TestStatus.SKIPPED
        else:
            raise

    def color(self):
        if self == TestStatus.OK:
            return "green"
        elif self == TestStatus.TIMEOUT:
            return "yellow"
        elif self == TestStatus.FAILED:
            return "red"
        elif self == TestStatus.PARTIAL:
            return "cyan"
        elif self == TestStatus.SKIPPED:
            return None
        else:
            raise

    def colored_name(self):
        return colored(self.name, self.color())


def load_testcases(filename: str) -> List[Tuple[str, TestStatus]]:
    cases = []
    with open(filename, "r") as f:
        for line in f.readlines():
            if line.startswith("#"):  # comments
                continue
            line = line.strip().split()
            if len(line) == 2:
                name, status = line
                cases.append((name, TestStatus.from_str(status)))
    return sorted(cases)

class TestRunner(object):
    def __init__(self, device):
        self.ser = serial.Serial(device,115200,timeout=3600)
        if self.ser.isOpen():
            print("open succeed >",self.ser.name)
        else:
            print("open failed >",self.ser.name)
            sys.exit(-1)

    def __del__(self):
        self.teardown()

    def build_cmdline(self):
        return None

    def run_cmdline(self):
        raise NotImplementedError

    def check_output(self, output: str) -> TestStatus:
        return TestStatus.OK

    def teardown(self):
        pass

    def set_logger(self, logger: Logger):
        self.logger = logger

    def build(self):
        self.logger.println("======== Build zcore ========")
        cmdline = self.build_cmdline()
        if cmdline is not None:
            self.logger.println(cmdline)
            subprocess.run(cmdline, shell=True, check=True)

    def burn(self):
        self.logger.println("======== Burn zcore =========")
        cmdline = self.build_cmdline()
        if cmdline is not None:
            self.logger.println(cmdline)
            subprocess.run(cmdline, shell=True, check=True)

    def run_one(self, name: str, fast=False, timeout=None) -> TestStatus:
        cmdline = name + '\n'
        self.ser.write(cmdline.encode())
        time_begin = time.time()

        while True:
            output = self.ser.readline()
            if re.search(r"/ # [/r/n]", output):
                break
            if time.time() - time_begin > timeout:
                status = TestStatus.TIMEOUT
                break
        time_end = time.time()

        if status != TestStatus.TIMEOUT:
            status = self.check_output(output)

        if status != TestStatus.OK or not fast:
            self.logger.println(colored(output, "magenta"))
        else:
            self.logger.println_file_only(output)
        self.logger.println("  %s (%.3fs)\n" % (status.colored_name(), time_end - time_begin))
        time.sleep(0.1)

        return status

    def run_all(self, testcases: List[Tuple[str, TestStatus]], fast=False, timeout=None) -> bool:
        self.logger.println("======== Run %d testcases ========" % len(testcases))
        result = []
        failed = False
        for (i, (name, expected_status)) in enumerate(sorted(testcases)):
            ignore_on_fast = expected_status in [TestStatus.TIMEOUT, TestStatus.FAILED, TestStatus.SKIPPED]
            self.logger.println("Test %d: %s" % (i, name if not ignore_on_fast else name + " (ignored)"))
            if fast and ignore_on_fast:
                self.logger.println("  SKIPPED\n")
                result.append((name, TestStatus.SKIPPED))
                continue

            actual_status = self.run_one(name, fast, timeout)
            result.append((name, actual_status))
            if actual_status != expected_status and expected_status == TestStatus.OK:
                failed = True

        self.print_result(testcases, result)
        return not failed

    def print_result(self, testcases: List[Tuple[str, TestStatus]], result: List[Tuple[str, TestStatus]]):
        passed_extra = []
        failed = []
        partial = []
        for ((name, expected_status), (_, actual_status)) in zip(testcases, result):
            t = (name, expected_status, actual_status)
            if actual_status != expected_status:
                if expected_status == TestStatus.OK:
                    failed.append(t)
                elif expected_status == TestStatus.PARTIAL:
                    partial.append(t)
                elif actual_status == TestStatus.OK:
                    passed_extra.append(t)

        status_list = list(map(lambda t: t[1], result))
        skipped_count = status_list.count(TestStatus.SKIPPED)
        passed_count = status_list.count(TestStatus.OK)
        failed_count = status_list.count(TestStatus.FAILED)
        timeout_count = status_list.count(TestStatus.TIMEOUT)
        self.logger.println("======== Test result ========")
        self.logger.println("%s    %d" % ("ALL     ", len(testcases)))
        self.logger.println("%s    %d" % ("SKIPPED ", skipped_count))
        self.logger.println("%s    %d" % (colored("PASSED  ", "green"), passed_count))
        self.logger.println("%s    %d" % (colored("FAILED  ", "red"), failed_count))
        self.logger.println("%s    %d" % (colored("TIMEOUT ", "yellow"), timeout_count))
        self.logger.println()

        def print_expected_actual_status(cases: List[Tuple[str, str, str]]):
            if len(cases) == 0:
                self.logger.println()
                return
            lines = []
            max_len = max(map(lambda t: len(t[0]), cases))
            max_len = ((max_len + 8) // 4) * 4
            for (name, expected, actual) in cases:
                lines.append("%s%s%s -> %s" % (name, ' ' * (max_len - len(name)),
                                               expected.name, actual.colored_name()))
            self.logger.print_list(lines, indent=2)

        self.logger.println(colored("======== Failed cases (not ignored): %d ========" % len(failed), "red"))
        print_expected_actual_status(failed)
        self.logger.println(colored("======== Passed extra cases: %d ========" % len(passed_extra), "green"))
        print_expected_actual_status(passed_extra)
        self.logger.println(colored("======== Partially passed cases: %d ========" % len(partial), "cyan"))
        print_expected_actual_status(partial)

        self.logger.println_file_only("======== All cases: %d ========" % len(testcases))
        self.logger.print_list_file_only(
            list(map(lambda t: t[0] + " " * max(80 - len(t[0]), 1) + t[1].name, result)), indent=2)
