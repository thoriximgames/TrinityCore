<#
.SYNOPSIS
    Sets up a Blank TrinityCore Database (Auth, Characters, and Schema-Only World).
#>

param (
    [string]$MySqlBinPath = "C:\Program Files\MySQL\MySQL Server 8.4\bin"
)

$ErrorActionPreference = "Stop"
$RepoRoot = "E:\GIT\TrinityCore"

# --- Paths to SQL Files ---
$SqlAuth       = Join-Path $RepoRoot "sql\base\auth_database.sql"
$SqlChars      = Join-Path $RepoRoot "sql\base\characters_database.sql"
$SqlWorldBlank = Join-Path $RepoRoot "sql\base\dev\world_database.sql"
$SqlCreate     = Join-Path $RepoRoot "sql\create\create_mysql.sql"

$MySqlExe = "$MySqlBinPath\mysql.exe"
$User = "root"
$Pass = "" # Using empty for initialized-insecure

# --- Build Command Helper ---
function Run-DbImport {
    param([string]$DbName, [string]$File)
    
    $ArgsList = @("-u", $User)
    if ($Pass) { $ArgsList += "-p$Pass" }
    if ($DbName) { $ArgsList += $DbName }

    Write-Host "Importing $File into $DbName..." -ForegroundColor Cyan
    $Process = Start-Process -FilePath "cmd.exe" -ArgumentList "/c `"`"$MySqlExe`" $($ArgsList -join ' ') < `"$File`"`"" -Wait -PassThru -NoNewWindow
    
    if ($Process.ExitCode -ne 0) {
        Write-Error "MySQL Error importing $File. Exit Code: $($Process.ExitCode)"
    }
}

# --- Execution ---
Write-Host "`nStep 1: Creating Databases/Users..." -ForegroundColor Yellow
$CreateArgs = @("-u", $User)
Start-Process -FilePath "cmd.exe" -ArgumentList "/c `"`"$MySqlExe`" $($CreateArgs -join ' ') < `"$SqlCreate`"`"" -Wait -NoNewWindow

Write-Host "`nStep 2: Importing Auth Database..." -ForegroundColor Yellow
Run-DbImport -DbName "auth" -File $SqlAuth

Write-Host "`nStep 3: Importing Characters Database..." -ForegroundColor Yellow
Run-DbImport -DbName "characters" -File $SqlChars

Write-Host "`nStep 4: Importing BLANK World Database (Schema Only)..." -ForegroundColor Yellow
Run-DbImport -DbName "world" -File $SqlWorldBlank

Write-Host "`nSuccess! Blank Server Databases are ready." -ForegroundColor Green