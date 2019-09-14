
/*
  NTP setup and operations
 
  Alfonso C. Alvarez (Alcar), 14nd September 2019

  @author <a href="mailto:alcar21@gmail.com">Alfonso Carlos Alvarez Reyes</a>

  Compile with Arduino 2.4.2
*/

void processSyncEvent (NTPSyncEvent_t ntpEvent) {
  
    if (ntpEvent < 0) {
        Serial.printf ("Time Sync error: %d\n", ntpEvent);
        if (ntpEvent == noResponse)
            Serial.println ("NTP server not reachable");
        else if (ntpEvent == invalidAddress)
            Serial.println ("Invalid NTP server address");
        else 
            Serial.println ("Error sending request or NTP response error");
    } else {
        if (ntpEvent == timeSyncd) {
            Serial.print ("Got NTP time: ");
            Serial.println (NTP.getTimeDateString (NTP.getLastNTPSync ()));
        }
    }
}

void setup_NTP() {
  
  NTP.onNTPSyncEvent ([](NTPSyncEvent_t event) {
      ntpEvent = event;
      syncEventTriggered = true;
  });
}

void ntp_loop() {

  if (wifiFirstConnected) {
        wifiFirstConnected = false;
        // NTP.setInterval (63);
        NTP.begin (ntpServer, timeZone, true, minutesTimeZone);
    }

  if (syncEventTriggered) {
      processSyncEvent (ntpEvent);
      syncEventTriggered = false;
  }
}

void print_ntp_time() {
  Serial.print (NTP.getTimeDateString ()); Serial.print (" ");
  Serial.print (NTP.isSummerTime () ? "Summer Time. " : "Winter Time. ");
  Serial.print ("Uptime: ");
  Serial.print (NTP.getUptimeString ()); Serial.print (" since ");
  Serial.println (NTP.getTimeDateString (NTP.getFirstSync ()).c_str ());
}

