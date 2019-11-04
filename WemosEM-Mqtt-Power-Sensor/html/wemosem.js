
// https://jscompress.com/

var postStatus;

(function ($) {
  $.each(['show', 'hide'], function (i, ev) {
    var el = $.fn[ev];
    $.fn[ev] = function () {
      this.trigger(ev);
      return el.apply(this, arguments);
    };
  });
})(jQuery);

function showInfo(message) {
  console.log("INFO: " + message);
  $(".message-info").html(message);
  $(".message-info").show();
  setTimeout(function () {
    $(".message-info").hide();
  }, 4000);
}

function showError(message) {
  console.log("ERROR: " + message);
  $(".message-error").html(message);
  $(".message-error").show();
  setTimeout(function () {
    $(".message-error").hide();
  }, 4000);
}

function prepareWifiMode() {

  if ($('#ipMode').val() == "dhcp") {
    $('.staticIP').css("display", "none");
  } else {
    $('.staticIP').css("display", "");
  }
}

function loadConfig() {
  var postWifi = $.post("/api/loadConfig").done(function (data) {

    // WIFI
    $('#wifi-name').val(data.wifi_ssid);
    $('#wifi-pass').val(data.wifi_password);
    $('#wifi-ipmode').val(data.wifi_mode);
    $('#wifi-ip').val(data.wifi_ip);
    $('#wifi-mask').val(data.wifi_mask);
    $('#wifi-gateway').val(data.wifi_gateway);

    // MQTT
    $('#enableMQTT').prop('checked', data.mqtt_enabled);
    $('#mqtt-ip').val(data.mqtt_server);
    $('#mqtt-port').val(data.mqtt_port);
    $('#mqtt-user').val(data.mqtt_username);
    $('#mqtt-pass').val(data.mqtt_password);

    // CALIBRATE
    $('#voltage').val(data.votalje);
    $('#ical').val(data.ical);
    $('#messageinterval').val(data.messageinterval);

    // IOT PLATFORMS
    $('#enableBlynk').prop('checked', data.blynk_enabled);
    $('#blynk-host').val(data.blynkHost);
    $('#blynk-port').val(data.blynkPort);
    $('#blynk-auth').val(data.blynkAuth);

    $('#enableThingspeak').prop('checked', data.ts_enabled);
    $('#tsChannelNumber').val(data.tsChannelNumber);
    $('#tsWriteAPIKey').val(data.tsWriteAPIKey);

    // SYSTEM - TIMEZONE
    $('#reset-day').val(data.resetDay);
    $('#timezone').val(data.timezone);
    $('#minute-timezone').val(data.minutestimezone);

  }).fail(function (data) {
    showError("Error load configuration of WemosEM");
  });
}

function saveWifi() {

  var postMQTT = $.post("/api/savewifi",
    {
      'wifi-name': $('#wifi-name').val(),
      'wifi-pass': $('#wifi-pass').val(),
      'wifi-ipmode': $('#wifi-ipmode').val(),
      'wifi-ip': $('#wifi-ip').val(),
      'wifi-mask': $('#wifi-mask').val(),
      'wifi-gateway': $('#wifi-gateway').val()
    }).done(function (data) {

      if (!data || data.length == 0) {
        showInfo("Wifi parameters saved");
        if ($('#wifi-ipmode').val() == 1) {
          window.location.href = "http://" + $('#wifi-ip').val();
        }
      } else {
        showError("Error in Wifi saved parameters: " + JSON.stringify(data));
      }

    }).fail(function (data) {
      showError("Error in save Wifi parameteres of WemosEM");
    });
}

function saveMQTT() {

  var postMQTT = $.post("/api/savemqtt",
    {
      mqtt_enabled: ($('#enableMQTT').prop('checked') ? "1" : "0"),
      mqtt_server: $('#mqtt-ip').val(),
      mqtt_port: $('#mqtt-port').val(),
      mqtt_username: $('#mqtt-user').val(),
      mqtt_password: $('#mqtt-pass').val()
    }).done(function (data) {

      if (!data || data.length == 0) {
        showInfo("MQTT parameters saved");
      } else {
        showError("Error in MQTT saved parameters: " + JSON.stringify(data));
      }

    }).fail(function (data) {
      showError("Error in save MQTT parameteres of WemosEM");
    });
}

function saveIOTPlatforms() {

  var postMQTT = $.post("/api/saveIotPlatforms",
    {
      blynk_enabled: ($('#enableBlynk').prop('checked') ? "1" : "0"),
      blynkHost: $('#blynk-host').val(),
      blynkPort: $('#blynk-port').val(),
      blynkAuth: $('#blynk-auth').val(),
      thingSpeak_enabled: ($('#enableThingspeak').prop('checked') ? "1" : "0"),
      tsChannelNumber: $('#tsChannelNumber').val(),
      tsWriteAPIKey: $('#tsWriteAPIKey').val()
    }).done(function (data) {

      if (!data || data.length == 0) {
        showInfo("IOT Platforms parameters saved");
      } else {
        showError("Error in IOT Platforms saved parameters: " + JSON.stringify(data));
      }

    }).fail(function (data) {
      showError("Error in save IOT Platforms parameteres of WemosEM");
    });
}

function saveCalibrate() {
  var postCalibrate = $.post("/api/savecalibrate",
    {
      voltage: $('#voltage').val(),
      ical: $('#ical').val(),
      messageinterval: $('#messageinterval').val()
    }).done(function (data) {

      if (!data || data.length == 0) {
        showInfo("Calibrate settings saved");
      } else {
        showError("Error saving calibrate settings: " + JSON.stringify(data));
      }

    }).fail(function (data) {
      showError("Error saving Calibrate parameteres of WemosEM");
    });
}

function saveSystem() {
  var postSystem = $.post("/api/savesystem",
    { systempassword: $('#systemPassword').val(),
      resetDay: $('#reset-day').val(),  
      timezone: $('#timezone').val(),
      minutestimezone: $('#minute-timezone').val() }).done(function (data) {

      if (!data || data.length == 0) {
        showInfo("System settings saved");
      } else {
        showError("Error saving system settings: " + JSON.stringify(data));
      }

    }).fail(function (data) {
      showError("Error saving system settings of WemosEM");
    });
}

function loadStatus() {

  postStatus = $.post("/api/status").done(function (data) {

      $('.current-value').html(data.current);
      $('.voltage-value').html(data.voltage);
      $('.watios-value').html(data.watios);
      $('.kwh-value').html(data.kwh);
      $('.ical-value').html(data.ical);
      $('.mqttreconnected-value').html(data.mqttreconnected);
      $('.wifidb-value').html(data.wifidb);
      $('.uptime-value').html(data.uptime);
      $('.time-value').html(data.time);
      $('.freemem-value').html(data.freemem);

    }).fail(function (data) {
      console.log("Error in load status of WemosEM");
    }).always(function (data) {
      setTimeout(function () {
        loadStatus();
      }, 2000);
    });

}



$(document).ready(function () {

  $(".see-system-password").mousedown(function () {
    $("#systemPassword").prop('type', 'text');
  });

  $(".see-system-password").mouseup(function () {
    $("#systemPassword").prop('type', 'password');
  });

  $(".see-mqtt-password").mousedown(function () {
    $("#mqtt-pass").prop('type', 'text');
  });

  $(".see-mqtt-password").mouseup(function () {
    $("#mqtt-pass").prop('type', 'password');
  });



  $(".btn-update").click(function () {
    window.location.href = "/WebFirmwareUpgrade";
  });

  var postId = $.post("/api/id").done(function (data) {

    $('.wemos-id').html(data);
  });

  loadStatus();
  
  loadConfig();

  $('.btn-reboot').click(function () {
    var postReboot = $.post("/reboot").done(function () {
      showInfo("reboot success");
    }).fail(function () {
      showError("error in reboot");
    });
  });

  $('.btn-reset').click(function () {
    var postReset = $.post("/reset").done(function () {
      showInfo("reset success");
    }).fail(function () {
      showError("error in reset");
    });
  });

  $(".card-header button.close").click(function() {
    $(".card-option").hide();
  });

  $(".btn-save-wifi").click(function () {
    saveWifi();
  });

  $(".btn-save-mqtt").click(function () {
    saveMQTT();
  });

  $(".btn-save-calibrate").click(function () {
    saveCalibrate();
  });

  $(".btn-save-iot").click(function () {
    saveIOTPlatforms();
  });

  $(".btn-save-system").click(function () {
    saveSystem();
  });

  $('#ipMode').change(function () {
    prepareWifiMode();
  });

  $(".cardWifi").on("show", function () {
    prepareWifiMode();
  });

  $(".btn-wifi").click(function () {
    $(".card-option").hide();
    $(".cardWifi").show();
  });

  $(".btn-mqtt").click(function () {
    $(".card-option").hide();
    $(".cardMQTT").show();
  });

  $(".btn-calibrate").click(function () {
    $(".card-option").hide();
    $(".cardCalibrate").show();
  });

  $(".btn-iot").click(function () {
    $(".card-option").hide();
    $(".cardIOT").show();
  });

  $(".btn-system").click(function () {
    $(".card-option").hide();
    $(".cardSystem").show();
  });

});