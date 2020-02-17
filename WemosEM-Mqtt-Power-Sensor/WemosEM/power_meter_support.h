
/*
  Metering power functions

  Alfonso C. Alvarez (Alcar), 14nd September 2019

  @author <a href="mailto:alcar21@gmail.com">Alfonso Carlos Alvarez Reyes</a>

  Compile with Arduino 2.4.2
*/

long lastEMRead = 0;  // Last EM Read
long lastMsgMQTT = 0; // Last Message MQTT

void resetKwh() {

  beforeResetKiloWattHours = kiloWattHours;
  watiosTotal = 0.0;
  kiloWattHours = 0.0;
}



void setIcal(float _Ical) {
  Ical = _Ical;
}

void em_calibrate() {

    for (int i = 0; i < 5; i++) {
      emon.calcIrms(NUM_SAMPLES);  // Calculate Irms only
    }

    if (lastTimeMeasure == 0) {
      lastTimeMeasure = millis();
    }
}

// Read information from Clamp
void em_read(bool calibrate) {

  if (calibrate) {
    em_calibrate();
    return;
  }

  unsigned long now = millis();
  if (now - lastEMRead > MIN_READ_EM_INTERVAL) {

    lastEMRead = now;

    rmsCurrent = emon.calcIrms(NUM_SAMPLES);  // Calculate Irms only

    rmsPower = rmsCurrent * mainsVoltage;                       // Calculates RMS Power
    watiosTotal += ((double)rmsPower * ((millis()-lastTimeMeasure) / 1000.0)/3600.0); // Calculates kilowatt hours used since last reboot. 3600 = 60min*60sec / 1000.0 watios = kwh

    kiloWattHours = watiosTotal / 1000.0;

    /*
    Serial.print(" [METER] - rmsCurrent: ");
    Serial.print(rmsCurrent);
    Serial.print(" mainsVoltage: ");
    Serial.print(mainsVoltage);
    Serial.print(" Watios: ");
    Serial.print(watiosTotal);

    Serial.print(" kwh: ");
    Serial.print(kiloWattHours);
    Serial.print(" Before reset kwh: ");
    Serial.print(beforeResetKiloWattHours);
    Serial.print(" lastTimeMeasure: ");
    Serial.println(lastTimeMeasure);
    */

    // If not AP mode
    // if (isSTA()) {
      // saveConfig();
    // }

    lastTimeMeasure = millis();
  }

}

// Read information from Clamp
void em_read() {
  em_read(false); // do not show calibrate by default
}

void em_loop() {

  if (isResetEnergyToday && day(now()) != dayReset) {

    isResetEnergyToday = false;
  } else if (!isResetEnergyToday && day(now()) == dayReset) {

    resetKwh();
    isResetEnergyToday = true;
  }

  unsigned long now = millis();
  // If enough time since sending last message, or if birth message must be published
  if (now - lastMsgMQTT < message_interval) {
    return;
  }

  lastMsgMQTT = now;

  Status_LED_On;
  em_read();
  Status_LED_Off;

  /*
  blynk_loop();
  thinkgSpeak_loop();
  */

  if (!mqtt_enabled) {
    return;
  }

      // Check if we're still connected to MQTT broker
  if (!mqtt_client.connected()) {
    // reconnect if not
    initMqtt();
  }

  String payload = build_payload();

  Serial.print(" [METER] - Payload: ");
  Serial.println(payload);

  Status_LED_On;
  // Publish a MQTT message with the payload
  if (mqtt_client.publish(mqtt_topic.c_str(), (char*) payload.c_str(), 0)) {
    /*
    Serial.print(" [MQTT] - Published: ");
    Serial.print(mqtt_topic);
    Serial.print(" > ");
    Serial.println(payload);
    */
    mqtt_client.publish(mqtt_topic_status.c_str(), (char*) "online", 0);
    /*
    Serial.print(" [MQTT] - Published: ");
    Serial.print(mqtt_topic_status);
    Serial.println(" > online");
    */
  } else {
    Serial.print("ERROR MQTT Topic not Published: ");
    Serial.println(mqtt_topic);
  }

  Status_LED_Off;


}
