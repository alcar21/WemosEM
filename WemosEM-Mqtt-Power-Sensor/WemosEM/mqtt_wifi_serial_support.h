/*
  MQTT, Wifi and serial functions

  Alfonso C. Alvarez (Alcar), 14nd September 2019

  @author <a href="mailto:alcar21@gmail.com">Alfonso Carlos Alvarez Reyes</a>

  Compile with Arduino 2.4.2
*/


String WifiGetRssiAsQuality(int rssi)
{
  String qualityStr = "0% No signal";
  int quality = 2 * (rssi + 100);

  if (rssi <= -100) {
    qualityStr = "0% No signal";
  } else if (rssi >= -50) {
    qualityStr = "100% Excelent";
  } else if (rssi >= -60) { 
    qualityStr = String(quality) + "% Good";
  } else if (rssi >= -70) { 
    qualityStr = String(quality) + "% Poor";
  } else if (rssi > -80) { 
    qualityStr = String(quality) + "% Bad";
  }  else {
    qualityStr = String(quality) + "% Very weak";
  }
  return qualityStr;
}

void callbackMqtt(char* topic, byte* payload, unsigned int length) {
  
  char topicWemosEM[25];

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  char* isTopic = strstr(topic, mqtt_topic_prefix_subscribe.c_str() );
  if (!isTopic) {
    return;
  }

  String command = String(strrchr(topic, '/') + 1);

  String s_payload = String((char *)payload);
  
  if(command.equals(TOPIC_VOLTAGE)) {
    Serial.println("Setting MQTT voltage " + s_payload);
    
    if (s_payload.length() > 0 && s_payload.toFloat() > 0) {
      mainsVoltage = s_payload.toFloat();
    }
  }

  if(command.equals(TOPIC_STATUS)) {
    Serial.println("Processing MQTT status ");
    mqtt_client.publish(mqtt_topic_status.c_str(), (char*) "online");
  }

}

// Compose a payload to return as required
// The payload and headers cannot exceed 128 bytes!
String build_payload() {

  StaticJsonDocument<256> json;
  String jsonString;

  // WiFi signal strength in dB
  String rssi = WifiGetRssiAsQuality(WiFi.RSSI());

  json["current"] = String(rmsCurrent);
  json["voltage"] = String(mainsVoltage);
  json["watios"] = String(rmsPower);
  json["kwh"] = String(kiloWattHours);
  json["ical"] = String(Ical);
  json["mqttreconnected"] = String(reconnected_count);
  json["wifidb"] = rssi;
  json["uptime"] = NTP.getUptimeString ();
  json["time"] = NTP.getTimeDateString();
  json["freemem"] = ESP.getFreeHeap();

  serializeJson(json, jsonString);

  return jsonString;
}

// SERIAL FUNCTIONS
void initSerial() {

  Serial.begin(115200);
  Serial.println(""); 
  Serial.println(""); 
  Serial.println("* Starting up *");
}

void prepareHostMacAndEvents() {

  gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event) {
    Serial.print("Station connected, IP: ");
    Serial.println(WiFi.localIP());
    wifiFirstConnected = true;
  });

  disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event)
  {
    Serial.println("Station disconnected");
    wifiFirstConnected = false;
  });
  
  // Get MAC address of ESP8266, 6 bytes in an array
  byte mac[6]; 
  WiFi.macAddress(mac);

  My_MAC = "";
  // Build a string of the MAC with "0" padding for each byte, and upper case
  for (int i = 0; i <= 5; i++) {
    String B = String(mac[i], HEX);
    B.toUpperCase();
    if (B.length() < 2) {
      // Pad with leading zero if needed
      B = "0" + B;
    }
    My_MAC += B;
  } // End of for
  wifi_hostname = String(HOSTNAME_PREFIX);
  wifi_hostname.concat(My_MAC.substring(6, 12));
}

// Try to connect to any of the WiFi networks configured in Custom_Settings.h
void setupWifi() {

  //Set the wifi config portal to only show for 3 minutes, then continue.
  WiFi.hostname(wifi_hostname.c_str());
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  wifiManager.setConfigPortalTimeout(180);
  Serial.println(" Wifi " + wifi_hostname + ", password " + system_password);
  wifiManager.autoConnect(wifi_hostname.c_str(), (char *) system_password.c_str());

  if (ipMode == 1) {
    IPAddress ipa_ip, ipa_gateway, ipa_subnet;
    if (ipa_ip.fromString(ip) && ipa_gateway.fromString(gateway) &&  ipa_subnet.fromString(mask)) {
      // Static IP Setup
      WiFi.config(ipa_ip, ipa_gateway, ipa_subnet);
      Serial.println(" using Static IP " + String(ip));
    } else {
      Serial.println("Error in static parameters, using DHCP");
    }
  } else {
    Serial.println(" using DHCP");
  }

}

void discoverHA() {

  char topic[60], message[600];

  // Current (A)
  Serial.println("Preparing topic HA Discover");
  sprintf_P(topic, TOPIC_HA_CURRENT, wifi_hostname.c_str() );
  Serial.println("Preparing message HA Discover");
  sprintf_P(message, MESSAGE_HA_CURRENT, wifi_hostname.c_str(), wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str() );
  mqtt_client.publish(topic, message, true);
  
  // Power (watios)
  sprintf_P(topic, TOPIC_HA_POWER, wifi_hostname.c_str() );
  sprintf_P(message, MESSAGE_HA_POWER, wifi_hostname.c_str(), wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str() );
  mqtt_client.publish(topic, message, true);

  // Power (kwh)
  sprintf_P(topic, TOPIC_HA_KWH, wifi_hostname.c_str() );
  sprintf_P(message, MESSAGE_HA_KWH, wifi_hostname.c_str(), wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str() );
  mqtt_client.publish(topic, message, true);

}

void initMqtt() {

  String mqtt_topic_prefix = "wemos/" + wifi_hostname + "/";
  
  mqtt_topic_prefix_subscribe = "wemos-cmd/" + wifi_hostname;
  mqtt_topic_subscribe = mqtt_topic_prefix_subscribe + "/#";
  mqtt_topic = mqtt_topic_prefix + "power";
  mqtt_topic_status = mqtt_topic_prefix + "status";
  
  mqtt_client.setServer(mqtt_server.c_str(), mqtt_port);
  mqtt_client.setCallback(callbackMqtt);

  if (WiFi.status() != WL_CONNECTED) {
      // Attempt to connect
      setupWifi();
  }

  int lwQoS = 1; // send last will at least once
  int lwRetain = 1;
  String lwPayload = "offline";
  bool cleanSession = true;
  int connected = false;

  if (mqtt_enabled) {
    Serial.println("Connecting Mqtt...");  
    if (mqtt_username.length() > 0 && mqtt_password.length() > 0) {
        Serial.println("Connecting MQTT with user/pass");
        connected = mqtt_client.connect(wifi_hostname.c_str(), (char*)mqtt_username.c_str(), (char*)mqtt_password.c_str(), mqtt_topic_status.c_str(), lwQoS, lwRetain, (char*)lwPayload.c_str(), cleanSession);
    } else {
        connected = mqtt_client.connect(wifi_hostname.c_str(), mqtt_topic_status.c_str(), lwQoS, lwRetain, (char*)lwPayload.c_str());
    }

    
    if (connected) {    
        Serial.println(" MQTT Connected. ");
        mqtt_client.subscribe((char *)mqtt_topic_subscribe.c_str());
        // Discover Notify Home Assistant
       discoverHA();
    } else {
      Serial.println("failed, rc=" + String(mqtt_client.state()) + " Try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }

  } else {
    Serial.println("MQTT Connection is disabled...");  
  }
}

bool mqtt_reconnect() {

  if (!mqtt_enabled) {
    return false;
  }
  Serial.print("Status MQTT: ");
  Serial.print(mqtt_client.state());
  Serial.println(" Reconnecting...");
  ++reconnected_count;

  // Loop until we're reconnected
  // (or until max retries)

  if (WiFi.status() != WL_CONNECTED) {
    // Attempt to connect
    setupWifi();
  }

  Serial.print(" Attempting MQTT Broker connection... ");
  int connected = false;
  
  int lwQoS = 1; // send last will at least once
  int lwRetain = 1;
  String lwPayload = "Offline";
  bool cleanSession = true;

  if (mqtt_username.length() > 0 && mqtt_password.length() > 0) {
      Serial.println("Reconnecting with user/pass");
      connected = mqtt_client.connect(wifi_hostname.c_str(), (char*)mqtt_username.c_str(), (char*)mqtt_password.c_str(), mqtt_topic_status.c_str(), lwQoS, lwRetain, (char*)lwPayload.c_str(), cleanSession);
  } else {
      connected = mqtt_client.connect(wifi_hostname.c_str(), mqtt_topic_status.c_str(), lwQoS, lwRetain, (char*)lwPayload.c_str());
  }

  if (connected) {    
    Serial.println(" MQTT Connected. ");
    mqtt_client.subscribe((char *)mqtt_topic_subscribe.c_str());
    discoverHA();
  } else {
    Serial.println("failed, rc=" + String(mqtt_client.state()) + " Try again...");
  }

  return mqtt_client.connected();
  
} // End of reconnect


