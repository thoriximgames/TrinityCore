@echo off
REM ========================================
REM MyMMOServer - Quick Incremental Build
REM Use this after small code changes
REM ========================================
echo.
echo ========================================
echo   Quick Incremental Build
echo ========================================
echo.

set BUILD_DIR=E:\GIT\MMO-Suite\MMO-Server\build
set DEPLOY_DIR=E:\GIT\MMO-Suite\MMO-Server\MyMMOServer\bin
set CMAKE_PATH=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe

cd /d "%BUILD_DIR%"

echo Building only changed files...
echo.

"%CMAKE_PATH%" --build . --config RelWithDebInfo --target worldserver

if errorlevel 1 (
    echo.
    echo BUILD FAILED!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Deploying to MyMMOServer...
echo ========================================

set BUILT_EXE=%BUILD_DIR%\bin\RelWithDebInfo\worldserver.exe
set BUILT_PDB=%BUILD_DIR%\bin\RelWithDebInfo\worldserver.pdb

if not exist "%BUILT_EXE%" (
    echo ERROR: Built worldserver.exe not found!
    echo Expected at: %BUILT_EXE%
    pause
    exit /b 1
)

echo Copying worldserver.exe...
copy /Y "%BUILT_EXE%" "%DEPLOY_DIR%\worldserver.exe"
if errorlevel 1 (
    echo ERROR: Failed to copy worldserver.exe
    pause
    exit /b 1
)
echo Done!

if exist "%BUILT_PDB%" (
    echo Copying worldserver.pdb...
    copy /Y "%BUILT_PDB%" "%DEPLOY_DIR%\worldserver.pdb"
    echo Done!
)

echo.
echo Verifying deployment...
dir "%DEPLOY_DIR%\worldserver.exe" | findstr /C:"worldserver.exe"

echo.
echo ========================================
echo   BUILD AND DEPLOY SUCCESSFUL!
echo ========================================
echo.
echo Only changed files were recompiled.
echo New worldserver.exe is ready!
echo.
echo Next: Restart server with Start_Server.bat
echo.
pause
