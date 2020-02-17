/*
  MQTT with ESP8266 and Power Monitor 5th December 2017
  David Mottram
  Updated by Alfonso C. Alvarez (Alcar), 14nd September 2019

  @author <a href="mailto:alcar21@gmail.com">Alfonso Carlos Alvarez Reyes</a>

  Compile with Arduino 2.4.2
*/

#include <ESP8266WiFi.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <NtpClientLib.h>
#include <EmonLib.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp8266.h>
#include <ThingSpeak.h>
#include <ArduinoOTA.h>

#include "WiFiClientPrint.h"
#include "variables.h"
#include "settings.h"
#include "ntp_support.h"
#include "mqtt_wifi_serial_support.h"
#include "iot_platforms_support.h"
#include "power_meter_support.h"
#include "webserver_support.h"
#include "MPU6050_motion.h"

// Initial setup
void setup(void) {

  delay(10);

  prepareHostMacAndEvents();

  pinMode(Status_LED, OUTPUT);                // Initialize Status LED
  Status_LED_Off;

  initSerial();

  loadConfig();

  setupWifi();

  // emon.current(A0, Ical);             // Current: input pin A6=D4, calibration factor

  initMqtt();

  randomSeed(micros());

  ms_since_last_message = millis();           // Reset time since sending last message

  setup_http_server();

  setup_blynk();

  setup_thingspeak();

  setupOTA();

  #ifdef MOTION
  setupMPU6050();
  #endif

  emon.current(A0, Ical);
  em_read(true);

  lastMsgMQTT = millis();
  lastEMRead = millis();
  Serial.println("Setup finished");
} // End of setup


// main loop
void loop() {

  ArduinoOTA.handle();

  ntp_loop();

  #ifdef MOTION
  loopMPU6050();
  #endif

  em_loop();

  mqtt_client.loop();
  httpServer.handleClient(); //handles requests for the firmware update page

  if (resetESP) {
    WiFi.persistent(true); // use SDK storage of SSID/WPA parameters
    WiFi.disconnect(); // this will store empty ssid/wpa into sdk storage
    WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
    Serial.println("formatting...");
    SPIFFS.format();
    ESP.restart();
    delay(100);
  }

  if (restartESP) {
    ESP.restart();
    delay(100);
  }

} // End of main loop
