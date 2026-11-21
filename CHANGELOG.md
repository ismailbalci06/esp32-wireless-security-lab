# Changelog

## 6.0

- Combined passive Wi-Fi and BLE observation into one ESP32-hosted dashboard.
- Added pseudonymous BLE observation IDs; raw BLE addresses are not returned to the browser.
- Added latest-40 browser-side history using `localStorage`.
- Added strongest Wi-Fi RSSI history chart.
- Added latest-two-scan BLE observation comparison.
- Added CSV export of saved scan history.
- Added sequential BLE then Wi-Fi scanning to reduce radio contention.
