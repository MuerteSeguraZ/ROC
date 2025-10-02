@echo off

echo [+] Compiling to test..
gcc roc.c main.c -o roc.exe -lpthread

echo [+] Running test...
roc.exe