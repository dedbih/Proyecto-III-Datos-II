@echo off
setlocal enabledelayedexpansion

:: Run as administrator
NET FILE >nul 2>&1 || (
    echo Requesting administrator privileges...
    powershell -command "Start-Process -FilePath '%~0' -Verb RunAs"
    exit /b
)

:: Define nodes array properly for paths with spaces
set "node1=C:\Users\lasle\Desktop\Datos II\Proyecto III\storage\node1"
set "node2=C:\Users\lasle\Desktop\Datos II\Proyecto III\storage\node2"
set "node3=C:\Users\lasle\Desktop\Datos II\Proyecto III\storage\node3"
set "node4=C:\Users\lasle\Desktop\Datos II\Proyecto III\storage\node4"

:: Loop through each node
for /l %%i in (1,1,4) do (
    echo.
    echo ===== Cleaning node%%i =====
    echo Path: !node%%i!

    :: Take ownership and grant full permissions
    echo Taking ownership...
    takeown /f "!node%%i!" /r /d y >nul
    icacls "!node%%i!" /grant administrators:F /t /c >nul

    :: Delete all contents
    echo Deleting files...
    del /f /q /s "!node%%i!\*" >nul 2>&1

    :: Force remove any remaining files/directories
    echo Force removing any remaining items...
    powershell -command "Remove-Item -LiteralPath '!node%%i!\*' -Force -Recurse -ErrorAction SilentlyContinue"

    :: Verify
    dir /a "!node%%i!" | find "File(s)" >nul
    if errorlevel 1 (
        echo  Successfully cleaned node%%i
    ) else (
        echo  Files remain in node%%i
        dir /a "!node%%i!"
    )
)

echo.
echo ===== Final Verification =====
for /l %%i in (1,1,4) do (
    dir /a "!node%%i!" 2>nul | find "File(s)" >nul && (
        echo  Files remain in node%%i
    ) || (
        echo ✔ node%%i is clean
    )
)

pause