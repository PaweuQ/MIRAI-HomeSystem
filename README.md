# MIRAI-HomeSystem
**MIRAI** is a modular home automation playground. It starts as a hobby project in a one-room apartament. Let's see how far it will go!!


## OUTDOOR WEATHER STATION

UPDATED 23.08.2025
Latest code supports following features: 
- weather station reads values of temperature (DS18B20, DHT) and humidity (DHT). It also detects rain. 3 sensors in total. 
- NTP synchronization for creating timestamps
- logging data on microSD card with 60 seconds interval
- automaticly refresh the connection with WiFi if needed
- automaticly reinitialize connection to microSD card module if it's lost 
- service button in case you need to remove SD card - to avoid file crushing
- additional button to reinitialize SD if necessary 
