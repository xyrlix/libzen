import subprocess, os

# WSL
r = subprocess.run('wsl --version', shell=True, capture_output=True, text=True)
print("WSL check:", r.returncode, r.stdout[:200], r.stderr[:200])

# vswhere
vswhere = r'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
print("vswhere exists:", os.path.isfile(vswhere))

# cl from PATH
r2 = subprocess.run('where cl', shell=True, capture_output=True, text=True)
print("where cl:", r2.stdout, r2.stderr)

# try direct wsl g++
r3 = subprocess.run('wsl g++ --version', shell=True, capture_output=True, text=True)
print("wsl g++:", r3.returncode, r3.stdout[:200])
