#include <lvgl.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ESPmDNS.h>  // include MDNS

#define FRONT_SW 27

// WIFI MANAGER SETUP
WiFiManager wm;
unsigned int  timeout   = 120; // seconds to run for
unsigned int  startTime = millis();
bool portalRunning      = false;
bool startAP            = false; // start AP and webserver if true, else start only webserver

//TFT STARTUP
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

// STARTUP ANIMATION CONFIG
//startup animation sweep time.
//delay before starting animation
#define STARTUP_GAUGE_SWEEP_DELAY_MS 500
//time in seconds to sweep 0-100
#define STARTUP_GAUGE_SWEEP_TIME_ONE_WAY_MS 1000

//lvgl objects
lv_obj_t *screenMain;
lv_obj_t *gauge1;

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors(&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

void update_ui(lv_task_t* task) {
  long ms_since_start = millis();
  
  // if we are still in the delay state
  if (ms_since_start<STARTUP_GAUGE_SWEEP_DELAY_MS) {
    lv_gauge_set_value(gauge1, 0, 0);
  // if we are in the sweep up state
  } else if (ms_since_start<(STARTUP_GAUGE_SWEEP_DELAY_MS + STARTUP_GAUGE_SWEEP_TIME_ONE_WAY_MS)){
    long ms_since_start_sweep = ms_since_start - STARTUP_GAUGE_SWEEP_DELAY_MS;
    int value_0_100 = 100*ms_since_start_sweep/STARTUP_GAUGE_SWEEP_TIME_ONE_WAY_MS;
    lv_gauge_set_value(gauge1, 0, value_0_100);
  // if we are in the sweep down state
  } else if (ms_since_start<(STARTUP_GAUGE_SWEEP_DELAY_MS + 2*STARTUP_GAUGE_SWEEP_TIME_ONE_WAY_MS)){
    long ms_since_start_sweep = ms_since_start - STARTUP_GAUGE_SWEEP_DELAY_MS - STARTUP_GAUGE_SWEEP_TIME_ONE_WAY_MS;
    int value_0_100 = 100-(100*ms_since_start_sweep/STARTUP_GAUGE_SWEEP_TIME_ONE_WAY_MS);
    lv_gauge_set_value(gauge1, 0, value_0_100);
  } else{
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n gauge booted");

  
  pinMode(FRONT_SW, INPUT_PULLUP);
  
  lv_init();
  
  tft.init();
  tft.setRotation(0);

  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 240;
  disp_drv.ver_res = 240;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  //lvgl task to update UI from live value
  lv_task_create(&update_ui, 15, 3, nullptr);

  // create screen objects
  
  screenMain = lv_obj_create(NULL, NULL);

  //gauge
  static lv_color_t needle_colors[0];
  needle_colors[0] = LV_COLOR_PURPLE;

  gauge1 = lv_gauge_create(screenMain, NULL);
  lv_gauge_set_needle_count(gauge1, 1, needle_colors);
  lv_obj_set_size(gauge1, 240, 240);
  lv_obj_align(gauge1, NULL, LV_ALIGN_CENTER, 0, 0);
  
  lv_scr_load(screenMain);

  // WiFi manager startup
  // wm.resetSettings();
  wm.setHostname("MDNSEXAMPLE");
  // wm.setEnableConfigPortal(false);
  // wm.setConfigPortalBlocking(false);
  wm.autoConnect();
  
}

void loop() {
  lv_task_handler(); /* let the GUI do its work */
  delay(5);

  //wifimanager
  #ifdef ESP8266
  MDNS.update();
  #endif
  doWiFiManager();
}
