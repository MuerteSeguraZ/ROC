@echo off
REM ================================
REM Publish script for ROC project
REM ================================

REM Change to script directory
cd /d "%~dp0"

REM Ensure git is initialized
if not exist ".git" (
    echo Initializing new git repository...
    git init
    git branch -M main
)

REM Add all changes
git add .

REM Commit with timestamp message
for /f "tokens=1-3 delims=/ " %%a in ("%date%") do (
    for /f "tokens=1-2 delims=:." %%x in ("%time%") do (
        set commitmsg=Update %%c-%%a-%%b %%x:%%y
    )
)
git commit -m "%commitmsg%"

REM Push to remote
git push origin main

echo.
echo ================================
echo   ✅ Published to GitHub
echo ================================
pause
