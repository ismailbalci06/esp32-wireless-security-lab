# Roadmap

The current 6.0 release is a working prototype validated through manual use on one ESP32 development board. Potential follow-up work:

1. **Robust scan-state handling:** Return explicit scan status and duration; avoid ambiguous empty results when a scan fails.
2. **History observability:** Show whether localStorage persistence succeeded and provide a storage/export status indicator.
3. **Better BLE identity model:** Keep the privacy-preserving default, document address rotation, and consider a configurable per-session salt if stable cross-session correlation is not wanted.
4. **Data controls:** Add configurable retention (e.g. 10/20/40 scans) and a confirmation before exporting the complete history.
5. **RF measurement clarity:** Continue to label channel counts as observed AP counts, not utilization or interference.
6. **Testing:** Add repeatable tests for JSON escaping, empty scans, storage quota failures, and browser refresh recovery.
7. **Documentation:** Add screenshots only after SSIDs, BLE IDs, and personal desktop details have been redacted.

No active scanning, deauthentication, credential collection, or connection attempts are planned for this passive-observation project.
