# Security and privacy notes

## Intended use

Use this device only for your own equipment, a private lab, or environments where passive wireless observation is authorized. Do not use observations to identify, track, or profile people. Do not publish raw nearby network names, BLE observation IDs, or scan logs without checking for sensitive information.

## What the project does

- Passively observes nearby Wi-Fi access-point broadcasts.
- Passively observes BLE advertisement packets.
- Hosts a password-protected local Wi-Fi access point and dashboard.
- Keeps the last 40 snapshots in the browser's local storage.

## What the project does not do

- It does not connect to scanned Wi-Fi access points or attempt authentication.
- It does not capture Wi-Fi credentials or decrypt traffic.
- It does not pair with BLE devices, connect to BLE GATT services, or send active BLE scan requests.
- It does not send scan results to a cloud service.

## Password handling

`AP_PASSWORD` is compiled into firmware. Change the placeholder before uploading, and do not commit a password you intend to keep private. Anyone with physical access to the device or firmware image may be able to recover embedded strings; treat this as a local-lab access control, not a high-assurance secrets vault.

## BLE observation IDs

The sketch derives a 32-bit FNV-1a hash from the BLE address. The raw address is not included in the JSON response or dashboard, but the hash is only a **pseudonymous observation identifier**:

- it is not a cryptographic hash or a guarantee of anonymity;
- collisions are possible;
- if a device rotates its BLE address, its ID may change;
- if an address is known, the corresponding hash can be recomputed.

The dashboard's “new” and “not seen in the latest scan” labels describe scan-to-scan observations only. They do not prove a physical device arrived, departed, powered on, or powered off.

## Browser history and exports

History is stored on the computer running the browser, in `localStorage` for `http://192.168.4.1`. Clearing site data deletes it. CSV files contain observed SSIDs, BLE pseudonymous IDs, signal levels, and timestamps; keep exported reports private and redact them before publication.

## Reporting a project issue

For a code issue, open a GitHub issue with the firmware version, ESP32 core version, board selection, partition scheme, serial output, and reproduction steps. Remove personal identifiers, Wi-Fi names, and BLE IDs from logs before posting. Do not attach unredacted scan exports.
