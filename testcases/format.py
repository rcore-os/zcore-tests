import sys

if __name__ == '__main__':
    fname = sys.argv[1]
    width = sys.argv[2] or 80
    with open(fname, "r") as f:
        data = f.readlines()

    output = []
    for line in data:
        arr = line.split()
        if len(arr) != 2:
            raise Exception("format error")
        if len(arr[0]) >= int(width):
            raise Exception("width %s is too small" % width)
        # if "libos" in fname and arr[0].endswith("-static.exe"):
        #     continue
        format = "%%-%ss%%s" % width
        output.append(format % (arr[0], arr[1]))

    with open(fname, "w") as f:
        f.write("\n".join(sorted(output)) + "\n")
