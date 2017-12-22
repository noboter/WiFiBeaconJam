# WiFiBeaconJam
ESP8266 programm to fool WiFi trackers

This crates a lot of WiFi packets, so that any receiving WiFi device will observe lots of nonexisting base stations.
Additionally it creates a lot of probe requests, to simulate the presence of a high number of devices looking for a base station.

Generates macadresses from existing vendors throug usage of this http://standards-oui.ieee.org/oui.txt list
