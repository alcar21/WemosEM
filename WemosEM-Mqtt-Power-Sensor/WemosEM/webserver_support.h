
/*
  Webserver functions and webpages into data directory
 
  Alfonso C. Alvarez (Alcar), 14nd September 2019

  @author <a href="mailto:alcar21@gmail.com">Alfonso Carlos Alvarez Reyes</a>

  Compile with Arduino 2.4.2
*/

#include "data/index.html.gz.h"

void handleRoot() {

  if (system_password.length() > 0 && !httpServer.authenticate(DEFAULT_SYSTEM_USER, system_password.c_str())) {
    return httpServer.requestAuthentication();
  }
  
  httpServer.sendHeader("Content-Encoding", "gzip", true);
  httpServer.send_P(200, "text/html", index_html_gz, index_html_gz_len);
}

void handleStatus() {
  
  httpServer.send(200, "application/json", build_payload());
}

void handleReset() {
  
  Serial.println("FACTORY RESET...");
  httpServer.send(200);
  resetESP = true;
}

void handleReboot() {
  
  Serial.println("Restarting...");
  httpServer.send(200);
  restartESP = true;
}

void handleMQTT() {

  StaticJsonDocument<256> json;
  String jsonString;

  json["mqtt_enabled"] = mqtt_enabled;
  json["mqtt_server"] = mqtt_server.c_str();
	json["mqtt_port"] = mqtt_port;
  json["mqtt_username"] = mqtt_username.c_str();
  json["mqtt_password"] = mqtt_password.c_str();
	
  serializeJson(json, jsonString);
  httpServer.send(200, "application/json", jsonString);
}

void handleSaveMQTT() {

  StaticJsonDocument<256> json;
  String jsonString, _mqtt_enabled, _mqtt_server, _mqtt_port, _mqtt_username, _mqtt_password;

	_mqtt_enabled = httpServer.arg("mqtt_enabled");
  _mqtt_server = httpServer.arg("mqtt_server");
	_mqtt_port = httpServer.arg("mqtt_port");
	_mqtt_username = httpServer.arg("mqtt_username");
	_mqtt_password = httpServer.arg("mqtt_password");

  Serial.println("MQTT Enabled: " + _mqtt_enabled);
  Serial.println("MQTT Server: " + _mqtt_server);
  Serial.println("MQTT Port: " + _mqtt_port);
  Serial.println("MQTT Username: " + _mqtt_username);
  Serial.println("MQTT Password: " + _mqtt_password);
	
	if(_mqtt_enabled.length() > 0 && _mqtt_enabled.length() <= MAXLEN_MQTT_ENABLED) {
    if (_mqtt_enabled.toInt() == 1) {
      mqtt_enabled = true;
    } else {
      mqtt_enabled = false;
    }
  } else {
    json["mqtt_enabled"] = "Error: mqtt enable parameter incorrect";
  }

  if(_mqtt_server.length() > 0 && _mqtt_server.length() <= MAXLEN_MQTT_SERVER) {
    mqtt_server = _mqtt_server;
  } else {
    json["mqtt_server"] = "Error: mqtt server parameter incorrect";
  }

  if(_mqtt_port.length() > 0 && _mqtt_port.length() <= MAXLEN_MQTT_PORT && _mqtt_port.toInt() > 0) {
    mqtt_port = _mqtt_port.toInt();
  } else {
    json["mqtt_port"] = "Error: mqtt port parameter incorrect";
  }

  if(_mqtt_username.length() > 0 && _mqtt_username.length() <= MAXLEN_MQTT_USERNAME) {
    mqtt_username = _mqtt_username;
  } else {
    json["mqtt_username"] = "Error: mqtt username parameter incorrect";
  }

  if(_mqtt_password.length() > 0 && _mqtt_password.length() <= MAXLEN_MQTT_PASSWORD) {
    mqtt_password = _mqtt_password;
  } else {
    json["mqtt_password"] = "Error: mqtt password parameter incorrect";
  }

  if (json.size() == 0 ) {
    initMqtt();
  }

  serializeJson(json, jsonString);
  httpServer.send(200, "application/json", jsonString);

  saveConfig();
}

void handleWIFI() {
  StaticJsonDocument<256> json;
  String jsonString;

  json["wifi_ssid"] = WiFi.SSID();
  json["wifi_password"] = WiFi.psk();
	json["wifi_mode"] = ipMode;
  json["wifi_ip"] = WiFi.localIP().toString();
  json["wifi_mask"] = WiFi.subnetMask().toString();
  json["wifi_gateway"] = WiFi.gatewayIP().toString();
	
  serializeJson(json, jsonString);
  httpServer.send(200, "application/json", jsonString);
}

void handleSaveWifi() {

  StaticJsonDocument<256> json;
  String jsonString, _wifi_name, _wifi_pass, _wifi_ipmode, _wifi_ip, _wifi_mask, _wifi_gateway;
  IPAddress checkIP;

	_wifi_name = httpServer.arg("wifi-name");
  _wifi_pass = httpServer.arg("wifi-pass");
	_wifi_ipmode = httpServer.arg("wifi-ipmode");
	_wifi_ip = httpServer.arg("wifi-ip");
	_wifi_mask = httpServer.arg("wifi-mask");
  _wifi_gateway = httpServer.arg("wifi-gateway");

  Serial.println("WIFI name: " + _wifi_name);
  Serial.println("WIFI pass: " + _wifi_pass);
  Serial.println("WIFI ip mode: " + String(_wifi_ipmode.equals("1") ? "Static" : "DHCP"));
  Serial.println("WIFI ip: " + _wifi_ip);
  Serial.println("WIFI mask: " + _wifi_mask);
  Serial.println("WIFI gateway: " + _wifi_gateway);

  if(_wifi_name.length() > 0) {
    wifi_name = _wifi_name;
  } else {
    json["wifi_name"] = "Error: wifi name parameter incorrect";
  }

  if(_wifi_pass.length() > 0) {
    wifi_password = _wifi_pass;
  } else {
    json["wifi_password"] = "Error: wifi password parameter incorrect";
  }

  if(_wifi_ipmode.length() == 1) {
    ipMode = _wifi_ipmode.toInt();
  } else {
    json["wifi_mode"] = "Error: wifi ip mode parameter incorrect";
  }

  if(_wifi_ip.length() > 0 && checkIP.fromString(_wifi_ip)) {
    ip = _wifi_ip;
  } else {
    json["wifi_ip"] = "Error: wifi ip parameter incorrect";
  }
  
  if(_wifi_mask.length() > 0 && checkIP.fromString(_wifi_mask)) {
    mask = _wifi_mask;
  } else {
    json["wifi_mask"] = "Error: wifi mask parameter incorrect";
  }

  if(_wifi_gateway.length() > 0 && checkIP.fromString(_wifi_gateway)) {
    gateway = _wifi_gateway;
  } else {
    json["wifi_gateway"] = "Error: wifi gateway parameter incorrect";
  }

  serializeJson(json, jsonString);
  httpServer.send(200, "application/json", jsonString);

  saveConfig();
  if (json.size() == 0 ) {
    setupWifi();
  }

}

void handleCalibrate() {
  StaticJsonDocument<256> json;
  String jsonString;

  json["votalje"] = mainsVoltage;
  json["ical"] = Ical;
  json["messageinterval"] = message_interval;
	
  serializeJson(json, jsonString);
  httpServer.send(200, "application/json", jsonString);
}

void handleSaveCalibrate() {

  StaticJsonDocument<256> json;
  String jsonString, _voltage, _ical, _message_interval;

  _voltage = httpServer.arg("voltage");
  _ical = httpServer.arg("ical");
   _message_interval = httpServer.arg("messageinterval");

  Serial.print("WEB voltage SIZE: ");
  Serial.print( _voltage.length());
  Serial.print(" voltage value: ");
  Serial.println( _voltage.toFloat());
  if(_voltage.length() > 0 && _voltage.length() <= MAXLEN_VOLTAGE && _voltage.toFloat() > 0) {
    mainsVoltage = _voltage.toFloat();
  } else {
    json["voltage"] = "Error: voltage value incorrect";
  }

  Serial.print("WEB ical SIZE: ");
  Serial.print( _ical.length());
  Serial.print(" ical value: ");
  Serial.println( _ical.toFloat());
 if(_ical.length() > 0 && _ical.length() <= MAXLEN_ICAL && _ical.toFloat() > 0) {
    Ical = _ical.toFloat();
  } else {
    json["ical"] = "Error: ical value incorrect";
  }

  // Reconfigure monitor
	if (json.size() == 0 ) {
    emon.current(A0, Ical);
    em_read(true);
  }

  Serial.print("WEB message interval SIZE: ");
  Serial.print( _message_interval.length());
  Serial.print(" message interval value: ");
  Serial.println( _message_interval.toInt());
 if(_message_interval.length() > 0 && _message_interval.length() <= MAXLEN_MESSAGE_INTERVAL && _message_interval.toInt() >= MIN_MESSAGE_INTERVAL) {
    message_interval = _message_interval.toInt();
  } else {
    json["messageinterval"] = "Error: message interval value incorrect";
  }

  serializeJson(json, jsonString);
  httpServer.send(200, "application/json", jsonString);

  saveConfig();
}

void handleTime() {
  StaticJsonDocument<256> json;
  String jsonString;

  json["timezone"] = timeZone;
  json["minutestimezone"] = minutesTimeZone;
	
  serializeJson(json, jsonString);
  httpServer.send(200, "application/json", jsonString);
}


void handleSaveTime() {

  StaticJsonDocument<256> json;
  String jsonString, _timezone, _minutestimezone;

  _timezone = httpServer.arg("timezone");
  _minutestimezone = httpServer.arg("minutestimezone");

  Serial.print("WEB timezone SIZE: ");
  Serial.print( _timezone.length());
  Serial.print(" timezone value: ");
  Serial.println( _timezone.toInt());
  if(_timezone.length() > 0 && _timezone.length() <= MAXLEN_TIMEZONE && _timezone.toInt() >= 0) {
    timeZone = _timezone.toInt();
  } else {
    json["timezone"] = "Error: timezone value incorrect (-14-+14)";
  }

  Serial.print("WEB minutestimezone SIZE: ");
  Serial.print( _minutestimezone.length());
  Serial.print(" minutestimezone value: ");
  Serial.println( _minutestimezone.toInt());
 if(_minutestimezone.length() > 0 && _minutestimezone.length() <= MAXLEN_TIMEZONE && _minutestimezone.toInt() >= 0) {
    minutesTimeZone = _minutestimezone.toInt();
  } else {
    json["minutesTimeZone"] = "Error: minutes timezone value incorrect (0-60)";
  }

  // Reconfigure NTP client
  wifiFirstConnected = true;

  serializeJson(json, jsonString);
  httpServer.send(200, "application/json", jsonString);

  saveConfig();
}

void handleSaveSystem() {

  StaticJsonDocument<256> json;
  String jsonString, _systempassword;

	_systempassword = httpServer.arg("systempassword");

  Serial.println("Password: " + _systempassword);

  if(_systempassword.length() <= MAXLEN_SYSTEM_PASSWORD) {
    system_password = _systempassword;
  } else {
    json["password"] = "Error: password system is incorrect > 30 characters";
  }

  serializeJson(json, jsonString);
  httpServer.send(200, "application/json", jsonString);

  saveConfig();

  if (json.size() == 0 ) {
    restartESP = true;
  }
}

void handleNotFound() {
  
  String message = "WemosEM File Not Found\n\n";
  message += "URI: ";
  message += httpServer.uri();
  message += "\nMethod: ";
  message += (httpServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += httpServer.args();
  message += "\n";
  for (uint8_t i = 0; i < httpServer.args(); i++) {
    message += " " + httpServer.argName(i) + ": " + httpServer.arg(i) + "\n";
  }
  httpServer.send(404, "text/plain", message);
  
}


void setup_http_server() {
  
  // Setup http firmware update page.
  if (MDNS.begin(wifi_hostname.c_str(), WiFi.localIP())) {
    Serial.println("MDNS Started");
  } else {
    Serial.println("MDNS started FAILED");
  }
  httpUpdater.setup(&httpServer, update_path, DEFAULT_SYSTEM_USER, (char *) system_password.c_str());

  httpServer.on("/", handleRoot);

  httpServer.on("/api/status", handleStatus);
  
  httpServer.on("/api/loadwifi", handleWIFI);
  httpServer.on("/api/loadmqtt", handleMQTT);
  httpServer.on("/api/loadcalibrate", handleCalibrate);
  httpServer.on("/api/loadtime", handleTime);

  httpServer.on("/api/savewifi", handleSaveWifi);
  httpServer.on("/api/savemqtt", handleSaveMQTT);
  httpServer.on("/api/savecalibrate", handleSaveCalibrate);
  httpServer.on("/api/savetime", handleSaveTime);
  httpServer.on("/api/savesystem", handleSaveSystem);
  

  httpServer.on("/api/id", [](){
    String id = wifi_hostname.substring(0, 1);
    id.toUpperCase();
    id.concat(wifi_hostname.substring(1));
    httpServer.send(200, "text/plain", id);
  });

  httpServer.on("/reset", handleReset);
	httpServer.on("/reboot", handleReboot);
  

  httpServer.onNotFound(handleNotFound);
  
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and your password\n", wifi_hostname.c_str(), update_path, DEFAULT_SYSTEM_USER);
}