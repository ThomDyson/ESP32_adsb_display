#include <Arduino.h>
extern const int numMapSets;



// plotted data, as lat and lon, with optional fill color
struct Mapsetbase {
  float (*coordinates)[2]; // Pointer to coordinate array
  uint16_t numPoints;   // numPoints is 1 based human friendly
  int fillColor;   // -1 for no fill
};

extern Mapsetbase myMapSets[];
