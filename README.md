# VCP Clock

Replica of the electronic clock installed in the former Vilnius Central Post building operations hall.

The clock uses individually addressable LEDs to reproduce the original hour and minute display and is controlled by an ESP8266-based ESP-12F module.

## Hardware

- ESP-12F / ESP8266
- WS2812 addressable LED strips
- 36 LEDs for the hour display
- 72 LEDs for the minute display
- 3D-printed enclosure

## Time synchronization

The clock synchronizes its time over Wi-Fi using NTP.

Time is obtained from `pool.ntp.org`. The configured UTC offset is applied locally.

Optional automatic daylight-saving time adjustment is supported according to the European DST schedule.

## Wi-Fi configuration

On startup, the clock attempts to connect to the previously configured Wi-Fi network.

The number of connection attempts and the amount of time spent waiting during each attempt can be configured through the web interface.

While connecting, the minute strip displays a red loading animation.

If no Wi-Fi credentials are stored, or if the configured network cannot be reached after the specified number of attempts, the clock starts a Wi-Fi configuration portal.

Connect to:

`VCP-Clock`

and use the ConfigPortal to select and configure a Wi-Fi network.

While the ConfigPortal is active, the minute strip displays a separate random-pixel animation.

The configured Wi-Fi network can also be erased from the clock's web interface using the **Forget Wi-Fi and reboot** button. After rebooting, the clock will enter ConfigPortal mode.

## Web interface

When connected to Wi-Fi, the clock runs a web configuration interface on its local IP address.

The interface allows configuration of:

- Timezone offset
- Automatic daylight-saving time adjustment
- LED brightness
- LED color
- Automatic clock on/off
- Automatic off time
- Automatic on time
- Wi-Fi connection attempt count
- Waiting time per Wi-Fi connection attempt

The page also displays diagnostic information such as:

- Device uptime
- Last ESP reset reason
- Current IP address

The clock's IP address can be found in the connected-device list of the local router.

## Automatic on/off

The clock can automatically turn its LEDs off and on according to a configured daily schedule.

For example:

- Auto-off: `23:00`
- Auto-on: `07:00`

With these settings enabled, the display remains off between 23:00 and 07:00 while the ESP8266 itself continues running normally.

This does not disconnect Wi-Fi or stop time synchronization.

## Persistent settings

The clock stores its main configuration in ESP8266 flash-backed EEPROM.

Persistent settings include:

- Timezone
- DST mode
- Automatic on/off state
- Automatic on/off times
- Wi-Fi connection retry settings

Wi-Fi credentials are stored separately by the ESP8266 Wi-Fi subsystem / WiFiManager.

LED brightness and color are currently runtime settings and return to their firmware defaults after a reboot.

## Default settings

The firmware currently uses the following defaults:

- Timezone: UTC+2
- DST adjustment: enabled
- Automatic on/off: disabled
- Auto-on time: 07:00
- Auto-off time: 23:00
- Wi-Fi connection attempts: 3
- Wait per attempt: 30 seconds
- Brightness: 50
- LED color: orange

## Previous version

The original version of this project was based on an Arduino Nano, DS3231 RTC module and HC-06 Bluetooth module.

That implementation used a Thunkable mobile application and serial commands for configuration.

The current ESP8266 version replaces the RTC, Bluetooth interface and mobile application with NTP synchronization, Wi-Fi configuration and a browser-based settings interface.
