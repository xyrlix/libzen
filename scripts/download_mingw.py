"""
Download a portable MinGW g++ (winlibs) and compile the test.
"""
import os
import sys
import urllib.request
import zipfile
import subprocess

DEST_DIR = r'C:\tools\mingw64'
WINLIBS_URL = "https://github.com/brechtsanders/winlibs_mingw/releases/download/15.1.0posix-19.1.7-12.0.0-ucrt-r2/winlibs-x86_64-posix-seh-gcc-15.1.0-mingw-w64ucrt-12.0.0-r2.zip"
# Fallback smaller URL
WINLIBS_URL_SMALL = "https://winlibs.com/tdm/gcc-4.9.2-tdm64-1.zip"

# Use a known working release
WINLIBS_RELEASE = "https://github.com/brechtsanders/winlibs_mingw/releases/download/14.2.0posix-19.1.1-12.0.0-ucrt-r2/winlibs-x86_64-posix-seh-gcc-14.2.0-mingw-w64ucrt-12.0.0-r2.zip"

ZIP_PATH = r'C:\tools\mingw64.zip'

def download_with_progress(url, dest):
    print(f"Downloading {url} -> {dest}")
    os.makedirs(os.path.dirname(dest), exist_ok=True)
    
    def progress(count, block_size, total_size):
        if total_size > 0:
            pct = count * block_size * 100 // total_size
            print(f"\r  {pct}%", end='', flush=True)
    
    urllib.request.urlretrieve(url, dest, reporthook=progress)
    print()

# Check if already downloaded
gpp = os.path.join(DEST_DIR, 'bin', 'g++.exe')
if os.path.isfile(gpp):
    print(f"g++ already at {gpp}")
else:
    if not os.path.isfile(ZIP_PATH):
        download_with_progress(WINLIBS_RELEASE, ZIP_PATH)
    
    print(f"Extracting to C:\\tools\\...")
    os.makedirs(r'C:\tools', exist_ok=True)
    with zipfile.ZipFile(ZIP_PATH, 'r') as z:
        # Only extract the first 100 files (bin/ dir)
        names = [n for n in z.namelist() if 'bin/' in n.lower()][:200]
        for name in names:
            z.extract(name, r'C:\tools')
    
    # Find extracted dir
    for d in os.listdir(r'C:\tools'):
        if 'mingw' in d.lower() or 'gcc' in d.lower():
            extracted = os.path.join(r'C:\tools', d)
            print(f"Extracted to: {extracted}")
            break

# Try compile
if os.path.isfile(gpp):
    print(f"\nCompiling with {gpp}...")
    src  = r'f:\output\libzen\tests\test_core_verify.cpp'
    out  = r'f:\output\libzen\tests\test_core_verify.exe'
    inc  = r'f:\output\libzen'
    
    r = subprocess.run(
        [gpp, '-std=c++17', f'-I{inc}', src, '-o', out, '-Wall'],
        capture_output=True, text=True
    )
    print("STDOUT:", r.stdout[:4000])
    print("STDERR:", r.stderr[:4000])
    print("Exit:", r.returncode)
    
    if r.returncode == 0:
        print("\n=== Running tests ===")
        r2 = subprocess.run([out], capture_output=True, text=True)
        print(r2.stdout)
        if r2.stderr:
            print("STDERR:", r2.stderr)
else:
    print(f"g++ not found at {gpp}")
    print("Available in C:\\tools:")
    if os.path.isdir(r'C:\tools'):
        for item in os.listdir(r'C:\tools'):
            print(f"  {item}")
