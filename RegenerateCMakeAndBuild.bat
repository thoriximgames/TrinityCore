@echo off
REM ========================================
REM MMO Server - Regenerate CMake & Build
REM Use this if BuildServer.bat fails with CMake cache errors
REM ========================================
echo.
echo ========================================
echo   Regenerating CMake Cache and Building
echo ========================================
echo.

REM Set paths
set BUILD_DIR=E:\GIT\MMO-Suite\MMO-Server\build
set SOURCE_DIR=E:\GIT\MMO-Suite\MMO-Server
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

REM Delete old CMake cache
echo Step 1: Removing old CMake cache...
if exist CMakeCache.txt del /F /Q CMakeCache.txt
if exist CMakeFiles rmdir /S /Q CMakeFiles
echo Done!
echo.

REM Regenerate CMake files
echo Step 2: Regenerating CMake build files...
"%CMAKE_PATH%" -S "%SOURCE_DIR%" -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=RelWithDebInfo -G "Visual Studio 18 2022" -A x64

if errorlevel 1 (
    echo.
    echo ========================================
    echo   CMAKE GENERATION FAILED!
    echo ========================================
    echo.
    pause
    exit /b 1
)
echo Done!
echo.

REM Build the server
echo Step 3: Building worldserver...
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
