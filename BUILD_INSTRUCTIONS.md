# MMO Server Build Instructions

## Quick Build

### Option 1: Normal Build (Fastest)

Double-click on:
```
BuildServer.bat
```

This will rebuild just the `worldserver` executable using your existing CMake configuration.

**Use this for:** Regular rebuilds after code changes

---

### Option 2: Full Regenerate + Build

If you encounter CMake cache errors, double-click on:
```
RegenerateCMakeAndBuild.bat
```

This will:
1. Delete old CMake cache
2. Regenerate CMake build files
3. Build the worldserver

**Use this for:**
- First time building
- After moving the project directory
- When you get CMake cache errors
- When CMake says "directory is different than the directory where CMakeCache.txt was created"

---

## Manual Build (Advanced)

### Using CMake Command Line

```cmd
cd E:\GIT\MMO-Suite\MMO-Server\build
cmake --build . --config RelWithDebInfo --target worldserver
```

### Using Visual Studio

1. Open `E:\GIT\MMO-Suite\MMO-Server\build\TrinityCore.sln` in Visual Studio
2. Set configuration to **RelWithDebInfo**
3. Right-click **worldserver** project → Build

---

## After Building

The compiled executable will be at:
```
E:\GIT\MMO-Suite\MMO-Server\build\bin\RelWithDebInfo\worldserver.exe
```

You may need to copy it to your server directory:
```
E:\GIT\MMO-Suite\MMO-Server\MyMMOServer\bin\worldserver.exe
```

Or use the existing server start scripts which reference the correct location.

---

## Troubleshooting

### "CMake Error: The current CMakeCache.txt directory is different"

**Solution:** Run `RegenerateCMakeAndBuild.bat`

### "CMake not found"

**Solution:** Edit `BuildServer.bat` and `RegenerateCMakeAndBuild.bat` to update the `CMAKE_PATH` variable to point to your cmake.exe location.

Common locations:
- `C:\Program Files\CMake\bin\cmake.exe`
- `C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`
- `C:\Program Files (x86)\CMake\bin\cmake.exe`

### "Build directory not found"

**Solution:** Make sure you're running the batch file from the correct location and the paths in the script are correct for your setup.

---

## Build Configurations

- **Debug** - Full debug symbols, no optimization (slow, large files)
- **Release** - Full optimization, no debug symbols (fast, small files)
- **RelWithDebInfo** - Optimized code with debug symbols (recommended for development)
- **MinSizeRel** - Optimized for smallest binary size

The default scripts use **RelWithDebInfo** which is the best balance for development.
