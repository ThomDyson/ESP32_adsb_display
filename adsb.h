#pragma once
#include <Arduino_GFX_Library.h>
#include "maps.h"


#define COLOR_GOLDEN_AMBER  0xebc3
#define COLOR_MID_ORANGE  0xeca3
#define COLOR_MUSTARD_YELLOW   0xdde2
#define COLOR_MID_YELLOW   0xbe22  
#define COLOR_BRIGHT_GREEN   0x5622
#define COLOR_LEAF_GREEN    0x2621
#define COLOR_TEAL_BLUE     0x1597
#define COLOR_MID_BLUE  0x41dd
#define COLOR_DEEP_MAGENTA  0xc099

typedef struct {
    uint16_t max_altitude;
    uint16_t color;
} AltitudeColorRange;

const AltitudeColorRange altitude_color_map[] = {
    {400, RED},          
    {1000, COLOR_GOLDEN_AMBER},      
    {2000, COLOR_MID_ORANGE},       
    {4000, COLOR_MUSTARD_YELLOW},  
    {6000, COLOR_MID_YELLOW},
    {8000, COLOR_BRIGHT_GREEN},
    {10000, COLOR_LEAF_GREEN},       
    {20000, COLOR_TEAL_BLUE},        
    {30000, COLOR_MID_BLUE},       
    {UINT16_MAX, COLOR_DEEP_MAGENTA}     
};


extern Arduino_RGB_Display *gfx;


extern uint16_t screenWidth;
extern uint16_t screenHeight;
extern float screenPixelRatio;
extern bool showSideBar;

extern float centerLat;
extern float centerLon;

extern float defaultLat;
extern float defaultLon;

extern float lonRange;
extern const char *adsbSource;

struct Aircraft {
  char hex[7];
  char flight[10];
  char airline[30];  // update by get_airline_name for sidebar
  char reg[8];
  char type[7];
  char desc[60];
  char category[3];
  char ownOp[30];
  int track;
  int alt_baro;
  int gs;
  float lat;
  float lon;
  int alert;
  char route[15];
  char emergency[8];
  uint16_t altColor;
  char airportCodes[20];  //updated by get_route_info
  char departureCode[5];
  char departureName[60];
  char departureCity[20];
  char departureCountry[3];
  char arrivalCode[5];
  char arrivalName[60];
  char arrivalCity[20];
  char arrivalCountry[3];
};

extern Aircraft aircraftList[];

// used to track locations for touch screen
struct ScreenPoint {
  int x;
  int y;
  Aircraft info;
};
extern ScreenPoint aircraftLocations[];
extern int aircraftCount;

typedef struct {
    char airlineName[50];  // Airline name
    char icao[4];   // ICAO code
} Airline;

const Airline airlines[] = {
    // Top  US Airlines and transatlantic and cargo, etc
    {"American Airlines", "AAL"},
    {"Delta Air Lines", "DAL"},
    {"United Airlines", "UAL"},
    {"Southwest Airlines", "SWA"},
    {"Alaska Airlines", "ASA"},
    {"JetBlue Airways", "JBU"},
    {"SkyWest Airlines", "SKW"},
    {"Republic Airways", "RPA"},
    {"Envoy Air", "ENY"},
    {"Piedmont Airlines", "PDT"},
    {"GoJet Airlines", "GJS"},
    {"Mesa Air Group", "MES"},
    {"Hawaiian Airlines", "HAL"},
    {"Sun Country Airlines", "SCX"},
    {"Allegiant Air", "AAY"},
    {"Spirit Airlines", "NKS"},
    {"Republic Airways", "RPA"},
    {"Compass Airlines", "CPZ"},
    {"Silver Airways", "SIL"},
    {"Piedmont Airlines", "PDT"},
    {"British Airways", "BAW"},
    {"Lufthansa", "DLH"},
    {"Air France", "AFR"},
    {"KLM Royal Dutch Airlines", "KLM"},
    {"Iberia", "IBE"},
    {"Aer Lingus", "EIN"},
    {"Norwegian Air Shuttle", "NAX"},
    {"Finnair", "FIN"},
    {"Swiss International Air Lines", "SWR"},
    {"Air Canada", "ACA"},
    {"Alitalia", "AZA"},
    {"Virgin Atlantic", "VIR"},
    {"SAS (Scandinavian Airlines)", "SAS"},
    {"TAP Air Portugal", "TAP"},
    {"Turkish Airlines", "THY"},
    {"Emirates", "UAE"},
    {"FedEx", "FDX"},
    {"UPS", "UPS"},
    {"DHL", "DHL"},
    {"Jazz Aviation", "JZA"}, 
    {"Air Transat", "TSC"},
    {"Cape Air", "KAP"}
};

const int numAirlines = sizeof(airlines) / sizeof(airlines[0]);

void parse_ADSB();
void process_ADSB();
void refresh_screen();
void zoom_screen(bool redrawOnly);
void lat_lon_to_screen(float lat, float lon, int &x, int &y);
void show_side_bar(Aircraft thisAircraft);
void get_route_info(Aircraft *thisAircraft);
