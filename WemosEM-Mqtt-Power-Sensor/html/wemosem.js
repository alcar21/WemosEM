
// https://jscompress.com/

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
      setTimeout(function(){
        $(".message-info").hide();
      }, 4000);
    }

    function showError(message) {
      console.log("ERROR: " + message);
      $(".message-error").html(message);
      $(".message-error").show();
      setTimeout(function(){
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

    function loadWIFI() {
      var postWifi = $.post("/api/loadwifi").done(function (data) {

          $('#wifi-name').val(data.wifi_ssid);
          $('#wifi-pass').val(data.wifi_password);
          $('#wifi-ipmode').val(data.wifi_mode);
          $('#wifi-ip').val(data.wifi_ip);
          $('#wifi-mask').val(data.wifi_mask);
          $('#wifi-gateway').val(data.wifi_gateway);

        }).fail(function (data) {
          showError("Error in load WIFI parameteres of WemosEM");
        });
    }

    function loadCalibration() {
      
      var postWifi = $.post("/api/loadcalibrate").done(function (data) {

        $('#voltage').val(data.votalje);
        $('#ical').val(data.ical);
        $('#messageinterval').val(data.messageinterval);
        
      }).fail(function (data) {
        showError("Error in load Calibrate parameteres of WemosEM");
      });

    }
      
    function loadtime() {
      var postWifi = $.post("/api/loadtime").done(function (data) {

        $('#timezone').val(data.timezone);
        $('#minute-timezone').val(data.minutestimezone);
        
      }).fail(function (data) {
        showError("Error in load Calibrate parameteres of WemosEM");
      });
    }
      
    function loadSystem() {

    }

    function loadMQTT() {
      
      var postMQTT = $.post("/api/loadmqtt").done(function (data) {

          $('#enableMQTT').prop('checked', data.mqtt_enabled);
          $('#mqtt-ip').val(data.mqtt_server);
          $('#mqtt-port').val(data.mqtt_port);
          $('#mqtt-user').val(data.mqtt_username);
          $('#mqtt-pass').val(data.mqtt_password);

        }).fail(function (data) {
          showError("Error in load MQTT parameteres of WemosEM");
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
        'wifi-gateway': $('#wifi-gateway').val() } ).done(function (data) {

          loadMQTT();
          if (!data || data.length == 0) {
            showInfo("Wifi parameters saved");
            if ($('#wifi-ipmode').val() == 1) {
              window.location.href =  "http://" + $('#wifi-ip').val();
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
      { mqtt_enabled: ($('#enableMQTT').prop('checked') ? "1" : "0"), 
        mqtt_server: $('#mqtt-ip').val(),
        mqtt_port: $('#mqtt-port').val(),
        mqtt_username: $('#mqtt-user').val(),
        mqtt_password: $('#mqtt-pass').val() } ).done(function (data) {

          loadMQTT();
          if (!data || data.length == 0) {
            showInfo("MQTT parameters saved");
          } else {
            showError("Error in MQTT saved parameters: " + JSON.stringify(data));
          }

        }).fail(function (data) {
          showError("Error in save MQTT parameteres of WemosEM");
        });
    }
    
    function saveCalibrate() {
      var postCalibrate = $.post("/api/savecalibrate",
      { voltage: $('#voltage').val(),
        ical: $('#ical').val(),
        messageinterval: $('#messageinterval').val() } ).done(function (data) {

          loadCalibration();
          if (!data || data.length == 0) {
            showInfo("Calibrate settings saved");
          } else {
            showError("Error saving calibrate settings: " + JSON.stringify(data));
          }

        }).fail(function (data) {
          showError("Error saving Calibrate parameteres of WemosEM");
        });
    }

    function saveTime() {
      var postTime = $.post("/api/savetime",
      { timezone: $('#timezone').val(),
        minutestimezone: $('#minute-timezone').val() } ).done(function (data) {

          loadCalibration();
          if (!data || data.length == 0) {
            showInfo("Timezone settings saved");
          } else {
            showError("Error saving time settings: " + JSON.stringify(data));
          }

        }).fail(function (data) {
          showError("Error saving time settings of WemosEM");
        });
    }

    function saveSystem() {
      var postSystem = $.post("/api/savesystem",
      { systempassword: $('#systemPassword').val() } ).done(function (data) {

          loadSystem();
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
      setInterval(function () {

        var postStatus = $.post("/api/status").done(function (data) {

          $('.current-value').html(data.current);
          $('.voltage-value').html(data.voltage);
          $('.watios-value').html(data.watios);
          $('.kwh-value').html(data.kwh);
          $('.ical-value').html(data.ical);
          $('.curmqttreconnectedrent-value').html(data.mqttreconnected);
          $('.wifidb-value').html(data.wifidb);
          $('.uptime-value').html(data.uptime);
          $('.time-value').html(data.time);
          $('.freemem-value').html(data.freemem);

        }).fail(function (data) {
          console.log("Error in load status of WemosEM");
        });

      }, 2000);
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

      

      $(".btn-update").click(function() {
        window.location.href="/WebFirmwareUpgrade";
      });

      var postStatus = $.post("/api/id").done(function (data) {

        $('.wemos-id').html(data);
      });

      loadStatus();
      loadWIFI();
      loadMQTT();
      loadCalibration();
      loadtime();
      loadSystem();

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

      $(".btn-save-wifi").click(function () {
        saveWifi();
      });

      $(".btn-save-mqtt").click(function () {
        saveMQTT();
      });

      $(".btn-save-calibrate").click(function () {
        saveCalibrate();
      });

      $(".btn-save-time").click(function () {
        saveTime();
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
        $(".card").hide();
        $(".cardWifi").show();
      });

      $(".btn-mqtt").click(function () {
        $(".card").hide();
        $(".cardMQTT").show();
      });
      
      $(".btn-calibrate").click(function () {
        $(".card").hide();
        $(".cardCalibrate").show();
      });
      
      $(".btn-time").click(function () {
        $(".card").hide();
        $(".cardTime").show();
      });
      
      $(".btn-system").click(function () {
        $(".card").hide();
        $(".cardSystem").show();
      });

    });