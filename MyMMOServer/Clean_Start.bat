@echo off
TITLE My MMO - Clean Start Tool
COLOR 0B

SET "SERVER_ROOT=%~dp0"

echo =====================================================
echo       CLEAN START: PURGING LOGS AND CACHE
echo =====================================================

:: 1. Force Stop Everything
echo [1/4] Stopping all current processes...
taskkill /f /im authserver.exe /t 2>nul
taskkill /f /im worldserver.exe /t 2>nul
taskkill /f /im mariadbd.exe /t 2>nul
timeout /t 2 /nobreak >nul

:: 2. Clean Logs
echo [2/4] Clearing Log Files...
if exist "%SERVER_ROOT%bin\*.log" del /q "%SERVER_ROOT%bin\*.log"
if exist "%SERVER_ROOT%bin\Crashes" rd /s /q "%SERVER_ROOT%bin\Crashes"

:: 3. Clean World Server Cache
echo [3/4] Clearing Server Cache...
if exist "%SERVER_ROOT%bin\Cache" rd /s /q "%SERVER_ROOT%bin\Cache"

:: 4. Start Everything
echo [4/4] Handing over to Starter...
echo.
call "%SERVER_ROOT%Start_Server.bat"
