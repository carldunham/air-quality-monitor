# air-quality-monitor

Measure local air quality, display values from sensor and public sources

Built on the awesome [Mongoose OS](https://mongoose-os.com) platform.

Designed to run on an [Adafruit HUZZAH32 – ESP32 Feather Board](https://www.adafruit.com/product/3619), which is a nice, compact ESP32 platform.

Uses an [Adafruit FeatherWing OLED - 128x32](https://www.adafruit.com/product/2900) for display.

## Setup

## Building the software

The usual Mongoose-OS stuff:

```
mos build --local --board featheresp32 --verbose && mos flash && mos console
```
