"""
Try to compile using online compiler (godbolt API)
or use MSVC from VS Build Tools if available.
"""
import subprocess
import os
import sys

def try_compile_with_wsl():
    """Try WSL if available."""
    r = subprocess.run(['wsl', '--version'], capture_output=True, text=True)
    if r.returncode == 0:
        print("WSL available:", r.stdout.strip())
        src = '/mnt/f/output/libzen/tests/test_core_verify.cpp'
        inc = '/mnt/f/output/libzen'
        out = '/mnt/f/output/libzen/tests/test_core_verify'
        r2 = subprocess.run(
            ['wsl', 'g++', '-std=c++17', f'-I{inc}', src, '-o', out, '-Wall', '-Wextra'],
            capture_output=True, text=True
        )
        print("Compile STDOUT:", r2.stdout[:2000])
        print("Compile STDERR:", r2.stderr[:2000])
        print("Exit:", r2.returncode)
        if r2.returncode == 0:
            r3 = subprocess.run(['wsl', out], capture_output=True, text=True)
            print("\n=== Test Output ===")
            print(r3.stdout)
            if r3.stderr:
                print("STDERR:", r3.stderr[:500])
        return r2.returncode == 0
    return False

def find_msvc():
    """Search for MSVC cl.exe via vswhere."""
    vswhere_paths = [
        r'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe',
        r'C:\Program Files\Microsoft Visual Studio\Installer\vswhere.exe',
    ]
    for vs in vswhere_paths:
        if os.path.isfile(vs):
            r = subprocess.run([vs, '-latest', '-property', 'installationPath'],
                              capture_output=True, text=True)
            if r.returncode == 0 and r.stdout.strip():
                vs_path = r.stdout.strip()
                # Search for cl.exe
                for root, dirs, files in os.walk(os.path.join(vs_path, 'VC')):
                    if 'cl.exe' in files and 'Hostx64' in root and 'x64' in root:
                        return os.path.join(root, 'cl.exe')
    return None

def try_compile_with_cl(cl_path):
    """Try compiling with MSVC cl.exe."""
    src = r'f:\output\libzen\tests\test_core_verify.cpp'
    out = r'f:\output\libzen\tests\test_core_verify.exe'
    inc = r'f:\output\libzen'
    
    r = subprocess.run(
        [cl_path, '/std:c++17', f'/I{inc}', src, f'/Fe:{out}', '/EHsc', '/nologo'],
        capture_output=True, text=True,
        cwd=r'f:\output\libzen'
    )
    print("STDOUT:", r.stdout[:4000])
    print("STDERR:", r.stderr[:2000])
    print("Exit:", r.returncode)
    if r.returncode == 0:
        r2 = subprocess.run([out], capture_output=True, text=True)
        print("\n=== Test Output ===")
        print(r2.stdout)
    return r.returncode == 0

# Try methods in order
print("=== Method 1: WSL ===")
if try_compile_with_wsl():
    print("Success with WSL!")
    sys.exit(0)

print("\n=== Method 2: MSVC vswhere ===")
cl = find_msvc()
if cl:
    print(f"Found cl.exe: {cl}")
    if try_compile_with_cl(cl):
        print("Success with MSVC!")
        sys.exit(0)
else:
    print("cl.exe not found via vswhere")

print("\n=== No compiler available ===")
