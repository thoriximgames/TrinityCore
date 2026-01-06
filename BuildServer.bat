@echo off
REM ========================================
REM MMO Server Build Script
REM ========================================
echo.
echo ========================================
echo   Building MMO Server (worldserver)
echo ========================================
echo.

REM Set paths
set BUILD_DIR=E:\GIT\MMO-Suite\MMO-Server\build
set CMAKE_PATH=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe

REM Check if cmake exists
if not exist "%CMAKE_PATH%" (
    echo ERROR: CMake not found at: %CMAKE_PATH%
    echo.
    echo Please update CMAKE_PATH in this script to point to your cmake.exe
    pause
    exit /b 1
)

REM Navigate to build directory
cd /d "%BUILD_DIR%"
if errorlevel 1 (
    echo ERROR: Build directory not found: %BUILD_DIR%
    pause
    exit /b 1
)

echo Building worldserver in RelWithDebInfo configuration...
echo.

REM Build the server
"%CMAKE_PATH%" --build . --config RelWithDebInfo --target worldserver

if errorlevel 1 (
    echo.
    echo ========================================
    echo   BUILD FAILED!
    echo ========================================
    echo.
    pause
    exit /b 1
) else (
    echo.
    echo ========================================
    echo   BUILD SUCCESSFUL!
    echo ========================================
    echo.
    echo Executable location:
    echo %BUILD_DIR%\bin\RelWithDebInfo\worldserver.exe
    echo.
    pause
)
