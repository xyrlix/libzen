import subprocess
r = subprocess.run('wsl g++ --version', shell=True, capture_output=True, text=True)
print(r.stdout[:200])
print(r.returncode)
