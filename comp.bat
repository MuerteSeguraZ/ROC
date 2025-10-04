@echo off
setlocal

if exist roc.exe del roc.exe

echo [+] Compiling ROC project...

:: compila tot a dins del folder de src
gcc -Iinclude src\*.c src\hw\*.c -o roc.exe -lpthread
if errorlevel 1 (
    echo [!] Compilation failed. Check for errors.
    exit /b 1
)

echo [+] Compilation successful! Executable: roc.exe
endlocal
