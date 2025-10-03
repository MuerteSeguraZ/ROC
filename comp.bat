@echo off
setlocal

if exist roc.exe del roc.exe

echo [+] Compiling ROC project...

gcc roc.c roc_task.c roc_scheduler.c main.c -o roc.exe -lpthread
if errorlevel 1 (
    echo [!] Compilation failed. Check for errors.
    exit /b 1
)

echo [+] Compilation successful! Executable: roc.exe
endlocal
