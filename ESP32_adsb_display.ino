// waveshare 4.3 8MB, PSRAM = 8 OPI
// Adruino GFX 1.5.x - requires ESP32 3.1.x board definitions
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>
#include <U8g2lib.h> //https://github.com/olikraus/u8g2
#include <WiFi.h>

#include "adsb.h"

// ******************************************
// SETUP VALUES HERE
// un comment the wifi lines or create a secrets.h
// *****************************************
#if __has_include("secrets.h")
   #include "secrets.h"
#endif

// const char *ssid = "YOUR WIFI HERE";                    // Replace with your
// const char *password = "YOUR WIFI PASSWORD HERE";  // Replace with
const char *adsbSource = "http://192.168.0.199:8080/data/aircraft.json";
uint16_t screenWidth = 800;
uint16_t screenHeight = 480;
float screenPysicalWidth =    95.26; // units do not matter. need this to calc pixel aspect ratio
float screenPhysicalHeight = 54.3;

float centerLat = 43.386667;
float centerLon = -70.71973;

// ***  add mapping data to mapdata.cpp - airports, coastlines, roads, lakes,
// etc
// ****** END SETUP VALUES ***************

bool showSideBar = false;
unsigned long refreshTime = 0;
float defaultLat = centerLat;
float defaultLon = centerLon;

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    5 /* DE */, 3 /* VSYNC */, 46 /* HSYNC */, 7 /* PCLK */, 1 /* R0 */,
    2 /* R1 */, 42 /* R2 */, 41 /* R3 */, 40 /* R4 */, 39 /* G0 */, 0 /* G1 */,
    45 /* G2 */, 48 /* G3 */, 47 /* G4 */, 21 /* G5 */, 14 /* B0 */,
    38 /* B1 */, 18 /* B2 */, 17 /* B3 */, 10 /* B4 */,

    // Esta configuración es la que mejor funciona de momento
    0 /* hsync_polarity */, 8 /* hsync_front_porch */,
    4 /* hsync_pulse_width */, 8 /* hsync_back_porch */, 0 /* vsync_polarity */,
    8 /* vsync_front_porch */, 4 /* vsync_pulse_width */,
    8 /* vsync_back_porch */, 1 /* pclk_active_neg */,
    14000000 /* prefer_speed */);

Arduino_RGB_Display *gfx =
    new Arduino_RGB_Display(screenWidth /* width */, screenHeight /* height */,
                            rgbpanel, 0 /* rotation */, true /* auto_flush */
    );

bool GFXinit() {
  Serial.println("GFX init...");
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
    return false;
  }
  gfx->fillScreen(0x003030);
  BLset(HIGH);
  gfx->println("Please wait...");
  return true;
}

/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

/*******************************************************************************
 * START TOUCH CONFIG
 ******************************************************************************/
// v 1.2.2 https://github.com/bitbank2/bb_captouch
#include <bb_captouch.h>
BBCapTouch bbct;
const char *szNames[] = {"Unknown", "FT6x36", "GT911", "CST820"};

#define TOUCH_SDA 8
#define TOUCH_SCL 9
#define TOUCH_INT 4
#define TOUCH_RST 0

void TouchInit() {
  Serial.println("Touch init...");
  // Init touch device
  bbct.init(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_INT);
  uint16_t iType = bbct.sensorType();
  Serial.printf("Touch sensor type = %s\n", szNames[iType]);
}

void TouchRead() {
  static unsigned long lastTouch = 0;
  TOUCHINFO thisTouch;
  if (bbct.getSamples(&thisTouch)) { // if touch event happened
    if (millis() - lastTouch > 1000) {
      char buffer[64];
      // snprintf(buffer, sizeof(buffer), "Touched at: X=%d, Y=%d",
      // thisTouch.x[0], thisTouch.y[0]); Serial.println(buffer);
      // Serial.printf("Touch x: %d y: %d size: %d\n", thisTouch.x[0],
      // thisTouch.y[0], thisTouch.area[0]);
      refreshTime = 0;
      lastTouch = millis();
      if (showSideBar) {
        showSideBar = false;
        centerLat = defaultLat;
        centerLon = defaultLon;
        zoom_screen(true);
        Serial.print(" sidebar closed, set lat lon to center on DEFAULT");
        gfx->setTextBound(0, 0, screenWidth, screenHeight);
      } else {
        Aircraft thisAircraft;
        if (isTouchNearPoint(thisTouch.x[0], thisTouch.y[0], 40,  &thisAircraft)) { // 20 pixels radius
          centerLat = thisAircraft.lat;
          centerLon = thisAircraft.lon - (lonRange / 4); // places this in the right side of the screen;
          Serial.print("sidebar opened set lat lon to center on plane");
          showSideBar = true;
          zoom_screen(true);
          refresh_screen();
          show_side_bar(thisAircraft);
          bbct.getSamples(&thisTouch); // clear the touch buffer
          bbct.getSamples(&thisTouch); // clear the touch buffer
        } else {
          zoom_screen(false);
        }
      }
    }
  }
}

/*******************************************************************************
 * END TOUCH CONFIG
 ******************************************************************************/
/******************************************************************************
 * START IO Expander config
 *********************************************************************************/
#include <ESP_IOExpander_Library.h>
ESP_IOExpander *expander;

// Extender Pin define
#define TP_RST 1
#define LCD_BL 2
#define LCD_RST 3
#define SD_CS 4
#define USB_SEL 5

// I2C Pin define
#define I2C_MASTER_NUM 0
#define I2C_MASTER_SDA_IO 8
#define I2C_MASTER_SCL_IO 9

void ExpanderInit() {
  Serial.println("IO expander init...");
  expander = new ESP_IOExpander_CH422G((i2c_port_t)I2C_MASTER_NUM,
                                       ESP_IO_EXPANDER_I2C_CH422G_ADDRESS);

  expander->init();
  expander->begin();
  expander->multiPinMode(TP_RST | LCD_BL | LCD_RST | SD_CS | USB_SEL, OUTPUT);
  BLset(HIGH);
}

void BLset(byte state) { expander->digitalWrite(LCD_BL, state); }

/********************************************************************************
 * END IOExpander Code
 **********************************************************************************/
/*********************************************************************
 *  END OF CONFIG AND INSTALLATION ELEMENTS. 
 * Should not need to edit below here.
******************************************************************** */


uint32_t bufSize;
float screenPixelRatio;

bool isTouchNearPoint(uint16_t touchX, uint16_t touchY, uint16_t radius,
                      Aircraft *matchedAircraft) {
  for (int i = 0; i < aircraftCount; i++) {
    int dx = touchX - aircraftLocations[i].x;
    int dy = touchY - aircraftLocations[i].y;
    int distanceSquared = dx * dx + dy * dy; // Avoid sqrt() for efficiency
    if (distanceSquared <= (radius * radius)) {
      *matchedAircraft = aircraftLocations[i].info;
      return true; // Touch is within radius of a stored point
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("Initialising...");
  TouchInit();
  delay(200);
  screenPixelRatio = (screenPysicalWidth / screenWidth) /
                     (screenPhysicalHeight / screenHeight);
  ExpanderInit();
  bool GFXinitOK = GFXinit();
  if (GFXinitOK) {
    Serial.println("GFX display initialised");
  }
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Prints the assigned IP address

  zoom_screen(false); // does the init calc for screen dimensions
}

void loop() {
  if ((millis() - refreshTime) > 5000) {
    process_ADSB();
    if (!showSideBar) {
      refresh_screen();
    }
    refreshTime = millis();
  }
  vTaskDelay(50 / portTICK_PERIOD_MS);
  TouchRead();
}
