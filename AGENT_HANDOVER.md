# Agent Handover: TrinityCore Blank MMO Transformation

## Project Objective
Transform a standard TrinityCore repository into a "Blank MMO" backend by stripping Blizzard-specific content (scripts, world data) while retaining core engine features (networking, authentication, spell engine, physics).

## Current Status
**Status:** Compilation Complete. Database Initialized. Awaiting Client Data Files.

## Technical Environment (win32)
- **Compiler:** MSVC 19.50 (Visual Studio 2026).
- **Package Manager:** `vcpkg` (Standalone, located at `E:\GIT\vcpkg`).
- **Dependencies:** 
    - **Boost 1.90.0** (Dynamic, `x64-windows` triplet).
    - **OpenSSL 3.6.0** (Dynamic, `x64-windows` triplet).
    - **MySQL 8.4.6** (Community Server).

## Build Configuration
- **Build Directory:** `E:\GIT\TrinityCore\build`
- **CMake Preset:** 
    - `-DSCRIPTS=minimal-static` (Removes all quest/zone scripts; keeps Commands and Spells).
    - `-DUSE_COREPCH=ON` (Precompiled headers enabled).
    - `-DCMAKE_BUILD_TYPE=RelWithDebInfo`.
- **Binaries Location:** `E:\GIT\TrinityCore\build\bin\RelWithDebInfo`
    - `worldserver.exe`
    - `bnetserver.exe` (Auth server)
    - Extraction Tools included.

## Database Setup
A local MySQL instance is configured to avoid permission issues in Program Files.
- **Data Directory:** `E:\GIT\TrinityCore\mysql_data`
- **User/Pass:** `trinity` / `trinity` (configured in `.conf` files).
- **Root Access:** `root` / (no password).
- **Databases Initialized:**
    - `auth`: Standard base.
    - `characters`: Standard base (empty).
    - `world`: **Schema Only** (Imported from `sql/base/dev/world_database.sql`). No WoW data present.

## Crucial Commands
### 1. Start MySQL (Required for Server)
```powershell
Start-Process -FilePath "C:\Program Files\MySQL\MySQL Server 8.4\bin\mysqld.exe" -ArgumentList "--datadir=E:\GIT\TrinityCore\mysql_data" -WindowStyle Hidden
```

### 3. Stop All Processes
Use the included tool in the portable folder:
`E:\GIT\TrinityCore\MyMMOServer\Kill_Server.bat`
This force-kills `bnetserver`, `worldserver`, and `mysqld` and cleans up PID files.


## Next Steps & Blockers
1.  **Blocker: Client Data Files.** The server will not boot without `dbc`, `maps`, `vmaps`, and `mmaps` folders in the binary directory. These must be provided or extracted.
2.  **Startup:** Once data files are present, run `bnetserver.exe` first, then `worldserver.exe`.
3.  **Development:** Begin implementing custom opcodes or modifying the `world` database schema for the new MMO's requirements.

## Contact/History
- **Checkpoint File:** `checkpoint.json`
- **Setup Script:** `setup_database.ps1`
