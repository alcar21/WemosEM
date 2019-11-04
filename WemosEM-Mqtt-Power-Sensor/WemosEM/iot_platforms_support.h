

/*
  IOT Plattform integration
 
  Alfonso C. Alvarez (Alcar), 14nd September 2019

  @author <a href="mailto:alcar21@gmail.com">Alfonso Carlos Alvarez Reyes</a>

  Compile with Arduino 2.4.2
*/

BLYNK_CONNECTED() {
    Blynk.syncAll();
}

void setup_blynk() {
  
  if (blynk_enabled && blynkAuth.length() > 0 && blynkAuth != "null") {
    Serial.println("[BLYNK] Enabled");
    if (blynkServer.length() > 0 && blynkPort != 0 ) {
      Serial.println("[BLYNK] Custom server connecting...");
      Blynk.config(blynkAuth.c_str(), blynkServer.c_str(), blynkPort);
    } else {
      Blynk.config(blynkAuth.c_str());
    } 
  }
}

void blynk_loop() {

  if (!blynk_enabled) {
    Serial.println("[BLYNK] Disabled");  
    return;
  }

  unsigned long init = millis();
  Serial.println("[BLYNK] Sending data");
  
  if (Blynk.connected()) { 
    Blynk.run(); 
  } else { 
    Blynk.connect(); 
  }

  Blynk.virtualWrite(VPIN_VOLTAGE, mainsVoltage);
  Blynk.virtualWrite(VPIN_CURRENT, rmsCurrent);
  Blynk.virtualWrite(VPIN_WATIOS, rmsPower);
  Blynk.virtualWrite(VPIN_ENERGY, kiloWattHours);
  Blynk.virtualWrite(VPIN_ENERGY_LAST_RESET, beforeResetKiloWattHours);
  
  Serial.println("[BLYNK] End sending data. time: " + String(millis() - init) + "ms");
}

void setup_thingspeak() {
  
  if (thingSpeak_enabled) {
    Serial.println("[THINGSPEAK] Enabled");
    ThingSpeak.begin(espClient);
  }
}


void thinkgSpeak_loop() {

  if (!thingSpeak_enabled) {
    Serial.println("[THINGSPEAK] Disabled");
    return;
  }

  if ((millis() - lastTimeMeasureTS) < TIME_BEETWEEN_REQUEST) {
    return;
  }

  lastTimeMeasureTS = millis();

  unsigned long init = millis();
  Serial.println("[THINGSPEAK] Sending data");
  int statusTS;

  ThingSpeak.setField(1, mainsVoltage);
  ThingSpeak.setField(2, (float)rmsCurrent);
  ThingSpeak.setField(3, (float)rmsPower);
  ThingSpeak.setField(4, (float)kiloWattHours);
  ThingSpeak.setField(5, (float)beforeResetKiloWattHours);
  
  statusTS = ThingSpeak.writeFields(tsChannelNumber, tsWriteAPIKey.c_str());

  Serial.println("[THINGSPEAK] Status: " + String(statusTS));
  Serial.println("[THINGSPEAK] End sending data. time: " + String(millis() - init) + "ms");

}