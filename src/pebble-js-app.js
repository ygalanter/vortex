Pebble.addEventListener("ready",
  function(e) {
    // console.log("PebbleKit JS ready!");
  }
);



Pebble.addEventListener("showConfiguration",
  function(e) {
   
    //Load the remote config page
    Pebble.openURL("http://codecorner.galanter.net/pebble/vortex/vortex_config.html");
    
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    
    if (e.response !== '') {
      
      console.log('resonse: ' + decodeURIComponent(e.response));
      
      //Get JSON dictionary
      var settings = JSON.parse(decodeURIComponent(e.response));
      
      console.log(settings);
      
      //Send to Pebble, persist there
      Pebble.sendAppMessage(
          {
            "KEY_SHOW_DIGITAL_TIME": settings.showDigitalTime,
            "KEY_SHOW_BLUETOOTH": settings.showBluetooth,
            "KEY_SHOW_BATTERY": settings.showBattery,
            "KEY_SHOW_DATE": settings.showDate,
            "KEY_SHOW_DOW": settings.showDow,
            "KEY_SHOW_SECONDS": settings.showSeconds,
            "KEY_DISABLE_VORTEX_ANIMATION": settings.disableVortexAnimation
          },
        function(e) {
          console.log("Sending settings data...");
        },
        function(e) {
          console.log("Settings feedback failed!");
        }
      );
    }
  }
);