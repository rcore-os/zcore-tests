import re
from typing import List


def colorless(text: str) -> str:
    return re.sub("\x1b\[\d*m", "", text)


class Logger(object):
    def __init__(self, log_file_name: str = None):
        if log_file_name is not None:
            self.file = open(log_file_name, "w")
        else:
            self.file = None

    def __del__(self):
        if self.file is not None:
            self.file.close()

    def print(self, message: str):
        print(message, end="")
        if self.file:
            self.file.write(colorless(message))

    def println(self, message: str = ""):
        self.print(message + "\n")

    def print_list(self, list: List[str], indent=0):
        for item in sorted(list):
            self.println(" " * indent + item)
        self.println()

    def println_file_only(self, message: str):
        if self.file:
            self.file.write(colorless(message) + "\n")

    def print_list_file_only(self, list: List[str], indent=0):
        if self.file:
            for item in sorted(list):
                self.file.write(" " * indent + colorless(item) + "\n")
            self.file.write("\n")
