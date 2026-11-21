# Manual test results

## Test environment

- Test date shown in the submitted logs/screenshots: 21 November 2026.
- Arduino IDE: 2.3.11.
- Espressif ESP32 Arduino core: 3.3.12.
- Board selection: `ESP32 Dev Module`.
- Chip reported by esptool: `ESP32-D0WD-V3 (revision v3.1)`.
- Partition scheme: `Huge APP (3MB No OTA/1MB SPIFFS)`.
- Browser: Chrome on macOS.

## Firmware build and upload

The user-provided Arduino output reported:

- Sketch size: 1,679,235 bytes (53% of 3,145,728-byte application partition).
- Global variables: 65,556 bytes (20% of 327,680-byte dynamic memory).
- Flash write completed and `Hash of data verified` was reported.
- Device reset completed via RTS.

## Startup validation

The Serial Monitor showed the following successful startup sequence:

```text
Starting Wireless Security Lab 6.0
Access Point started!
Network: ESP32-Wireless-Lab
Password: [redacted in this document]
Dashboard: http://192.168.4.1
BLE passive scanner initialized.
Web server started!
Wireless Security Lab 6.0 READY!
```

## Functional checks observed in the browser

| Test | Observed result |
|---|---|
| Connect to ESP32 SoftAP | Successful after removing the saved earlier network profile and using the 6.0 password |
| Open local dashboard | Successful at `http://192.168.4.1` |
| Wi-Fi scan list | Observed; counts varied between scans as expected |
| BLE scan list | Observed; counts varied between scans as expected |
| Channel distribution | Rendered |
| Manual and automatic scans | Scan status and data updated |
| History chart | Rendered with multiple scan timestamps |
| BLE latest-scan comparison | Rendered “newly seen” / “not seen in latest scan” or no-change messages |
| Page refresh persistence | Previous history remained visible after refresh; later UI showed 9 of 40 retained snapshots |
| CSV export | A CSV export was provided and parsed successfully |

## CSV sample analysis

The supplied `wireless-security-lab-6-history.csv` sample contained:

- 7 distinct local timestamps (scan snapshots).
- 98 observation rows in total.
- 59 Wi-Fi observation rows.
- 39 BLE observation rows.

This CSV was exported before the later screenshot that showed 9/40 saved snapshots; the counts therefore describe that CSV file, not the later live browser state. Raw network names and BLE IDs are deliberately not reproduced here.

## Interpretation and limits

These are manual functional checks, not a formal RF performance, security certification, or long-duration reliability test. Detected network/device counts vary with location, antenna orientation, radio conditions, and timing. The channel chart counts observed access points; it is not a channel-utilization or interference measurement. BLE IDs are pseudonymous and may change due to address randomization.
