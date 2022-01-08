## Enviroment monitor
Measure temperature, humidity, atmospheric pressure, and CO2 concentration using [Atom Lite](https://docs.m5stack.com/en/core/atom_lite). The measurements are pushed to [Ambient](https://ambidata.io/) IoT server. The data can be visualized and seen in the web app.

## Preparation
Prepare below devices and connect them to Atom Lite.

- [ENV III sensor](https://docs.m5stack.com/en/unit/envIII) for temperature, humidity, and pressure measurement.
- [TVOC/eCO2](https://docs.m5stack.com/en/unit/tvoc) for CO2 measurement.
- [HUB](https://docs.m5stack.com/en/unit/hub) to expand single port into three ports. 

## How to use
Specify below configuration depending on your environment, then upload the program to your device.

```cpp
// WiFi
#define WIFI_SSID "ssid"
#define WIFI_PASSWD "passwd"

// Measurement interval in second
#define INTERVAL 180

// Ambient
#define IS_USE_AMBIENT true
#define CHANNELID -1
#define WRITEKEY "writekey"
```