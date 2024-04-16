# NodeMcu-Evil-Twin
This project aims to facilitate Evil Twin attacks using NodeMCU and an OLED display.

## How It Works

The device connects to a network via NodeMCU's WiFi module and creates a fake network resembling the target one. When users connect to the fake network, the attacker gains control.

## Requirements

- NodeMCU (ESP8266-based)
- OLED display
- Arduino IDE or PlatformIO

## Setup

1. Connect NodeMCU and OLED display.
2. Upload the code using Arduino IDE or PlatformIO.
3. Power up the device.

## Usage

1. Power up the device.
2. Wait for the device to create a WiFi access point with the SSID "iPhone (Chip ID)".
3. Connect to this access point with "nodemcu1" password.
4. Open a web browser and navigate to the default IP address of the device (http://192.168.4.1).
5. In the web interface, select the target network from the list provided.
6. The device will then create a fake network resembling the selected target.

![IMG_1996](https://github.com/batuhanturker/NodeMcu-Evil-Twin/assets/57283569/c1d790ce-c4d8-4eea-a0c1-8573979f2927)


![IMG_1999](https://github.com/batuhanturker/NodeMcu-Evil-Twin/assets/57283569/357f10e8-8e66-47db-b2df-c3499d681a21)


![IMG_1998](https://github.com/batuhanturker/NodeMcu-Evil-Twin/assets/57283569/c334d9c7-0bd2-422d-9a2c-1b19c1ecbb0b)


![IMG_2001](https://github.com/batuhanturker/NodeMcu-Evil-Twin/assets/57283569/616d37c2-ff1f-4f58-90b1-28e8be18bfb7)

## Disclaimer

This project is for educational purposes only. Any illegal use is prohibited and may lead to legal consequences. Use it within legal boundaries only.

## Contributing

This project is open-source. Contributions and feedback are welcome. Submit a pull request or open an issue to contribute.

**WARNING: This project is for educational purposes only. Any illegal use is prohibited and may lead to legal consequences. Use it within legal boundaries only.**

