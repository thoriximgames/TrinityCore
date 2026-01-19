# Remote Build Automation for MMO-Server
# Usage: ./scripts/remote_build.ps1

$ServerIP = "192.168.0.101"
$User = "trader2"
$RemotePath = "/home/trader2/mmo-server"

Write-Host "--- Syncing source to $ServerIP ---" -ForegroundColor Cyan

# Sync src, Shared, and CMakeLists.txt (excluding build/temp files)
# Note: Using scp -r for simplicity in this environment
scp -r src Shared CMakeLists.txt Dockerfile docker-compose.yml "${User}@${ServerIP}:${RemotePath}"

Write-Host "--- Executing Remote Build (Oracle) ---" -ForegroundColor Cyan

# Execute build commands on server
# We assume the server has the necessary tools installed as per Dockerfile
ssh "${User}@${ServerIP}" "cd ${RemotePath} && mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make -j$(nproc)"

if ($LASTEXITCODE -eq 0) {
    Write-Host "--- Oracle Verification SUCCESS ---" -ForegroundColor Green
} else {
    Write-Host "--- Oracle Verification FAILED ---" -ForegroundColor Red
    exit 1
}
