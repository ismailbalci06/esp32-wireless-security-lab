# ESP32 Wireless Security Lab 6.0

**A local, passive Wi-Fi and Bluetooth Low Energy (BLE) observation dashboard built with an ESP32.**

The project scans nearby 2.4 GHz Wi-Fi access points and BLE advertisements, then serves the observations through a small web dashboard hosted by the ESP32 itself. The dashboard keeps a rolling history in the browser, plots the strongest observed Wi-Fi RSSI, compares the last two BLE observation sets, and exports saved observations to CSV.

> **Safety and privacy:** This project is intended for your own equipment, a private lab, and environments where passive observation is permitted. It does not attempt to join scanned Wi-Fi networks, authenticate to access points, pair with BLE devices, or transmit BLE scan requests. Wi-Fi beacon/probe observation and BLE advertisements can still expose nearby network/device metadata. Do not publish raw scan CSVs or screenshots without reviewing and redacting them.

## Features

- Passive Wi-Fi scanning using the ESP32 Arduino Wi-Fi scan API.
- Wi-Fi list sorted by RSSI (strongest first), with SSID, RSSI, channel, and a basic open/secured label.
- 2.4 GHz channel-count visualization for channels 1–13.
- Passive BLE advertisement scanning (`setActiveScan(false)`).
- BLE entries show a short pseudonymous observation ID instead of the raw BLE address.
- Combined Wi-Fi + BLE scan endpoint, executed sequentially.
- Responsive local dashboard with Wi-Fi, BLE, channel, and history views.
- Automatic refresh every 30 seconds and a manual scan button.
- Last 40 scan snapshots saved in the browser's `localStorage`.
- Wi-Fi strongest-signal history chart and last-two-scan BLE observation comparison.
- CSV export of the saved browser-side scan history.

## Screenshots and test evidence

The dashboard was tested on the hardware described below. Screenshots are intentionally not bundled: the user's captures showed nearby SSIDs and device observations. Before adding screenshots to a public repository, mask nearby network names, BLE IDs, desktop account details, and other identifying information.

See [manual test results](docs/TEST_RESULTS.md) and the [architecture](docs/ARCHITECTURE.md).

## Hardware and software

| Item | Tested / required configuration |
|---|---|
| Board | ESP32 development board; upload log identified `ESP32-D0WD-V3` |
| Arduino board selection | `ESP32 Dev Module` |
| Arduino IDE | 2.3.11 (observed during testing) |
| ESP32 Arduino core | 3.3.12 (observed during testing) |
| Partition scheme | `Huge APP (3MB No OTA/1MB SPIFFS)` |
| USB connection | USB-to-serial; the tested macOS port was `/dev/cu.usbserial-0001` |
| Browser | Chrome on macOS (tested) |
| Extra components | None required; USB cable only |

The sketch uses the ESP32 core's Wi-Fi, WebServer, and bundled BLE APIs. No external Arduino library installation is required beyond installing the Espressif ESP32 board package.

## Quick start

1. Install Arduino IDE 2.x.
2. In Boards Manager, install **esp32 by Espressif Systems**. The tested setup used core **3.3.12**.
3. Open `ESP32_Wireless_Security_Lab_6_0.ino`.
4. Select **Tools → Board → esp32 → ESP32 Dev Module** (or use the board selector).
5. Select the correct USB serial port.
6. Select **Tools → Partition Scheme → Huge APP (3MB No OTA/1MB SPIFFS)**. The default application partition was too small for this combined sketch.
7. **Change `AP_PASSWORD` near the top of the sketch** to a unique password of at least 8 characters. Do not reuse a personal or production password.
8. Click **Verify**, then **Upload**.
9. Open Serial Monitor at **115200 baud** and reset the board if the startup messages are not visible.
10. Connect your computer to the ESP32 access point and open `http://192.168.4.1`.

### Access point settings

The source contains these defaults:

- SSID: `ESP32-Wireless-Lab`
- Password: `ChangeThis_AP_Password_2026!` (**placeholder — change it before uploading**)
- Dashboard: `http://192.168.4.1`

Your computer may warn that the ESP32 access point has no internet connection. That is expected; this is a local dashboard. Keep the local Wi-Fi connection to access the page. Internet access is not required for the dashboard.

## How it works

1. The ESP32 starts a password-protected soft access point and an HTTP server.
2. The browser loads the embedded dashboard from the ESP32.
3. The browser requests `/api/scan` for a combined scan.
4. The firmware performs a passive BLE scan, followed by a passive Wi-Fi scan, then returns JSON.
5. The browser timestamps the response, stores the snapshot in `localStorage`, renders the dashboard, and trims history to the most recent 40 snapshots.
6. CSV export is generated in the browser from the saved snapshots.

The browser clock supplies timestamps. The history is **not stored in ESP32 flash or NVS**. It is scoped to the browser's storage for `http://192.168.4.1`; clearing that site's browser data removes the history.

## Important interpretation notes

- RSSI values are approximate received-signal-strength measurements at the ESP32 antenna. They are not a measure of internet speed, link quality, or proof of interference.
- The channel panel counts observed access points per 2.4 GHz channel. It does **not** measure channel utilization, airtime, noise floor, or radio interference.
- The Wi-Fi label `Secured` means the scan API reported a non-open network; it does not assess password strength or overall security.
- BLE scanning sees advertisements in range during the scan window. Devices that do not advertise, are out of range, or are between advertising events may not appear.
- BLE IDs use a 32-bit FNV-1a hash of the observed BLE address. This is a **pseudonymous label, not cryptographic anonymization**; collisions are possible, and address randomization can make one physical device appear under a different ID.
- A BLE entry missing from the next scan is only “not seen in that scan”; it does not prove the device powered off or left the area.
- The ESP32 has a shared 2.4 GHz radio. Scans run sequentially, and the synchronous scan endpoint can temporarily delay dashboard requests.

## Repository structure

```text
ESP32_Wireless_Security_Lab_6_0/
├── ESP32_Wireless_Security_Lab_6_0.ino
├── README.md
├── README_TR.md
├── .gitignore
└── docs/
    ├── ARCHITECTURE.md
    ├── SECURITY_AND_PRIVACY.md
    ├── TEST_RESULTS.md
    ├── ROADMAP.md
    └── assets/
        └── architecture.svg
```

## Current status

The 6.0 firmware was compiled and uploaded to the user's ESP32-D0WD-V3 development board. The local dashboard was opened successfully, Wi-Fi and BLE observations appeared, channel distribution rendered, browser history remained after a page refresh, and CSV export was produced. Details and caveats are in [TEST_RESULTS.md](docs/TEST_RESULTS.md).

## License

No license has been selected for this repository yet. Choose and add a license before inviting others to reuse or distribute the code. Public visibility on GitHub does not, by itself, grant reuse permissions.

## Author

Add your preferred name, GitHub profile URL, and contact method here before publishing.
