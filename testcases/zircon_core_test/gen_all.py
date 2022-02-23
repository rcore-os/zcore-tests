import subprocess

CMD = "cd ../../../zCore && make run MODE=release ZBI=core-tests TEST=1 LIBOS=1 CMDLINE='userboot=test/core-standalone-test:userboot.shutdown:core-tests=-l'"

if __name__ == '__main__':
    p = subprocess.run(CMD, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output = p.stdout.decode()
    orig = output.splitlines()

    testcases = []
    start = False
    for line in orig:
        line = line.strip()
        if "Running standalone Zircon core tests" in line:
            start = True
            continue

        if start:
            if line[0] != '.':
                prefix = line
            else:
                print(prefix + line)
                testcases.append(prefix + line)

    with open("x86_64_bare.txt", "w") as f:
        f.write("\n".join(sorted(testcases)) + "\n")
