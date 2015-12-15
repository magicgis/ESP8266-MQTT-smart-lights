# ESP8266 MQTT client for smart lights
ESP8266 MQTT client which monitors temperature, atmospheric pressure and ambient illumination and sends them to specified MQTT broker (in this case Mosquitto). This client also allows to control output pin to control 230V relay.

It uses mqtt client from tuanpmt https://github.com/tuanpmt/esp_mqtt.git

## Messages
### Publish
Client is publishing heartbeat message every 2 seconds by default.

It publishes on topics:
- /ESP8266/ALL/HEARTBEAT 
- /ESP8266/chip_id/HEARTBEAT (chip_id is chip id in uppercase HEX)

and message format is following:

"chip_id:increment:wifi_rssi:relay_state:illumination:pressure:temperature"

where:
- chip_id = chip id in uppercase HEX
- increment = constantly incrementing (and then overflowing) byte value used for estimating packet loss and packet order
- wifi_rssi = receieved signal strength indicator for wifi AP
- relay_state = state of relay (and also LED), can be 0 or 1
- illumination = value of ambient illumination from OPT3001 sensor
- pressure = atmospheric pressure in pascals*100 from MPL3115 sensor
- temperature = temperature in degree celsius*100 from MPL3115 sensor

### Subscribe
Each client is subscribed to following topics:
- /ESP8266/ALL/GPIO
- /ESP8266/chip_id/GPIO (chip_id is chip id in uppercase HEX)

Message format is simply desired value of relay.
- "0" : disabled
- "1" : enabled

Client sends heartbeat packet with updated relay state as response immediately after receiving valid message on subscription topic.

## Sensor board
Communication with sensor board is realized via I2C.
Sensor board consists of following sensors:
- OPT3001 (I2C address 0x44)
- MPL3115 (I2C address 0x60)
