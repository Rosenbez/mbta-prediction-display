# MBTA Prediction Display
A wireless, battery powered, E-Ink arrival display for your local MBTA Stop.

![Bus Display](https://github.com/Rosenbez/mbta-prediction-display/blob/main/resources/images/bus-diplay-side.jpg)


## Features

- Wifi!
  - Uses wifi to get the arrival predictions for the MBTA stop
- Can be used with a Lion battery to be completely wireless, you can put it anywhere you want
- Bluetooth Low Energy Configuration
  - The settings for your local wifi name/password, and the stop number, are configurable using bluetooth. No firmware update required
  - Just hold the `C` button (on the linked display) to enter bluetooth mode and use your phone to adjust settings
- Rechargable with a mini-usb cable


## Parts

 - Adafruit Feather ESP32 [link](https://www.adafruit.com/product/3405)
 - Adafruit E-ink Display [link](https://www.adafruit.com/product/4777)
 - An Adafruit lion battery [2000mA](https://www.adafruit.com/product/2011)
 - Nylon screws for mounting [link](https://www.adafruit.com/product/3299)

## Dependancies
 - Adafruit EPD library
 - ArduinoJson

This project is built with PlatformIO, Arduino, and ESP32
