@echo off
setlocal enabledelayedexpansion

set "project_root=%~dp0"
set "python_script=%project_root%python\disknode.py"

for %%i in (1 2 3 4) do (
    start "Disk Node %%i" cmd /k "cd /d "%project_root%" && python "%python_script%" "disk_config\node%%i.xml""
)
echo All disk nodes started. Check individual windows for logs.