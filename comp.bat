@echo off
del roc.exe

echo [+] Compiling to test..
gcc roc.c main.c -o roc.exe -lpthread