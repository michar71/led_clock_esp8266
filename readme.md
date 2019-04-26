# LED CLock ESP8266

Simple Wifi-Enabled LED clock with 60 RGB-LED's

## Features
- Uses only two pins of the ESP8266 (LED and button)
- Connects to local Internet AP
- Pulls Time from NTP Server
- Adjusts brightness of LED accroding to local sunset/sunrise
- Deals with Daylighht Savings Time


## Compile Installation
- Instal Visual Studio Code
- Install PlatformIO Plugin
- Compile/Upload (Via USB)

## Setup
- Hold Button during power-up/Reset
- AP with Captive Webser will show up
- Select your Internet AP, Enter PAssword, select settings
- (Only the strongest 10 AP's will be shown, if yout AP doesn't show up move closer to it for setup...)
- On Save Clok will restart 

## ToDo
- Fix Webserver when clock is running
- Add Plugin Infrastructure

## External References
- NeoPixelBus Library
- DateTime Library