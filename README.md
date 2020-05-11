# Hew D1

[![Build Status](https://travis-ci.org/ayan4m1/hew-d1.svg?branch=master)](https://travis-ci.org/ayan4m1/hew-d1)

This project is an open source hardware implementation of end-user controllable RGB LED lighting using the [LOLIN D1 mini](https://docs.wemos.cc/en/latest/d1/d1_mini.html) and the [RGB LED shield](https://docs.wemos.cc/en/latest/d1_mini_shiled/rgb_led.html).

## Features

- Control the color and brightness of LEDs via [Hew](https://github.com/ayan4m1/hew-web)
- Solid color or animated patterns
- Simple passphrase based authentication
- Zeroconf/mDNS support
- Configuration persisted in NVRAM between restarts

## Usage

Install [PlatformIO](https://platformio.org/platformio-ide) for your favorite IDE. Then, use the [`platformio`](https://docs.platformio.org/en/latest/core/quickstart.html#process-project) command to build and upload the firmware to a D1 mini.

### Configuration

Edit `src/config.h` before building/flashing the firmware.

If you choose a device identifier of `puck-a`, then your device will be accessible at `http://puck-a.local` after it connects to WiFi. You can customize the HTTP port by editing `src/config.h`. You control it by sending a POST request to `/` with the following:

- Header: `Authorization: passphrase`, where `passphrase` is the configured `DEVICE_PASSPHRASE`
- Body: Raw JSON with the following keys
  - **r** - the red component of the color, 0-255
  - **g** - the green component of the color, 0-255
  - **b** - the blue component of the color, 0-255
  - **br** - the brightness of the lights, 0-255
  - **p** - a pattern string, can be either `SOLID` or `MARQUEE`

## Roadmap

- Overhaul HTTP request parsing
- Add Apple HomeKit / Alexa / Google Home integration
- Support arbitrary-length panels/strands
- Configurable animations
- TLS server support
