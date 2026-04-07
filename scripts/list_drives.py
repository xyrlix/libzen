import os, sys

print("CWD:", os.getcwd())
print("Python:", sys.executable)

# List drives
import string
drives = []
for c in string.ascii_uppercase:
    d = c + ":\\"
    if os.path.isdir(d):
        drives.append(d)
print("Available drives:", drives)

# Check common compiler paths
paths_to_check = [
    r'D:\msys64',
    r'D:\msys64\mingw64\bin',
    r'D:\msys64\mingw64\bin\g++.exe',
    r'C:\msys64',
    r'C:\MinGW',
    r'C:\mingw64',
    r'D:\Program Files (x86)\studiovs\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\cl.exe',
]
for p in paths_to_check:
    print(f"  exists({p}): {os.path.exists(p)}")
