#!/bin/bash

# Sync script for Nuvo ESPHome configuration to Home Assistant
# This script uses SCP to copy files to a remote Home Assistant server

# Configuration - EDIT THESE to match your Home Assistant setup
HA_HOST="ha.home.lan"  # or IP address like 192.168.1.100
HA_USER="root"                  # SSH user (root for HAOS, your user for Docker/Supervised)
HA_ESPHOME_PATH="/config/esphome"  # Remote path on Home Assistant

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo "======================================"
echo "Nuvo ESPHome Sync Script (SCP)"
echo "======================================"

# Check if configuration is set
if [ "$HA_HOST" = "homeassistant.local" ] || [ "$HA_HOST" = "" ]; then
    echo -e "${YELLOW}WARNING: Using default HA_HOST (homeassistant.local)${NC}"
    echo "Edit this script if your Home Assistant has a different hostname or IP"
    echo ""
fi

# Test SSH connection
echo -e "${YELLOW}Testing SSH connection to ${HA_USER}@${HA_HOST}...${NC}"
if ! ssh -o ConnectTimeout=5 -o BatchMode=yes "${HA_USER}@${HA_HOST}" "echo 'SSH connection successful'" 2>/dev/null; then
    echo -e "${RED}ERROR: Cannot connect to ${HA_USER}@${HA_HOST}${NC}"
    echo ""
    echo "Please ensure:"
    echo "  1. SSH is enabled on Home Assistant"
    echo "  2. Your SSH public key is installed (no password prompt)"
    echo "  3. HA_HOST and HA_USER are set correctly in this script"
    echo ""
    echo "For Home Assistant OS:"
    echo "  - Install the 'Terminal & SSH' add-on"
    echo "  - Add your public key in the add-on configuration"
    echo ""
    exit 1
fi

echo -e "${GREEN}✓ SSH connection successful${NC}"
echo ""

# Create nuvo subdirectory on remote
NUVO_DIR="${HA_ESPHOME_PATH}/nuvo"
echo -e "${YELLOW}Creating remote directory: ${NUVO_DIR}${NC}"
ssh "${HA_USER}@${HA_HOST}" "mkdir -p ${NUVO_DIR}" 2>/dev/null

if [ $? -ne 0 ]; then
    echo -e "${RED}ERROR: Failed to create directory on remote host${NC}"
    exit 1
fi

# Copy files via SCP
echo -e "${YELLOW}Copying files via SCP...${NC}"

scp -q essentia.yaml "${HA_USER}@${HA_HOST}:${NUVO_DIR}/"
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Copied essentia.yaml${NC}"
else
    echo -e "${RED}✗ Failed to copy essentia.yaml${NC}"
    exit 1
fi

scp -q zone.yaml "${HA_USER}@${HA_HOST}:${NUVO_DIR}/"
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Copied zone.yaml${NC}"
else
    echo -e "${RED}✗ Failed to copy zone.yaml${NC}"
    exit 1
fi

scp -q essentia.h "${HA_USER}@${HA_HOST}:${NUVO_DIR}/"
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Copied essentia.h${NC}"
else
    echo -e "${RED}✗ Failed to copy essentia.h${NC}"
    exit 1
fi

scp -q secrets.yaml "${HA_USER}@${HA_HOST}:${NUVO_DIR}/secrets.yaml"
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Copied secrets.yaml${NC}"
else
    echo -e "${RED}✗ Failed to copy secrets.yaml${NC}"
    exit 1
fi

# Create stub file on remote
STUB_FILE="${HA_ESPHOME_PATH}/nuvo.yaml"
echo -e "${YELLOW}Creating stub file on remote: ${STUB_FILE}${NC}"

ssh "${HA_USER}@${HA_HOST}" "cat > ${STUB_FILE}" << 'EOF'
# Nuvo Essentia G ESPHome Configuration
# Main configuration is in nuvo/essentia.yaml
# Edit files in the nuvo/ subfolder, not this file

<<: !include nuvo/essentia.yaml
EOF

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Created stub file${NC}"
else
    echo -e "${RED}✗ Failed to create stub file${NC}"
    exit 1
fi

# Show summary
echo ""
echo "======================================"
echo -e "${GREEN}Sync Complete!${NC}"
echo "======================================"
echo ""
echo "Files synced to:"
echo "  ${HA_USER}@${HA_HOST}:${NUVO_DIR}/"
echo ""
echo "Stub file created:"
echo "  ${HA_USER}@${HA_HOST}:${STUB_FILE}"
echo ""
echo "Next steps:"
echo "  1. Open Home Assistant ESPHome dashboard"
echo "  2. Look for 'nuvo' in the device list"
echo "  3. Click to compile and upload via OTA"
echo ""
echo "To edit configuration:"
echo "  - Edit files locally in: $(pwd)"
echo "  - Run this script again to sync changes"
echo ""
