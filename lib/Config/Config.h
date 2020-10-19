#pragma once

// REQUIRED

// Change this to your Wireless SSID.
#define HEW_WIFI_SSID "bar"
// Change this to your WPA2 PSK.
#define HEW_WIFI_PSK "wi9NNYara"

// Ensure that each device on your network has a unique identifier.
#define HEW_DEVICE_IDENTIFIER "puck-b"

// This passphrase is required to send commands to the device.
#define HEW_DEVICE_PASSPHRASE "hamsauce"

// Uncomment the line below to enable serial debugging.
#define HEW_LOGGING

// Change the serial baud rate if necessary.
#define HEW_LOGGING_BAUD_RATE 115200

// OPTIONAL

// Pin D4 on the D1 mini is the same as LED_BUILTIN, but they also
// chose to use it as the default pin for this shield. If you want
// to print a translucent shell, you'll want to remap the control pin
// by cutting the D4 jumper and bridging the gap between another pad
// and the control line using solder. This keeps the built-in blue LED
// off.
#define HEW_LED_PIN D4

// If you're using the RGB LED shield, leave this set to seven.
#define HEW_LED_COUNT 7

// If your WiFi network is congested, you may need to increase this.
#define HEW_HTTP_TIMEOUT_MS 2000

// You can customize the HTTP port by changing this value.
#define HEW_HTTP_PORT 80
