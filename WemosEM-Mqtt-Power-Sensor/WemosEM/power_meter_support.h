
/*
  Metering power functions
 
  Alfonso C. Alvarez (Alcar), 14nd September 2019

  @author <a href="mailto:alcar21@gmail.com">Alfonso Carlos Alvarez Reyes</a>

  Compile with Arduino 2.4.2
*/

long lastEMRead = 0;  // Last EM Read
long lastMsgMQTT = 0; // Last Message MQTT

void setIcal(float _Ical) {
  Ical = _Ical;
}

void em_calibrate() {

    for (int i = 0; i < 5; i++) {
      emon.calcIrms(NUM_SAMPLES);  // Calculate Irms only
    }
}

// Read information from Clamp
void em_read(bool calibrate) {

  if (calibrate) {
    em_calibrate();
    return;
  }

  long now = millis();
  if (now - lastEMRead > MIN_READ_EM_INTERVAL) {

    lastEMRead = now;

    unsigned long startMillis = millis();
    rmsCurrent = emon.calcIrms(NUM_SAMPLES);  // Calculate Irms only
    unsigned long sampledTime = millis() - startMillis;
  
    rmsPower = rmsCurrent * mainsVoltage;                       // Calculates RMS Power  
    kiloWattHours = kiloWattHours + ((double)rmsPower * ((double)sampledTime/3600000)); // Calculates kilowatt hours used since last reboot. 3600000 = 60*60*1000
    
  }

}

// Read information from Clamp
void em_read() {
  em_read(false); // do not show calibrate by default
}

void em_loop() {


  long now = millis();
  // If enough time since sending last message, or if birth message must be published
  if (now - lastMsgMQTT < message_interval) {
    return;
  }

  lastMsgMQTT = now;

  Status_LED_On;
  em_read();
  Status_LED_Off;

  if (!mqtt_enabled) {
    return;
  }

      // Check if we're still connected to MQTT broker
  if (!mqtt_client.connected()) {
    // reconnect if not
    mqtt_reconnect();
  } 

  
  String payload = build_payload();

  Serial.print("Payload: ");
  Serial.print(payload);
  Serial.print(" -   Reconnected Count: ");
  Serial.print(reconnected_count);
  Serial.printf (" -    Free heap: %u\n", ESP.getFreeHeap ());

  Status_LED_On;
  // Publish a MQTT message with the payload
  if (mqtt_client.publish(mqtt_topic.c_str(), (char*) payload.c_str(), 0)) {
    Serial.print("Published: ");
    Serial.print(mqtt_topic);
    Serial.print(" > ");
    Serial.println(payload);
    mqtt_client.publish(mqtt_topic_status.c_str(), (char*) "online", 0);
    Serial.print("Published: ");
    Serial.print(mqtt_topic_status);
    Serial.print(" > online");
  } else {
    Serial.print("ERROR MQTT Topic not Published: ");
    Serial.print(mqtt_topic);
  }

  Status_LED_Off;

}