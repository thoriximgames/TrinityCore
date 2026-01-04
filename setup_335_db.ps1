# 3.3.5 Database Setup
$MySqlExe = "E:\GIT\TrinityCore\MyMMOServer\mysql\bin\mysql.exe"
$RepoRoot = "E:\GIT\TrinityCore"

# 1. Start DB
Start-Process -FilePath "E:\GIT\TrinityCore\MyMMOServer\mysql\bin\mariadbd.exe" -ArgumentList "--datadir=E:\GIT\TrinityCore\MyMMOServer\mysql\data", "--bind-address=127.0.0.1", "--port=3306" -WindowStyle Hidden
Start-Sleep -s 10

# 2. Re-create Databases
& $MySqlExe -u root -e "DROP DATABASE IF EXISTS auth; DROP DATABASE IF EXISTS characters; DROP DATABASE IF EXISTS world; CREATE DATABASE auth; CREATE DATABASE characters; CREATE DATABASE world;"

# 3. Import 3.3.5 Base Files
& cmd /c "$MySqlExe -u root auth < $RepoRoot\sql\base\auth_database.sql"
& cmd /c "$MySqlExe -u root characters < $RepoRoot\sql\base\characters_database.sql"
& cmd /c "$MySqlExe -u root world < $RepoRoot\sql\base\dev\world_database.sql"

# 4. Create Trinity User
& $MySqlExe -u root -e "CREATE USER IF NOT EXISTS 'trinity'@'localhost' IDENTIFIED BY 'trinity'; GRANT ALL PRIVILEGES ON auth.* TO 'trinity'@'localhost'; GRANT ALL PRIVILEGES ON characters.* TO 'trinity'@'localhost'; GRANT ALL PRIVILEGES ON world.* TO 'trinity'@'localhost'; FLUSH PRIVILEGES;"

Write-Host "3.3.5 Databases are ready!"
