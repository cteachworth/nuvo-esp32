# Sync script for Nuvo ESPHome configuration to Home Assistant
# This script uses SCP to copy files to a remote Home Assistant server

# Configuration
$HA_HOST = "ha.home.lan"
$HA_USER = "root"
$HA_ESPHOME_PATH = "/config/esphome"
$NUVO_DIR = "$HA_ESPHOME_PATH/nuvo"

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Nuvo ESPHome Sync Script (SCP)"        -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan

# Test SSH connection
Write-Host "Testing SSH connection to ${HA_USER}@${HA_HOST}..." -ForegroundColor Yellow
$result = ssh -o ConnectTimeout=5 -o BatchMode=yes "${HA_USER}@${HA_HOST}" "echo ok" 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Cannot connect to ${HA_USER}@${HA_HOST}" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please ensure:"
    Write-Host "  1. SSH is enabled on Home Assistant"
    Write-Host "  2. Your SSH public key is installed (no password prompt)"
    Write-Host "  3. HA_HOST and HA_USER are set correctly in this script"
    exit 1
}
Write-Host "SSH connection successful" -ForegroundColor Green
Write-Host ""

# Create nuvo subdirectory on remote
Write-Host "Creating remote directory: $NUVO_DIR" -ForegroundColor Yellow
ssh "${HA_USER}@${HA_HOST}" "mkdir -p $NUVO_DIR"
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to create directory on remote host" -ForegroundColor Red
    exit 1
}

# Copy files via SCP
Write-Host "Copying files via SCP..." -ForegroundColor Yellow

$files = @("essentia.yaml", "zone.yaml", "essentia.h", "secrets.yaml")
foreach ($file in $files) {
    scp -q $file "${HA_USER}@${HA_HOST}:${NUVO_DIR}/"
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  Copied $file" -ForegroundColor Green
    } else {
        Write-Host "  Failed to copy $file" -ForegroundColor Red
        exit 1
    }
}

# Create stub file on remote
$STUB_FILE = "$HA_ESPHOME_PATH/nuvo.yaml"
Write-Host "Creating stub file on remote: $STUB_FILE" -ForegroundColor Yellow
$stubContent = "# Nuvo Essentia G ESPHome Configuration`n# Main configuration is in nuvo/essentia.yaml`n# Edit files in the nuvo/ subfolder, not this file`n`n<<: !include nuvo/essentia.yaml`n"
$stubContent | ssh "${HA_USER}@${HA_HOST}" "cat > $STUB_FILE"
if ($LASTEXITCODE -eq 0) {
    Write-Host "  Created stub file" -ForegroundColor Green
} else {
    Write-Host "  Failed to create stub file" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Sync Complete!" -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Files synced to: ${HA_USER}@${HA_HOST}:${NUVO_DIR}/"
Write-Host ""
Write-Host "Next steps:"
Write-Host "  1. Open Home Assistant ESPHome dashboard"
Write-Host "  2. Look for 'nuvo' in the device list"
Write-Host "  3. Click to compile and upload via OTA"
