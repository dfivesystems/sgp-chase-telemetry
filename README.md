# SailGP Chaseboat telemetry

## What is it

This provides a telemetry service to run on chase assets with the following features:
- Parse NMEA 2000 bus data and pass to Influx for presentation
- Parse GNSS data from UBlox/NMEA0183 GNSS devices
- Send GNSS data to NMEA 2000 bus
- Receive RTK correction messages and send to GNSS module
- Receive course information updates from MDSS and present on chart plotter
- Send asset position information to MDSS

Configuration is provided via a config.json file

NMEA PGNs are filtered according to the config file

NMEA instances are named in the config file