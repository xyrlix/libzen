import subprocess
import os
import sys

# Print env PATH
path_dirs = os.environ.get('PATH', '').split(os.pathsep)
print('PATH dirs count:', len(path_dirs))

# Common compiler locations
search_paths = path_dirs + [
    r'D:\msys64\mingw64\bin',
    r'D:\msys64\usr\bin',
    r'C:\mingw64\bin',
    r'C:\msys64\mingw64\bin',
    r'D:\Program Files (x86)\studiovs\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64',
]

found = []
for d in search_paths:
    d = d.strip().strip('"')
    if not d:
        continue
    for name in ['g++.exe', 'gcc.exe', 'cl.exe', 'clang++.exe']:
        p = os.path.join(d, name)
        try:
            if os.path.isfile(p):
                found.append(p)
                break
        except Exception:
            pass

print('Found compilers:')
for f in found:
    print(' ', f)

# Try compile with first g++ found
gpps = [p for p in found if 'g++' in p.lower()]
cls_ = [p for p in found if 'cl.exe' in p.lower() and 'clang' not in p.lower()]

src  = r'f:\output\libzen\tests\test_core_verify.cpp'
out  = r'f:\output\libzen\tests\test_core_verify.exe'
inc  = r'f:\output\libzen'

if gpps:
    gpp = gpps[0]
    print(f'\n=== Compiling with g++: {gpp} ===')
    r = subprocess.run(
        [gpp, '-std=c++17', f'-I{inc}', src, '-o', out, '-Wall', '-Wextra'],
        capture_output=True, text=True
    )
    print('STDOUT:', r.stdout[:3000])
    print('STDERR:', r.stderr[:3000])
    print('Exit code:', r.returncode)
    if r.returncode == 0:
        print('\n=== Running ===')
        r2 = subprocess.run([out], capture_output=True, text=True)
        print(r2.stdout)
        print(r2.stderr)
elif cls_:
    cl = cls_[0]
    print(f'\n=== Compiling with cl.exe: {cl} ===')
    r = subprocess.run(
        [cl, '/std:c++17', f'/I{inc}', src, f'/Fe:{out}', '/EHsc'],
        capture_output=True, text=True
    )
    print('STDOUT:', r.stdout[:3000])
    print('STDERR:', r.stderr[:3000])
    print('Exit code:', r.returncode)
    if r.returncode == 0:
        print('\n=== Running ===')
        r2 = subprocess.run([out], capture_output=True, text=True)
        print(r2.stdout)
        print(r2.stderr)
else:
    print('No compiler found!')
    # Dump full PATH for debugging
    print('\nFull PATH:')
    for d in path_dirs:
        print(' ', repr(d))
