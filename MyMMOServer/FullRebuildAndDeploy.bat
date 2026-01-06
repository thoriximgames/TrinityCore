@echo off
REM ========================================
REM MyMMOServer - Full Rebuild and Deploy
REM (Regenerates CMake cache if needed)
REM ========================================
echo.
echo ========================================
echo   Full Rebuild and Deploy
echo ========================================
echo.

REM Stop existing servers first
call "%~dp0Stop_Server.bat"

REM Set paths
set SOURCE_DIR=E:\GIT\MMO-Suite\MMO-Server
set BUILD_DIR=E:\GIT\MMO-Suite\MMO-Server\build
set DEPLOY_DIR=E:\GIT\MMO-Suite\MMO-Server\MyMMOServer\bin
set VCPKG_TOOLCHAIN=E:\GIT\MMO-Suite\vcpkg\scripts\buildsystems\vcpkg.cmake
set CMAKE_PATH=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe

REM Check if cmake exists
if not exist "%CMAKE_PATH%" (
    echo Searching for CMake...

    if exist "C:\Program Files\CMake\bin\cmake.exe" (
        set CMAKE_PATH=C:\Program Files\CMake\bin\cmake.exe
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
        set CMAKE_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
    ) else (
        echo ERROR: CMake not found!
        pause
        exit /b 1
    )
)

echo Using CMake: %CMAKE_PATH%
echo.

REM Check if build directory exists, create if not
if not exist "%BUILD_DIR%" (
    echo Creating build directory...
    mkdir "%BUILD_DIR%"
)

REM Navigate to build directory
cd /d "%BUILD_DIR%"

REM Check if CMake cache exists - if it does but is corrupted, clean it
if exist "CMakeCache.txt" (
    echo Found existing CMake cache. Cleaning for fresh configuration...
    del /F /Q CMakeCache.txt 2>nul
    if exist CMakeFiles rmdir /S /Q CMakeFiles 2>nul
    echo Done!
    echo.
)

echo ========================================
echo Step 1: Generating CMake configuration...
echo ========================================
echo.

"%CMAKE_PATH%" -S "%SOURCE_DIR%" -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_TOOLCHAIN_FILE="%VCPKG_TOOLCHAIN%" -G "Visual Studio 18 2026" -A x64

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

echo ========================================
echo Step 2: Building worldserver...
echo ========================================
echo.

"%CMAKE_PATH%" --build . --config RelWithDebInfo --target worldserver

if errorlevel 1 (
    echo.
    echo ========================================
    echo   BUILD FAILED!
    echo ========================================
    echo.
    pause
    exit /b 1
)

echo.
echo ========================================
echo Step 3: Deploying to MyMMOServer...
echo ========================================
echo.

REM Find the built executable
set BUILT_EXE=%BUILD_DIR%\bin\RelWithDebInfo\worldserver.exe
set BUILT_PDB=%BUILD_DIR%\bin\RelWithDebInfo\worldserver.pdb
set BUILT_DLLS=%BUILD_DIR%\bin\RelWithDebInfo\*.dll

if not exist "%BUILT_EXE%" (
    echo ERROR: Built executable not found at: %BUILT_EXE%
    echo.
    echo Searching for worldserver.exe...
    dir /s /b "%BUILD_DIR%\worldserver.exe" 2>nul
    echo.
    pause
    exit /b 1
)

REM Copy executable
echo Copying worldserver.exe...
copy /Y "%BUILT_EXE%" "%DEPLOY_DIR%\worldserver.exe" >nul
if errorlevel 1 (
    echo ERROR: Failed to copy worldserver.exe
    echo Target: %DEPLOY_DIR%\worldserver.exe
    pause
    exit /b 1
)
echo Done!

REM Copy DLLs
echo Copying DLLs...
copy /Y "%BUILT_DLLS%" "%DEPLOY_DIR%\" >nul
if errorlevel 1 (
    echo WARNING: Failed to copy some DLLs. This might be fine if they haven't changed.
)
echo Done!

REM Copy PDB (debug symbols) if it exists
if exist "%BUILT_PDB%" (
    echo Copying worldserver.pdb...
    copy /Y "%BUILT_PDB%" "%DEPLOY_DIR%\worldserver.pdb" >nul
    echo Done!
)

echo.
echo ========================================
echo   BUILD AND DEPLOY SUCCESSFUL!
echo ========================================
echo.
echo Deployed to: %DEPLOY_DIR%
echo.
echo Movement tracker logging has been added!
echo Look for [MOVEMENT_TRACKER] in server logs.
echo.
echo Next step: Run Start_Server.bat
echo.
pause
