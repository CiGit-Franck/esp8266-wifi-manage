# **esp8266-wifi-manage**
>### Goals
>- Implements libraries :
>	- [WiFiManager](https://github.com/tzapu/WiFiManager) to management of WiFi / MQTT parameters dynamically
>	- [PubSubClient](https://github.com/knolleary/pubsubclient) to communicate with an MQTT broker
>	- [ArduinoJson](https://github.com/bblanchon/ArduinoJson) to play with json
>	- [SPIFFS](https://www.teachmemicro.com/esp8266-spiffs-web-server-nodemcu/) stockage config WiFi & MQTT
>### Use
>1. Disconnect from WiFi relay
>2. Upload to ESP 
>3. Connect to the alternative wifi relay the portal for entering the parameters opens automatically otherwise (https://192.168.4.1)
>	![Portail](/Portail.PNG)
>4. Enter the parameters of your target network and it's save
>	![Form](/Credential.PNG)
