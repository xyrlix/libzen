import subprocess, sys

cmd = 'wsl g++ -std=c++17 -I/mnt/f/output/libzen /mnt/f/output/libzen/tests/test_core_verify.cpp -o /mnt/f/output/libzen/tests/test_core_verify -Wall -Wextra -Wpedantic 2>&1'
r = subprocess.run(cmd, shell=True, capture_output=True)
stdout = r.stdout.decode('utf-8', errors='replace')
stderr = r.stderr.decode('utf-8', errors='replace')
print("STDOUT:", stdout[:8000])
print("STDERR:", stderr[:2000])
print("Exit:", r.returncode)

if r.returncode == 0:
    print("\n=== Running tests ===")
    r2 = subprocess.run('wsl /mnt/f/output/libzen/tests/test_core_verify', shell=True, capture_output=True)
    print(r2.stdout.decode('utf-8', errors='replace'))
