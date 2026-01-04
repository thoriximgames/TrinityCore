# Plan: Blank MMO Backend (TrinityCore Base)

This plan outlines the steps to create a "Blank" TrinityCore backend suitable for development of a new MMORPG. The goal is to strip Blizzard-specific content (scripts, world data) while retaining the core MMO systems (networking, auth, spell engine).

## 1. Project Analysis & Preparation
- [x] Analyze directory structure.
- [x] Identify "Blank World" SQL schema (`sql/base/dev/world_database.sql` - confirmed no data).
- [x] Identify Minimal Script configuration (`-DSCRIPTS=none` in CMake).
- [x] Establish stable TrinityCore 3.3.5 (Legacy) foundation.

## 2. Build Strategy
- **Compiler:** MSBuild (via CMake)
- **Configuration:** 
    - `SCRIPTS` = `none` (Only Commands and Spells; removes all zone/quest scripts).
    - `USE_COREPCH` = `ON` (Speed up build).
- **Target:** `worldserver`, `authserver`, and tools.

## 3. Database Strategy
The "Blank" state is achieved by using the schema-only files and adding minimum boot data.

- **Auth Database:** `sql/base/auth_database.sql`.
- **Characters Database:** `sql/base/characters_database.sql`.
- **World Database:** `sql/base/dev/world_database.sql` + Custom boot data.

## 4. Execution Progress
1.  [x] **Generate Project Files:** Run CMake with the `none` script preset.
2.  [x] **Compile:** Build the server binaries.
3.  [x] **Setup Configuration:** Configured for local database and data paths.
4.  [x] **Database Assembly:** Initialized `auth`, `characters`, `world`, and `hotfixes`.
5.  [x] **Data Extraction:** Extracted `dbc`, `maps`, `vmaps`, and `mmaps` from WoW 3.3.5a client.
6.  [x] **First Boot:** Servers are up and listening on ports 3724 and 8085.

## Next Steps
- [ ] Develop the "Content Creator" tool to automate adding custom items/NPCs.
- [ ] Customize the starting equipment and location for new players.
- [ ] Begin stripping remaining WoW-specific strings if necessary.