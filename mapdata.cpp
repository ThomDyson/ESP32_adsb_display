#include <Arduino_GFX_Library.h>  // need this for color definitions

/*******************************************************************************
**
**  ADD a set of data for each object. lines are plotted from the first set to each set in succession to the last.
**  THEN append the array info to  myMapSets[] at the end of this file.
**  https://www.keene.edu/campus/maps/tool/
********************************************************************************/
#include "mapdata.h"

float ksfmRunway7[][2] = {
  { 43.386667, -70.719733 },  // Runway 7 Threshold
  { 43.395833, -70.697833 },  // Runway 25 Threshold
};


float ksfmRunway14[][2] = {
  { 43.400333, -70.714300 },  // Runway 14 Threshold
  { 43.393717, -70.697833 }   // Runway 32 Threshold
};

float coastCoordinates[][2] = {
     { 42.3805, -71.0320 },  // East Boston, MA
  { 42.4002, -71.0000 },  // Revere, MA
  { 42.4185, -70.9725 },  // Lynn, MA
  { 42.4371, -70.9500 },  // Swampscott, MA
  { 42.4668, -70.9495 },  // Salem, MA
  { 42.4872, -70.9300 },  // Marblehead, MA
  { 42.5088, -70.9000 },  // Beverly, MA
  { 42.5278, -70.8800 },  // Manchester-by-the-Sea, MA
  { 42.5533, -70.8200 },  // Gloucester (West), MA
  { 42.5780, -70.7800 },  // Gloucester (East), MA
  { 42.6000, -70.7100 },  // Rockport, MA
  { 42.6250, -70.6900 },  // Halibut Point, MA
  { 42.6592, -70.6222 },  // Cape Ann, MA*
  { 42.7214, -70.7781 },  // Ipswich Bay, MA*
  { 42.7400, -70.6500 },  // Plum Island (South), MA
  { 42.7700, -70.6800 },  // Plum Island (North), MA
  { 42.7637, -70.7900 },  // Newburyport, MA *
  { 42.8300, -70.9000 },  // Salisbury Beach, MA
  { 42.8888, -70.8146 },  // Seabrook, NH*
  { 42.9500, -70.9100 },  // Hampton Beach, NH
  { 42.9900, -70.9300 },  // North Hampton, NH
  { 43.0300, -70.9400 },  // Rye, NH
  { 43.0707, -70.7626 },  // Portsmouth, NH
  { 43.0841, -70.7443 },  // Kittery, ME
  { 43.1548, -70.6256 },  // York, ME
  { 43.2430, -70.6037 },  // Ogunquit, ME
  { 43.3637, -70.4756 },  // Kennebunkport, ME
  { 43.5057, -70.4418 },  // Biddeford, ME
  { 43.5372, -70.3755 },  // Saco, ME
  { 43.5139996, -70.3736115 },
  { 43.5388924, -70.3324128 },
  { 43.5364036, -70.3097534 },
  { 43.5622823, -70.2699280 },
  { 43.5518327, -70.2445221 },
  { 43.5622823, -70.2225495 },
  { 43.5568089, -70.2060700 },
  { 43.5662627, -70.1950836 },
  { 43.5926259, -70.2156830 },
  { 43.6239483, -70.2101899 },
  { 43.6656861, -70.2390290 },
  { 43.6934952, -70.2376557 },
  { 43.8130266, -70.1010132 },
  { 43.8665150, -69.9918366 },
  { 43.8288800, -70.0206757 },
  { 43.8576036, -69.9506378 },
  { 43.7436183, -70.0371552 },
  { 43.7460986, -70.0096894 },
  { 43.7391536, -69.9897766 },
  { 43.7738704, -69.9396515 },
  { 43.7748620, -69.8970795 },
  { 43.7793241, -69.8709870 },
  { 43.7371692, -69.8661805 },
  { 43.7064022, -69.8558808 },
  { 43.7108694, -69.8311615 },
  { 43.7267499, -69.8345948 },
  { 43.7803156, -69.7212982 },
  { 43.8516619, -69.6993256 },
  { 43.8556231, -69.6746064 },
  { 43.8343286, -69.6691132 },
  { 43.8239263, -69.6876526 },
  { 43.7837858, -69.6553803 },
  { 43.8303660, -69.6443940 },
  { 43.8329417, -69.6398621 },
  { 43.8141166, -69.5917969 },
  { 43.8775039, -69.5451050 },
  { 43.8750291, -69.5265656 },
  { 43.8349230, -69.5142060 },
  { 43.8364089, -69.5039063 },
  { 43.9764117, -69.4270020 },
  { 43.9180760, -69.2779999 },
  { 43.9566433, -69.1928559 },
  { 44.0726884, -69.0514069 },
  { 44.0924182, -69.0383606 },
  { 44, -68.5},
  { 42, -68.5}
};

float PWMrunway11_29[][2] = {
  { 43.645861, -70.326075 },  // Runway 11 Threshold
  { 43.644033, -70.298992 }   // Runway 29 Threshold
};

// Runway 18/36
float PWMrunway18_36[][2] = {
  { 43.654472, -70.307319 },  // Runway 18 Threshold
  { 43.638450, -70.300664 }   // Runway 36 Threshold
};

float PSMrunway16_34[][2] = {
 {43.091278, -70.834164},
  { 43.064630, -70.812389 }
};

float DAWrunway15_33[][2] = {
  {43.287653, -70.934933},
   {43.280453, -70.923596}
};



float sebagoLakeOutline[][2] = {
  { 43.8986, -70.6230 },  // Northwest point near Sebago, ME
  { 43.9117, -70.6438 },  // Northern shore near Northwest River
  { 43.9242, -70.6665 },  // Western cove area
  { 43.9300, -70.6843 },  // Jordan Bay (west side)
  { 43.9228, -70.7105 },  // Northern Jordan Bay
  { 43.9072, -70.7217 },  // Frye Island (north side)
  { 43.8906, -70.7314 },  // Frye Island (west side)
  { 43.8725, -70.7245 },  // Near Sebago Lake State Park
  { 43.8500, -70.7100 },  // Southern lake area
  { 43.8325, -70.6950 },  // South-central shoreline
  { 43.8250, -70.6700 },  // East side near Raymond Neck
  { 43.8352, -70.6453 },  // Northern Raymond Neck
  { 43.8542, -70.6308 },  // Eastern shore near Indian Island
  { 43.8708, -70.6175 },  // Near White’s Bridge
  { 43.8903, -70.6120 },  // Northern lake inlet
  { 43.9105, -70.6105 },  // Western shore near Sebago
  { 43.9250, -70.6200 },  // Moving back toward the northwest
  { 43.9333, -70.6350 },  // Close to original point
  { 43.9300, -70.6500 },  // Final smoothing point near the start
  { 43.8986, -70.6230 }   // Closing the loop
};
float lakeWinnipesaukeeShoreline[][2] = {
  { 43.530, -71.253 },
  { 43.565, -71.188 },
  { 43.617, -71.290 },
  { 43.653, -71.277 },
  { 43.687, -71.315 },
  { 43.673, -71.382 },
  { 43.692, -71.418 },
  { 43.684, -71.452 },
  { 43.633, -71.412 },
  { 43.629, -71.465 },
  { 43.540, -71.264 },
  { 43.530, -71.253 }
};

// defined colors include BLACK, WHITE, NAVY, DARKGREEN, OLIVE, GREEN, BLIE, YELLOW, WHITE, ORANGE
// See arduino_GFX.h for more - or define your own.  Colors are RGB565 hex L 0xffe0 is yellow
// Mapset data initialization
Mapsetbase myMapSets[] = {
  { ksfmRunway7, sizeof(ksfmRunway7) / sizeof(ksfmRunway7[0]), YELLOW },
  { ksfmRunway14, sizeof(ksfmRunway14) / sizeof(ksfmRunway14[0]), YELLOW },
  { coastCoordinates, sizeof(coastCoordinates) / sizeof(coastCoordinates[0]), 0x126e },
  { sebagoLakeOutline, sizeof(sebagoLakeOutline) / sizeof(sebagoLakeOutline[0]), BLUE },
  { PWMrunway18_36, sizeof(PWMrunway18_36) / sizeof(PWMrunway18_36[0]), -1 },
  { PWMrunway11_29, sizeof(PWMrunway11_29) / sizeof(PWMrunway11_29[0]), -1 },
  { lakeWinnipesaukeeShoreline, sizeof(lakeWinnipesaukeeShoreline) / sizeof(lakeWinnipesaukeeShoreline[0]), BLUE },
  { PSMrunway16_34, sizeof(PSMrunway16_34) / sizeof(PSMrunway16_34[0]), YELLOW },  
 { DAWrunway15_33, sizeof(DAWrunway15_33) / sizeof(DAWrunway15_33[0]), YELLOW }  

};

const int numMapSets = sizeof(myMapSets) / sizeof(myMapSets[0]);
