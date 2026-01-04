@echo off
TITLE My MMO - Stop Services
COLOR 0C

echo =====================================================
echo       STOPPING ALL SERVER SERVICES (3.3.5)
echo =====================================================

echo [1/3] Closing World Server...
taskkill /f /im worldserver.exe /t 2>nul

echo [2/3] Closing Auth Server...
taskkill /f /im authserver.exe /t 2>nul

echo [3/3] Closing Database...
taskkill /f /im mariadbd.exe /t 2>nul

:: Cleanup lock file if it persists
if exist "mysql\data\mysqld.pid" del /f "mysql\data\mysqld.pid"

echo.
echo =====================================================
echo       ALL SERVICES STOPPED.
echo =====================================================
timeout /t 3
