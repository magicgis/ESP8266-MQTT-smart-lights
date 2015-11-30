# ESP8266 MQTT client for smart lights
ESP8266 MQTT client which monitors temperature, atmospheric pressure and ambient illumination and sends them to specified MQTT broker. This client also allows to control output pin to control 230V relay.

It uses mqtt client from tuanpmt https://github.com/tuanpmt/esp_mqtt.git

## Messages
### Publish
Client is publishing heartbeat message every 2 seconds by default.

It publishes on topics:
- /ESP8266/ALL/HEARTBEAT 
- /ESP8266/chip_id/HEARTBEAT (chip_id is chip id in uppercase HEX)

and message format is following:

"chip_id:increment:wifi_rssi:relay_state:illumination:pressure:temperature"

### Subscribe
Each client is subscribed do following topics:
- /ESP8266/ALL/GPIO
- /ESP8266/chip_id/GPIO (chip_id is chip id in uppercase HEX)

Message format is simply desired value of relay.
- "0" : disabled
- "1" : enabled

Client sends heartbeat packet with updated relay state as response immediately after receiving valid message on subscription topic.
