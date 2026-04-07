import os
import subprocess

# Run cmd where
r = subprocess.run('where g++', shell=True, capture_output=True, text=True)
print("where g++:", r.stdout, r.stderr)
r = subprocess.run('where cl', shell=True, capture_output=True, text=True)
print("where cl:", r.stdout, r.stderr)
r = subprocess.run('where clang++', shell=True, capture_output=True, text=True)
print("where clang++:", r.stdout, r.stderr)

# Also check some known paths
known = [
    r'C:\ProgramData\chocolatey\bin\g++.exe',
    r'C:\ProgramData\chocolatey\lib\mingw\tools\install\mingw64\bin\g++.exe',
    r'C:\tools\mingw64\bin\g++.exe',
    r'C:\msys2\mingw64\bin\g++.exe',
    r'C:\msys64\mingw64\bin\g++.exe',
    r'D:\msys64\mingw64\bin\g++.exe',
]
for p in known:
    if os.path.isfile(p):
        print(f"FOUND: {p}")
    else:
        print(f"NOT found: {p}")
