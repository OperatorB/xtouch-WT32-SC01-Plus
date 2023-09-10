#include <Arduino.h>
#include <ArduinoJson.h>
#include "xtouch/eeprom.h"
#include "xtouch/paths.h"
#include "xtouch/types.h"
#include "xtouch/globals.h"
#include "xtouch/filesystem.h"
#include "ui/ui.h"
#include "xtouch/sdcard.h"
#include "xtouch/hms.h"
#include "xtouch/ssdp.h"

#if defined(__XTOUCH_SCREEN_28__)
#include "devices/2.8/screen.h"
#endif

#include "xtouch/pair.h"
#include "xtouch/net.h"
#include "xtouch/firmware.h"
#include "xtouch/mqtt.h"
#include "xtouch/settings.h"
#include "xtouch/sensors/chamber.h"
#include "xtouch/events.h"
#include "xtouch/connection.h"

void xtouch_intro_show(void)
{
  ui_introScreen_screen_init();
  lv_disp_load_scr(introScreen);
  lv_timer_handler();
}

void setup()
{

  Serial.begin(115200);

  xtouch_eeprom_setup();
  xtouch_globals_init();
  xtouch_screen_setup();
  xtouch_intro_show();

  while (!xtouch_sdcard_setup())
    ;

  xtouch_settings_setup();

  xtouch_firmware_checkFirmwareUpdate();

  xtouch_touch_setup();

  while (!xtouch_wifi_setup())
    ;

  xtouch_firmware_checkOnlineFirmwareUpdate();

  xtouch_screen_setupScreenTimer();
  xtouch_setupGlobalEvents();
  xtouch_pair_check();
  xtouch_mqtt_setup();
  xtouch_chamber_timer_init();
}

void loop()
{
  lv_timer_handler();
  lv_task_handler();
  xtouch_mqtt_loop();
}
