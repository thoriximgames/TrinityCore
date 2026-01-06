@echo off
SETLOCAL EnableDelayedExpansion
TITLE My MMO - One Click Starter
COLOR 0A

:: Root Path setup
SET "SERVER_ROOT=%~dp0"
SET "BIN_DIR=%SERVER_ROOT%bin"
SET "DB_DIR=%SERVER_ROOT%mysql"

echo =====================================================
echo       STARTING MY MMO SERVER (3.3.5)
echo =====================================================

:: 1. Start Database
echo [1/3] Starting Database Service...
if exist "%DB_DIR%\data\mysqld.pid" del /f "%DB_DIR%\data\mysqld.pid" >nul 2>&1

:: Change directory to DB_DIR so relative paths in my.ini work
cd /d "%DB_DIR%"
start "MMO-Database" "bin\mariadbd.exe" --defaults-file="my.ini" --console

:: 2. Wait for DB to be ready
echo Waiting for Database to initialize on port 3306...
:wait_db
netstat -ano | findstr ":3306" | findstr "LISTENING" >nul
if !errorlevel! neq 0 (
    timeout /t 1 /nobreak >nul
    goto wait_db
)

:: Check if it's the RIGHT database (optional but helpful)
tasklist /FI "IMAGENAME eq mariadbd.exe" | findstr "mariadbd.exe" >nul
if !errorlevel! neq 0 (
    echo WARNING: Port 3306 is in use, but NOT by mariadbd.exe.
    echo Please stop your local MySQL service!
    pause
)

echo Database is Online!

:: 3. Start Auth
echo [2/3] Starting Login Service...
cd /d "%BIN_DIR%"
:: Use cmd /c so the window stays open if it crashes
start "MMO-Login" cmd /c "authserver.exe || pause"

:: 4. Start World
echo [3/3] Starting World Service...
start "MMO-World" cmd /c "worldserver.exe || pause"

echo.
echo =====================================================
echo       MMORPG ENGINE IS NOW LIVE!
echo =====================================================
echo This window will close in 5 seconds. 
echo The Server windows will remain open.
timeout /t 5