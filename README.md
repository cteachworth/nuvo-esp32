# Nuvo Essentia ESPHome Integration

ESPHome-based integration for the Nuvo Essentia E6DM 6-zone audio distribution system. Control your multi-zone audio system through Home Assistant using an ESP32.

## Features

- **6 Independent Zones**: Full control of all 6 audio zones
- **Per-Zone Controls**:
  - Power on/off
  - Source selection (1-6)
  - Volume control (-78dB to 0dB)
  - Bass adjustment (-8 to +8)
  - Treble adjustment (-8 to +8)
  - Mute/unmute
- **Default Settings**: Configure default power state and audio settings per zone
- **All-Zone Controls**: Power and mute all zones simultaneously
- **State Synchronization**: Automatic polling and state updates
- **Home Assistant Dashboard**: Pre-configured Lovelace dashboard included

## Hardware Requirements

- **ESP32 NodeMCU-32S** (or compatible ESP32 board)
- **Nuvo Essentia E6DM** audio distribution system
- **RS232 to TTL Converter Module** - [HiLetgo MAX3232 Module](https://www.amazon.com/dp/B00LPK0Z9A?ref_=ppx_hzsearch_conn_dt_b_fed_asin_title_9) or equivalent MAX3232-based converter
- **DB9 cable** (optional, for connecting to the Nuvo's DB9 port)
- **5V Power Supply** for ESP32 (USB power adapter or similar)

## Wiring

### Serial Connection

The Nuvo Essentia E6DM uses a standard **DB9 female RS232 port** for serial communication at 9600 baud. You'll need an RS232-to-TTL converter (MAX3232) to interface with the ESP32's TTL logic levels.

**DB9 RS232 Pinout (Nuvo Essentia E6DM port):**
- Pin 2: RX (receive to Nuvo) - RS232 levels
- Pin 3: TX (transmit from Nuvo) - RS232 levels
- Pin 5: GND

**Connection Diagram:**
```
Nuvo Essentia E6DM     MAX3232 Module          ESP32
DB9 Female             RS232-to-TTL            NodeMCU-32S
─────────────          ──────────────          ───────────
Pin 3 (TX)  ──────────→ R1IN → T1OUT ─────────→ GPIO 16 (RX)
Pin 2 (RX)  ←────────── T1OUT ← R1IN ←───────── GPIO 17 (TX)
Pin 5 (GND) ───────────→ GND ←──────────────────→ GND
                        VCC ←──────────────────→ 3.3V
```

**Detailed Connections:**

MAX3232 Module to Nuvo DB9 (RS232 side):
- MAX3232 R1IN → Nuvo Pin 3 (TX)
- MAX3232 T1OUT → Nuvo Pin 2 (RX)
- MAX3232 GND → Nuvo Pin 5 (GND)

MAX3232 Module to ESP32 (TTL side):
- MAX3232 T1OUT (TTL) → ESP32 GPIO 16 (RX)
- MAX3232 R1IN (TTL) → ESP32 GPIO 17 (TX)
- MAX3232 VCC → ESP32 3.3V
- MAX3232 GND → ESP32 GND

**Power:**
- ESP32 5V → 5V USB power supply
- ESP32 GND → Common ground with MAX3232 module

**Note:** The MAX3232 module converts between RS232 voltage levels (±12V) used by the Nuvo and TTL levels (0-3.3V) used by the ESP32. Make sure your MAX3232 module is powered by 3.3V to match the ESP32's logic levels.

## Installation

### 1. Prepare Configuration Files

Clone this repository and create a `secrets.yaml` file:

```yaml
wifi_ssid: "YourWiFiSSID"
wifi_password: "YourWiFiPassword"
```

### 2. Deploy to Home Assistant

Use the included sync script to deploy configuration files to your Home Assistant instance:

```bash
# Edit sync-to-ha.sh to set your Home Assistant hostname/IP
./sync-to-ha.sh
```

The script will copy all necessary files to your Home Assistant ESPHome directory via SCP.

### 3. Compile and Upload

1. Open the Home Assistant ESPHome dashboard
2. Find the "nuvo" device
3. Click to compile and upload via OTA (or USB for first flash)

### 4. Import Dashboard (Optional)

The `dashboard.yaml` file contains a pre-configured Home Assistant dashboard. Import it into your Home Assistant Lovelace configuration.

## Configuration

### Zone Default Settings

Each zone has configurable default settings that are applied when the zone is powered on:

- **Default Power**: Auto-power on the zone at boot (ON/OFF)
- **Default Source**: Source input to select (1-6)
- **Default Volume**: Initial volume level (-78 to 0)
- **Default Bass**: Initial bass level (-8 to +8)
- **Default Treble**: Initial treble level (-8 to +8)

Configure these in Home Assistant after the device is added.

### Customization

Edit `essentia.yaml` to customize:

- Device name and friendly name
- Command delay between serial commands (default: 100ms)
- Polling interval (default: 60s)
- Boot behavior

## File Structure

```
.
├── essentia.yaml       # Main ESPHome configuration
├── zone.yaml           # Zone template (instantiated 6 times)
├── essentia.h          # C++ class for Nuvo protocol
├── dashboard.yaml      # Home Assistant Lovelace dashboard
├── sync-to-ha.sh       # Deployment script
└── secrets.yaml        # WiFi credentials (gitignored)
```

## Home Assistant Entities

After installation, the following entities are created:

**All Zones:**
- `switch.nuvo_essentia_all_zone_power` - Power all zones
- `switch.nuvo_essentia_all_zone_mute` - Mute all zones

**Per Zone (1-6):**
- `switch.nuvo_essentia_zone_N_power` - Zone power
- `switch.nuvo_essentia_zone_N_mute` - Zone mute
- `switch.nuvo_essentia_zone_N_default_power` - Auto-power at boot
- `number.nuvo_essentia_zone_N_source` - Source selection
- `number.nuvo_essentia_zone_N_volume` - Volume level
- `number.nuvo_essentia_zone_N_bass` - Bass level
- `number.nuvo_essentia_zone_N_treble` - Treble level
- `number.nuvo_essentia_zone_N_default_source` - Default source
- `number.nuvo_essentia_zone_N_default_volume` - Default volume
- `number.nuvo_essentia_zone_N_default_bass` - Default bass
- `number.nuvo_essentia_zone_N_default_treble` - Default treble

## Technical Details

### Serial Protocol

The Nuvo Essentia G uses a simple ASCII serial protocol at 9600 baud, 8N1:

- Commands: `*Z0<zone><command><parameters>\r`
- Responses: `#Z0<zone><data>,<data>,...\r`

Examples:
- Turn on zone 1: `*Z01ON\r`
- Set zone 1 volume to -30dB: `*Z01VOL30\r`
- Request zone 1 status: `*Z01CONSR\r`

### Command Queue

Commands are queued and processed sequentially with a configurable delay (default 100ms) to prevent overwhelming the serial interface.

### State Management

- Power and mute switches use `ALWAYS_OFF` restore mode to ensure clean state on boot
- Volume/bass/treble controls are optimistic with manual state updates
- 60-second polling interval keeps states synchronized with hardware

## Troubleshooting

### Commands not executing

Check the ESPHome logs for UART communication. You should see commands being sent and responses received. Increase the `cmd_delay` if commands are being dropped.

### Zones power on unexpectedly

Check the "Default Power" switch for each zone. If enabled, the zone will automatically power on at boot.

