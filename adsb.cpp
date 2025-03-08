#include <U8g2lib.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
HTTPClient http;

WiFiClient client;  // or WiFiClientSecure for HTTPS

#include <math.h>
#include "adsb.h"
#include "maps.h"
#include "mapdata.h"
JsonDocument thisADSB;
#define LARGEFONT u8g2_font_profont29_tr
#define MEDIUMFONT u8g2_font_profont22_tr
#define SMALLFONT u8g2_font_profont22_tr

float latMax;    // top of the screen display
float latMin;    // bottom
float lonMax;    // right  side (east side)
float lonMin;    // left side (west)
float latRange;  // how tall  is the display area
float lonRange;  // how wide is the display area
float scaleX;    //
float scaleY;

float viewRadius = 100;  //not a valid range, just forces the radius to default at setup
float pixelsPerMileY = 0;
float pixelsPerMileX = 0;

struct Track {
  float lat;
  float lon;
  uint16_t altColor;
  uint16_t time;
};

/*

// Example JSON data (truncated for brevity)
char *testData = R"(
{
  "now": 1739038368.000,
  "messages": 981379,
  "aircraft": [
        { "hex": "a1cb0f", "type": "adsb_icao",   "flight": "UAL988  ",   "r": "N2142U",   "t": "B77W",   "desc": "BOEING 777-300ER", "alt_baro": 34000, "alt_geom": 33400,
         "gs": 397.5, "track": 231.03, "baro_rate": 0, "category": "A5",  "nav_qnh": 1012.8,
        "lat": 42.951050000000002, "lon": -71.420968999999999,   "alert": 0,  "messages": 5626, "seen": 3.1000000000000001, "rssi": -28.199999999999999
        },
        { "hex": "aa2e56",   "type": "adsb_icao",   "flight": "N755TS  ",   "r": "N755TS",   "t": "C25C",   "desc": "CESSNA 525C Citation CJ4",   "alt_baro": 17450, "alt_geom": 17000, 
          "gs": 253.40000000000001, "track": 280.69,  "emergency": "none",   "category": "A2",
         "lat": 43.166167999999999, "lon": -71.186175000000006,  "alert": 0  },
        { "hex": "a9e172",   "type": "adsb_icao",   "flight": "N736BD  ",   "r": "N736BD",   "t": "C172",   "desc": "CESSNA 172 Skyhawk",  "alt_baro": 3300,
               "gs": 134.40000000000001, "track": 162.24000000000001,  "category": "A1",   "lat": 43.404654999999998, "lon": -71.091091000000006,"alert": 0, "messages": 54        },
        { "hex": "3c7a50",   "type": "adsb_icao",   "flight": "CFG2016 ",   "r": "D-ANRP",   "t": "A339",   "desc": "AIRBUS A-330-900",   "alt_baro": 1000,
         "category": "A5",   "lat": 42.852637999999999, "lon": -70.874894999999995, "nic": 8, "rc": 186        },
  ]
}
)";
*/
#define MAX_TRACK_COUNT 400
Track trackArray[MAX_TRACK_COUNT];  // Array of  tracks
int trackCount = 0;                 // Current number of tracks in the array
const int MAX_AIRCRAFT = 20;
Aircraft aircraftList[MAX_AIRCRAFT];
int aircraftCount;

ScreenPoint aircraftLocations[MAX_AIRCRAFT];  // Fixed array for screen points

// Define the line length
#define TRACK_LINE_LENGTH 25
#define TRACK_OFFSET_LENGTH 5  // Offset the start point by 5 pixels

// Convert latitude/longitude to screen coordinates.
// Scale is set when zomming in/out
void lat_lon_to_screen(float thisLat, float thisLon, int &x, int &y) {
  x = scaleX * (thisLon - lonMin);
  y = screenHeight - (scaleY * (thisLat - latMin));  // zero in the top left so we substract from the max
}

// change the zoom level (optional)
// calculate new screen range parameters
void zoom_screen(bool redrawOnly) {
  if (!redrawOnly) {
    if (viewRadius == 50) {
      viewRadius = 20;
    } else if (viewRadius == 20) {
      viewRadius = 5;
    } else {
      viewRadius = 50;
    }
  }
  float screenRatio;                                                  // x vs y
  static const float milesPerDegreeLat = 69;                          // a mile's a minute the world around (nautically)
  static float milesPerDegreeLon = 69 * cos(centerLat * DEG_TO_RAD);  // varies with how far north/south

  screenRatio = (float)screenWidth / screenHeight;

  latRange = (2 * viewRadius) / milesPerDegreeLat;                                     //height is the controlling dimension.
  lonRange = screenRatio * ((2 * viewRadius) / milesPerDegreeLon) * screenPixelRatio;  // screen is wider than tall, so we scale the range, pixels not square
  latMax = centerLat + (latRange / 2);
  latMin = centerLat - (latRange / 2);
  lonMax = centerLon + (lonRange / 2);
  lonMin = centerLon - (lonRange / 2);
  scaleX = screenWidth / lonRange;   // pixels per degree
  scaleY = screenHeight / latRange;  // pixels per degree

  pixelsPerMileY = scaleY / milesPerDegreeLat;  //used to draw the scale
  pixelsPerMileX = scaleX / milesPerDegreeLon;
}

// reg numbers come in padded with spaces
void trim_right(char *str) {
  int len = strlen(str);
  while (len > 0 && str[len - 1] == ' ') {
    str[--len] = '\0';  // Null terminate at the last non-space character
  }
}

// https to get route details from flight number
void get_route_info(Aircraft *thisAircraft) {
  trim_right(thisAircraft->flight);
  char payload[200];  // Ensure the buffer is large enough
  snprintf(payload, sizeof(payload),
           "{\"planes\": [{\"callsign\": \"%s\", \"lat\": %d, \"lng\": %d}]}",
           thisAircraft->flight, centerLat, centerLon);

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;
  https.begin("https://api.adsb.lol/api/0/routeset");
  https.addHeader("Content-Type", "application/json");
  uint16_t httpResponseCode = https.POST((uint8_t *)payload, strlen(payload));

  if (httpResponseCode > 0) {
    Serial.print("Response code: ");
    Serial.println(httpResponseCode);
    // Parse the response (if it's a JSON)
    //    String response = https.getString();
    //    Serial.println("Response: ");
    //    Serial.println(response);
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, https.getString());
    if (error) {
      Serial.print("JSON Parsing failed: ");
      Serial.println(error.c_str());
      return;
    }

    const char *airportCodes = doc[0]["_airport_codes_iata"];
    if (airportCodes) {
      strlcpy(thisAircraft->airportCodes, airportCodes, sizeof(thisAircraft->airportCodes));
    } else {
      serializeJson(doc, Serial);
    }
    // Extract Airport details
    JsonArray airports = doc[0]["_airports"];
    strlcpy(thisAircraft->departureName, airports[0]["name"] | "-", sizeof(thisAircraft->departureName));
    strlcpy(thisAircraft->arrivalName, airports[1]["name"] | "-", sizeof(thisAircraft->arrivalName));

    strlcpy(thisAircraft->departureCode, airports[0]["iata"] | "-", sizeof(thisAircraft->departureCode));
    strlcpy(thisAircraft->arrivalCode, airports[1]["iata"] | "-", sizeof(thisAircraft->arrivalCode));

    strlcpy(thisAircraft->departureCity, airports[0]["location"] | "-", sizeof(thisAircraft->departureCity));
    strlcpy(thisAircraft->arrivalCity, airports[1]["location"] | "-", sizeof(thisAircraft->arrivalCity));

    strlcpy(thisAircraft->departureCountry, airports[0]["countryiso2"] | "-", sizeof(thisAircraft->departureCountry));
    strlcpy(thisAircraft->arrivalCountry, airports[1]["countryiso2"] | "-", sizeof(thisAircraft->arrivalCountry));
  } else {
    Serial.print("Error on sending POST request: ");
    Serial.println(httpResponseCode);
  }
}

void read_ADSB() {
  http.useHTTP10(true);
  http.begin(client, adsbSource);
  int httpResponseCode = http.GET();
  Serial.println("Reading stream data");
  // Parse response
  DeserializationError error = deserializeJson(thisADSB, http.getStream());  // rem for  testing
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  http.end();
}

uint16_t map_alt_to_color(uint16_t alt_baro) {
  // Loop through the array of altitude ranges
  for (uint16_t i = 0; i < sizeof(altitude_color_map) / sizeof(altitude_color_map[0]); i++) {
    if (alt_baro < altitude_color_map[i].max_altitude) {
      return altitude_color_map[i].color;
    }
  }
  return COLOR_DEEP_MAGENTA;  // Default case, if altitude exceeds all ranges
}

//deletes tracks older than 5 minutes
void clear_stale_track_data() {
  unsigned long currentTime = (millis() / 1000);  // just convert to seconds
  // Remove entries older than 5 minutes (300 seconds)
  for (int i = 0; i < trackCount; i++) {
    if (currentTime - trackArray[i].time > 60) {  // 300 seconds = 5 minutes
      // Shift all elements down to remove the old entry
      for (int j = i; j < trackCount - 1; j++) {
        trackArray[j] = trackArray[j + 1];
      }
      trackCount--;  // Decrease the count of tracks
    }
  }
}

// adds new data
void add_track_data(float lat, float lon, uint16_t altColor) {
  if ((lat > latMin) && (lat < latMax) && (lon > lonMin) && (lon < lonMax)) {}
  unsigned long currentTime = (millis() / 1000);  // just convert to seconds
  // Add the new track data to the array if there's space
  if (trackCount < MAX_TRACK_COUNT) {  // Ensure we don't exceed the array size
    trackArray[trackCount].lat = lat;
    trackArray[trackCount].lon = lon;
    trackArray[trackCount].altColor = altColor;
    trackArray[trackCount].time = currentTime;  // Timestamp for the current time
    trackCount++;
  }
}

// Function to draw a line based on the track (angle in degrees)
void draw_track_line(float track, int startX, int startY) {
  // Convert the track value (angle) to radians
  int aviationAngle = round(track);
  float angle = ((450 - aviationAngle) % 360) * DEG_TO_RAD;  // Arduino's math library uses radians

  int offsetX = TRACK_OFFSET_LENGTH * cos(angle);
  int offsetY = TRACK_OFFSET_LENGTH * sin(angle);

  // Adjust the starting point by offsetting it in the direction of the track
  int offsetStartX = startX + offsetX;
  int offsetStartY = startY - offsetY;  // Y is inverted because screen coordinates start from top left

  int endX = offsetStartX + TRACK_LINE_LENGTH * cos(angle);
  int endY = offsetStartY - TRACK_LINE_LENGTH * sin(angle);  // Y is inverted again

  gfx->drawLine(offsetStartX, offsetStartY, endX, endY, WHITE);
}

void draw_scale() {
  char buffer[6];
  float lineLength = 0;
  strlcpy(buffer, "5m", sizeof(buffer));
  lineLength = pixelsPerMileY * 5;
  gfx->drawLine(10, screenHeight - 10, 10 + lineLength, screenHeight - 10, WHITE);  // Use the desired color for the line
  gfx->setCursor(30, screenHeight - 40);
  gfx->println(buffer);
  gfx->drawCircle(screenWidth / 2, screenHeight / 2, 10 * pixelsPerMileY, WHITE);
}

void get_airline_name(struct Aircraft *thisFlight) {
  Serial.println(thisFlight->flight);
  if (thisFlight == NULL || strlen(thisFlight->flight) < 3) {
    strlcpy(thisFlight->airline, "  ", sizeof(thisFlight->airline));
  }
 
  char icaoCode[4];
  strlcpy(icaoCode, thisFlight->flight, 4);
  Serial.println("code");
  Serial.println(icaoCode);
  for (int i = 0; airlines[i].icao[0] != '\0'; i++) {
    if (strcmp(icaoCode, airlines[i].icao) == 0) {
      Serial.println(airlines[i].airlineName);
      strlcpy(thisFlight->airline, airlines[i].airlineName, sizeof(thisFlight->airline));
      return ;
    }
  }
  strlcpy(thisFlight->airline, " ", sizeof(thisFlight->airline));;
}
void parse_ADSB() {
  JsonArray aircraftArray = thisADSB["aircraft"];
  aircraftCount = 0;  // Reset count before parsing
  for (JsonObject ac : aircraftArray) {
    if (aircraftCount >= MAX_AIRCRAFT) break;

    Aircraft &thisAircraft = aircraftList[aircraftCount];
    strlcpy(thisAircraft.hex, ac["hex"] | "xx", sizeof(thisAircraft.hex));
    strlcpy(thisAircraft.reg, ac["r"] | "no reg", sizeof(thisAircraft.reg));
    strlcpy(thisAircraft.flight, ac["flight"] | thisAircraft.reg, sizeof(thisAircraft.flight));
    strlcpy(thisAircraft.type, ac["t"] | "no type", sizeof(thisAircraft.type));
    strlcpy(thisAircraft.desc, ac["desc"] | "", sizeof(thisAircraft.desc));
    strlcpy(thisAircraft.ownOp, ac["ownOp"] | "", sizeof(thisAircraft.ownOp));
    strlcpy(thisAircraft.category, ac["category"] | "", sizeof(thisAircraft.category));
    strlcpy(thisAircraft.emergency, ac["emergency"] | "", sizeof(thisAircraft.emergency));

    thisAircraft.track = (int)ac["track"] | 0;
    thisAircraft.lat = ac["lat"] | 0.0;
    thisAircraft.lon = ac["lon"] | 0.0;
    thisAircraft.alt_baro = max(0, ac["alt_baro"] | 0);  // less than zero ain't good
    thisAircraft.gs = (int)ac["gs"] | 0;
    thisAircraft.alert = ac["alert"] | 0;
    thisAircraft.altColor = map_alt_to_color(thisAircraft.alt_baro);
    add_track_data(thisAircraft.lat, thisAircraft.lon, thisAircraft.altColor);
    aircraftCount++;  // Increment counter
  }                   // for each
}

void plot_aircraft_data() {
  int screen_x, screen_y;
  //  Serial.println("Aircraft Data:");
  for (int i = 0; i < (aircraftCount); i++) {
    if (aircraftList[i].hex == "") continue;
    char buffer[20];
    lat_lon_to_screen(aircraftList[i].lat, aircraftList[i].lon, screen_x, screen_y);
    if ((screen_x > 0) && (screen_y > 0) && (screen_x < screenWidth) && (screen_y < screenHeight)) {
      gfx->fillCircle(screen_x, screen_y, 3, aircraftList[i].altColor);  //  circle to represent aircraft
      gfx->setTextSize(1);
      gfx->setFont(SMALLFONT);
      gfx->setTextColor(WHITE);
      gfx->setCursor(screen_x + 10, screen_y + 10);
      snprintf(buffer, sizeof(buffer), "%s%s", aircraftList[i].flight, aircraftList[i].type);
      gfx->println(buffer);
      gfx->setCursor(screen_x + 10, screen_y + 30);
      gfx->println(aircraftList[i].gs);
      gfx->setCursor(screen_x + 70, screen_y + 30);
      gfx->println((int)(aircraftList[i].alt_baro / 100));
      draw_track_line(aircraftList[i].track, screen_x, screen_y);
      // Store in array to test screen touches - this is not for storing tracks
      aircraftLocations[i].x = screen_x;
      aircraftLocations[i].y = screen_y;
      aircraftLocations[i].info = aircraftList[i];
    }  //  if on screeen
  }    // for each
}

void plot_track_data() {
  Serial.println("Plotting track data");
  int screen_x, screen_y;
  for (int i = 0; i < trackCount; i++) {
    lat_lon_to_screen(trackArray[i].lat, trackArray[i].lon, screen_x, screen_y);
    if ((screen_x > 0) && (screen_y > 0)) {
      gfx->fillCircle(screen_x, screen_y, 3, trackArray[i].altColor);
    }
  }  // for each
}

void show_side_bar(Aircraft thisAircraft) {
  showSideBar = true;
  gfx->fillRect(16, 16, screenWidth / 2, screenHeight * .9, 0x203030);
  gfx->setTextBound(40, 30, (screenWidth / 2) - 20, (screenHeight * .9) - 25);
  get_route_info(&thisAircraft);
get_airline_name(&thisAircraft);
  gfx->setCursor(20, 35);
  char buffer[50];
  gfx->setTextSize(1);
  gfx->setFont(LARGEFONT);
  gfx->setTextColor(WHITE);
    snprintf(buffer,sizeof(buffer),thisAircraft.airline);
    gfx->println(buffer);
  snprintf(buffer, sizeof(buffer), "Reg: %s", thisAircraft.reg);
  gfx->println(buffer);
  snprintf(buffer, sizeof(buffer), "Type: %s", thisAircraft.type);
  gfx->println(buffer);
  snprintf(buffer, sizeof(buffer), "Desc: %s", thisAircraft.desc);
  gfx->println(buffer);
  snprintf(buffer, sizeof(buffer), "Own/Op: %s", thisAircraft.ownOp);
  gfx->println(buffer);
  snprintf(buffer, sizeof(buffer), "Flight: %s", thisAircraft.flight);
  gfx->println(buffer);
  snprintf(buffer, sizeof(buffer), "Route: %s", thisAircraft.airportCodes);
  gfx->println(buffer);
  gfx->setFont(MEDIUMFONT);
  snprintf(buffer, sizeof(buffer), "From: %s, %s", thisAircraft.departureCity, thisAircraft.departureCountry);
  gfx->println(buffer);
  snprintf(buffer, sizeof(buffer), "%s", thisAircraft.departureName);
  gfx->println(buffer);
  snprintf(buffer, sizeof(buffer), "To: %s, %s", thisAircraft.arrivalCity, thisAircraft.arrivalCountry);
  gfx->println(buffer);
  snprintf(buffer, sizeof(buffer), "%s", thisAircraft.arrivalName);
  gfx->println(buffer);
  gfx->setFont(LARGEFONT);
  snprintf(buffer, sizeof(buffer), "Speed: %d", thisAircraft.gs);
  gfx->println(buffer);
  snprintf(buffer, sizeof(buffer), "Alt: %d", thisAircraft.alt_baro);
  gfx->println(buffer);
}

void process_ADSB() {
  Serial.println("starting adsb processing");
  read_ADSB();
  parse_ADSB();
}

void refresh_screen() {
  clear_stale_track_data();
  gfx->fillScreen(0x5aeb);  // dark gray
  draw_map_outline();
  plot_track_data();
  plot_aircraft_data();
  draw_scale();
}
