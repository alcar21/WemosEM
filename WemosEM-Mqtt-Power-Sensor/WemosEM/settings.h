
/*
  Default parameters, settings load and save functions

  Alfonso C. Alvarez (Alcar), 14nd September 2019

  @author <a href="mailto:alcar21@gmail.com">Alfonso Carlos Alvarez Reyes</a>

  Compile with Arduino 2.4.2
*/

#define DEFAULT_EMPTY ""

// WIFI
#define DEFAULT_WIFI_NAME ""
#define DEFAULT_WIFI_PASSWORD ""
#define DEFAULT_IP_MODE 0 //  0 = DHCP, 1 = Static
#define DEFAULT_IP ""
#define DEFAULT_MASK ""
#define DEFAULT_GATEWAY ""
// MQTT
#define DEFAULT_MQTT_SERVER "192.168.1.2"
#define DEFAULT_MQTT_PORT 1883
#define DEFAULT_MQTT_USERNAME ""
#define DEFAULT_MQTT_PASSWORD ""
// CALIBRATE
#define DEFAULT_MESSAGE_INTERVAL 10000
#define DEFAULT_VOLTAGE 225.0
#define DEFAULT_ICAL 30.50
// TIMEZONE
#define DEFAULT_TIMEZONE 1
#define DEFAULT_MINUTES_TIMEZONE 0
// SYSTEM
#define DEFAULT_SYSTEM_USER "wemosem"
#define DEFAULT_SYSTEM_PASSWORD ""
// Motion Sensor
#define MOTION           // Uncomment to enable
#define INTERRUPT_PIN D5  // use pin 2 on Arduino Uno & most boards
#define SCL_PIN D6
#define SDA_PIN D7

// VARIABLES
String system_password = DEFAULT_SYSTEM_PASSWORD;
int ipMode = DEFAULT_IP_MODE; // 0 = DHCP, 1 = Static

// Add your Wifi details below. Set SSID to NULL if unused.
// WiFi slot 1
String wifi_name = DEFAULT_WIFI_NAME;
String wifi_password =  DEFAULT_WIFI_PASSWORD;
String ip =  DEFAULT_IP;
String mask = DEFAULT_MASK;
String gateway = DEFAULT_GATEWAY;

// MQTT Broker address and port
boolean mqtt_enabled = false;
String mqtt_server = DEFAULT_MQTT_SERVER;
int mqtt_port = 1883;
String mqtt_username = DEFAULT_MQTT_USERNAME;
String mqtt_password = DEFAULT_MQTT_PASSWORD;

int message_interval = DEFAULT_MESSAGE_INTERVAL;     // Time in ms between each message. 5000 ms = 5 seconds

// Mains voltage and calibration
float mainsVoltage = DEFAULT_VOLTAGE;          // Mains voltage in Volts. Change this value according to your mains.
float Ical = DEFAULT_ICAL;             // Varia dentro del mismo fabricante, hay que calibrarlo (1800 vueltas del secundario STC013 / 69 Ohm resistencia carga (burden)) = 26.1

// Iot Platforms
// Blynk platform
boolean blynk_enabled = false;
String blynkAuth = DEFAULT_EMPTY;
String blynkServer = DEFAULT_EMPTY;
uint16_t blynkPort = 0;

// ThingSpeak platform
boolean thingSpeak_enabled = false;
unsigned long tsChannelNumber = 0;
String tsWriteAPIKey = DEFAULT_EMPTY;

// TimeZone
int8_t timeZone = DEFAULT_TIMEZONE;
int8_t minutesTimeZone = DEFAULT_MINUTES_TIMEZONE;

// Date RESET
int dayReset = 0;

boolean loadConfig() {

  String jsonString;

  if (!SPIFFS.begin()) {
    Serial.println("ERROR: Failed to mount file system");
    return false;
  }

  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("ERROR: Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  // if (size > 1024) {
  //   Serial.println("ERROR: Config file size is too large");
  //   return false;
  // }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  DynamicJsonDocument jsonConfig(1024);
  auto error = deserializeJson(jsonConfig, buf.get());
  if (error) {
    Serial.println("Failed to parse config file");
    return false;
  }

  serializeJson(jsonConfig, jsonString);
  Serial.println("LOADED CONFIG: " + jsonString);

  // WIFI
  wifi_name = (jsonConfig["wifi_name"] == "" ? DEFAULT_WIFI_NAME : jsonConfig["wifi_name"].as<String>());
  wifi_password = (jsonConfig["wifi_password"] == "" ? DEFAULT_WIFI_PASSWORD : jsonConfig["wifi_password"].as<String>());
  ipMode = (jsonConfig["ip_mode"] == "" ? DEFAULT_IP_MODE : jsonConfig["ip_mode"]);
  ip = (jsonConfig["ip"] == "" ? DEFAULT_IP : jsonConfig["ip"].as<String>());
  mask = (jsonConfig["mask"] == "" ? DEFAULT_MASK : jsonConfig["mask"].as<String>());
  gateway = (jsonConfig["gateway"] == "" ? DEFAULT_GATEWAY : jsonConfig["gateway"].as<String>());
  // MQTT
  mqtt_enabled = (jsonConfig["mqtt_enabled"]);
  mqtt_server = (jsonConfig["mqtt_server"] == "" ? DEFAULT_MQTT_SERVER : jsonConfig["mqtt_server"].as<String>());
  mqtt_port = (jsonConfig["mqtt_port"] == "" ? DEFAULT_MQTT_PORT : jsonConfig["mqtt_port"]);
  mqtt_username = (jsonConfig["mqtt_username"] == "" ? DEFAULT_MQTT_USERNAME : jsonConfig["mqtt_username"].as<String>());
  mqtt_password = (jsonConfig["mqtt_password"] == "" ? DEFAULT_MQTT_PASSWORD : jsonConfig["mqtt_password"].as<String>());
  // CALIBRATE
  message_interval = (jsonConfig["message_interval"] == "" ? DEFAULT_MESSAGE_INTERVAL : jsonConfig["message_interval"]);
  mainsVoltage = (jsonConfig["voltage"] == "" ? DEFAULT_VOLTAGE : jsonConfig["voltage"]);
  Ical = (jsonConfig["ical"] == "" ? DEFAULT_ICAL : jsonConfig["ical"]);
  // IOT PLATFORMS
  blynk_enabled = (jsonConfig["blynk_enabled"] == 1 ? true : false);
  blynkAuth = (jsonConfig["blynkAuth"] == "" || jsonConfig["blynkAuth"] == "null" ? DEFAULT_EMPTY : jsonConfig["blynkAuth"].as<String>());
  blynkServer = (jsonConfig["blynkServer"] == "" || jsonConfig["blynkServer"] == "null" ? DEFAULT_EMPTY : jsonConfig["blynkServer"].as<String>());
  blynkPort = (jsonConfig["blynkPort"] == "" ? DEFAULT_MQTT_PORT : jsonConfig["blynkPort"]);

  thingSpeak_enabled = (jsonConfig["ts_enabled"] == 1 ? true : false);
  tsChannelNumber = (jsonConfig["tsChannelNumber"] == "" ? 0 : jsonConfig["tsChannelNumber"]);
  tsWriteAPIKey = (jsonConfig["tsWriteAPIKey"] == "" || jsonConfig["tsWriteAPIKey"] == "null" ? DEFAULT_EMPTY : jsonConfig["tsWriteAPIKey"].as<String>());

  // SYSTEM - TIMEZONE
  timeZone = (jsonConfig["timezone"] == "" ? DEFAULT_TIMEZONE : jsonConfig["timezone"]);
  minutesTimeZone = (jsonConfig["minutes_timezone"] == "" ? DEFAULT_MINUTES_TIMEZONE : jsonConfig["minutes_timezone"]);
  // SYSTEM
  system_password = (jsonConfig["system_password"] == "" ? DEFAULT_SYSTEM_PASSWORD : jsonConfig["system_password"].as<String>());
  dayReset = (jsonConfig["dayReset"] == "" ? 0 : jsonConfig["dayReset"]);

  // KWH
  watiosTotal = (jsonConfig["watiosTotal"] == "" ? 0.0 : jsonConfig["watiosTotal"]);
  kiloWattHours = (jsonConfig["kiloWattHours"] == "" ? 0.0 : jsonConfig["kiloWattHours"]);
  beforeResetKiloWattHours = (jsonConfig["beforeResetKiloWattHours"] == "" ? 0.0 : jsonConfig["beforeResetKiloWattHours"]);

  return true;

}

bool saveConfig() {
  DynamicJsonDocument jsonConfig(1024);
  String jsonString;

    // WIFI
  jsonConfig["wifi_name"] = wifi_name;
  jsonConfig["wifi_password"] = wifi_password;
  jsonConfig["ip_mode"] = ipMode;
  jsonConfig["ip"] = ip;
  jsonConfig["mask"] = mask;
  jsonConfig["gateway"] = gateway;
  // MQTT
  jsonConfig["mqtt_enabled"] = mqtt_enabled;
  jsonConfig["mqtt_server"] = mqtt_server;
  jsonConfig["mqtt_port"] = mqtt_port;
  jsonConfig["mqtt_username"] = mqtt_username;
  jsonConfig["mqtt_password"] = mqtt_password;
  // CALIBRATE
  jsonConfig["message_interval"] = message_interval;
  jsonConfig["voltage"] = mainsVoltage;
  jsonConfig["ical"] = Ical;

  // IOT PLATFORMS
  jsonConfig["blynk_enabled"] = blynk_enabled;
  jsonConfig["blynkAuth"] = blynkAuth;
  jsonConfig["blynkServer"] = blynkServer;
  jsonConfig["blynkPort"] = blynkPort;

  jsonConfig["ts_enabled"] = thingSpeak_enabled;
  jsonConfig["tsChannelNumber"] = tsChannelNumber;
  jsonConfig["tsWriteAPIKey"] = tsWriteAPIKey;

  // SYSTEM - TIMEZONE
  jsonConfig["timezone"] = timeZone;
  jsonConfig["minutes_timezone"] = minutesTimeZone;
  // SYSTEM
  jsonConfig["system_password"] = system_password;
  jsonConfig["dayReset"] = dayReset;


  // KWH
  jsonConfig["watiosTotal"] = watiosTotal;
  jsonConfig["kiloWattHours"] = kiloWattHours;
  jsonConfig["beforeResetKiloWattHours"] = beforeResetKiloWattHours;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(jsonConfig, configFile);
  serializeJson(jsonConfig, jsonString);
  Serial.println("SAVED CONFIG: " + jsonString);
  return true;
}
