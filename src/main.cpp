#include <LittleFS.h>     // SPIFFS : stockage config WiFi & MQTT
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient
#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson

#define ESP_NAME "ESPWiFi"

WiFiManager wm;
WiFiClient espClient;
PubSubClient clientMQTT(espClient);

#define FS_NAME "/credential.json"

//define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[40] = "127.0.0.1"; // privilege the IP address
char mqtt_port[6] = "1883";
char mqtt_user[20] = "mqtt_user";
char mqtt_pass[20] = "mqtt_pass";
char mqtt_topic[40] = "topic";

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// MQTT message collected
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived: [");
  Serial.print(topic);
  Serial.println("]"); // Prints out any topic that has arrived and is a topic that we subscribed to.
}

void connectWifi()
{
  // wifi
  if (WiFi.status() != WL_CONNECTED)
  {
    wm.autoConnect();

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
      ;

    Serial.println("WiFi connected.");
  }
  delay(100);
  // mqtt
  clientMQTT.setServer(mqtt_server, atoi(mqtt_port));
  clientMQTT.setCallback(callback);
  if (!clientMQTT.connected())
  {
    if (clientMQTT.connect(ESP_NAME, mqtt_user, mqtt_pass))
    {
      Serial.println("MQTT connected.");
      clientMQTT.subscribe(mqtt_topic);
    }
    else
    {
      Serial.print("MQTT failed with state ");
      Serial.println(clientMQTT.state());
      delay(5e3);
    }
  }
}

void readConfig()
{
  Serial.println("Mounting FS...");
  if (LittleFS.begin())
  {
    if (LittleFS.exists(FS_NAME))
    {
      //file exists, reading and loading
      Serial.println("Reading config file");
      File configFile = LittleFS.open(FS_NAME, "r");
      // Deserialize file to json
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, configFile);
      if (error)
      {
        Serial.println("FS failed to load json config");
        Serial.println(error.f_str());
      }
      else
      {
        JsonObject obj = doc.as<JsonObject>();
        strcpy(mqtt_server, obj["mqtt_server"]);
        strcpy(mqtt_port, obj["mqtt_port"]);
        strcpy(mqtt_user, obj["mqtt_user"]);
        strcpy(mqtt_pass, obj["mqtt_pass"]);
        strcpy(mqtt_topic, obj["mqtt_topic"]);
      }
      configFile.close();
    }
  }
  else
  {
    Serial.println("Fdailed to mount FS");
  }
}

void saveConfig()
{
  if (shouldSaveConfig)
  {
    Serial.println("Saving config");
    DynamicJsonDocument doc(1024);
    doc["mqtt_server"] = mqtt_server;
    doc["mqtt_port"] = mqtt_port;
    doc["mqtt_user"] = mqtt_user;
    doc["mqtt_pass"] = mqtt_pass;
    doc["mqtt_topic"] = mqtt_topic;

    File configFile = LittleFS.open(FS_NAME, "w");
    if (!configFile)
    {
      Serial.println("Failed to open config file for writing");
    }

    serializeJson(doc, configFile);
    configFile.close();
  }
}

void setup()
{
  Serial.begin(115200);

  // Read configuration from FS json
  readConfig();
  Serial.print("MQTT : ");
  Serial.print(mqtt_server);
  Serial.print("|");
  Serial.print(mqtt_port);
  Serial.print("|");
  Serial.print(mqtt_user);
  Serial.print("|");
  Serial.print(mqtt_pass);
  Serial.print("|");
  Serial.println(mqtt_topic);

  // The extra parameters to be config used (can be either global or just in the setup)
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 20);
  WiFiManagerParameter custom_mqtt_pass("pass", "mqtt pass", mqtt_pass, 20);
  WiFiManagerParameter custom_mqtt_topic("topic", "mqtt topic", mqtt_topic, 40);

  //set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_mqtt_user);
  wm.addParameter(&custom_mqtt_pass);
  wm.addParameter(&custom_mqtt_topic);

  //reset settings - for testing
  // wm.resetSettings();

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnect"
  //and goes into a blocking loop awaiting configuration
  while (!wm.autoConnect())
  {
    Serial.println("failed to connect and hit timeout");
    delay(3e3);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5e3);
  }

  //if you get here you have connected to the WiFi
  Serial.println("Wifi connected ...");

  // charging updated parameters
  boolean actual = (mqtt_server == custom_mqtt_server.getValue());
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  actual &= (mqtt_port == custom_mqtt_port.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  actual &= (mqtt_user == custom_mqtt_user.getValue());
  strcpy(mqtt_user, custom_mqtt_user.getValue());
  actual &= (mqtt_pass == custom_mqtt_pass.getValue());
  strcpy(mqtt_pass, custom_mqtt_pass.getValue());
  actual &= (mqtt_topic == custom_mqtt_topic.getValue());
  strcpy(mqtt_topic, custom_mqtt_topic.getValue());

  // save ifupdated the custom parameters to FS
  if (!actual)
  {
    saveConfig();
  }
}

void loop()
{
  if (!clientMQTT.connected())
  {
    connectWifi();
  }
  clientMQTT.loop();
  delay(5e3);
  clientMQTT.publish(mqtt_topic, ESP_NAME);
}
