#include "maps.h"
#include "mapdata.h"

#define MAX_MAP_POINTS 100
int polygonScreenPoints[MAX_MAP_POINTS][2];

// Fill polygon using scanline method
void fill_polygon_latlon(float latLon[][2], int numPoints, uint16_t fillColor) {
  float extendedLatLon[3][2];  // If only 2 points, add a duplicate of the first to close the shape
  if (numPoints == 2) {
    memcpy(extendedLatLon, latLon, 2 * sizeof(latLon[0]));  // Copy first 2 points
    extendedLatLon[2][0] = latLon[0][0];                    // Duplicate first point
    extendedLatLon[2][1] = latLon[0][1];
    latLon = extendedLatLon;  // Reassign to local array
    numPoints = 3;
  }
  //int screenX[numPoints], screenY[numPoints];

  // Convert all lat/lon points to screen coordinates
  for (int i = 0; i < numPoints; i++) {
    lat_lon_to_screen(latLon[i][0], latLon[i][1], polygonScreenPoints[i][0], polygonScreenPoints[i][1]);
  }

  fill_polygon_sl(polygonScreenPoints, numPoints, fillColor);
  /*
  // Decompose into triangles
  for (int i = 1; i < numPoints - 1; i++) {
    gfx->fillTriangle(screenX[0], screenY[0],
                      screenX[i], screenY[i],
                      screenX[i + 1], screenY[i + 1], color);
  }
*/
}

void draw_map_outline() {
  int x1, y1, x2, y2;
  for (int n = 0; n < numMapSets; n++) {
    if (myMapSets[n].fillColor == -1) {
      // Access each mapset's coordinates dynamically.
      // numPoints is 1 based human friendly, but we
      // don't draw a line off the last coord
      for (int j = 0; j < (myMapSets[n].numPoints - 1); j++) {
        x1 = 0;
        y1 = 0;
        x2 = 0;
        y2 = 0;
        if (myMapSets[n].coordinates[j][0] > 0) {
          lat_lon_to_screen(myMapSets[n].coordinates[j][0], myMapSets[n].coordinates[j][1], x1, y1);
          lat_lon_to_screen(myMapSets[n].coordinates[j + 1][0], myMapSets[n].coordinates[j + 1][1], x2, y2);
          gfx->drawLine(x1, y1, x2, y2, WHITE);  // Draw outline in blue
        }
      }
    } else {
      Serial.println("Fill this");
      fill_polygon_latlon(myMapSets[n].coordinates, myMapSets[n].numPoints, myMapSets[n].fillColor);
    }
  }
}

// Function to fill a polygon using a scanline algorithmvoid fillPolygon(int polygon[][2], int numVertices, uint16_t color) {
void fill_polygon_sl2(int polygon[][2], int numVertices, uint16_t color) {
  // Find the min and max Y values (bounding box)
  int minY = polygon[0][1], maxY = polygon[0][1];
  for (int i = 0; i < numVertices; i++) {
    int y = polygon[i][1];
    if (y < minY) minY = y;
    if (y > maxY) maxY = y;
  }

  // Loop over all scanlines from minY to maxY
  for (int y = minY; y <= maxY; y++) {
    // Declare a 2D array for intersections (with each row being an (x, y) pair)
    int intersections[10][2];  // Max 10 intersections per scanline, each with x, y pair
    int intersectionCount = 0;

    for (int i = 0; i < numVertices; i++) {
      int x1 = polygon[i][0];
      int y1 = polygon[i][1];
      int x2 = polygon[(i + 1) % numVertices][0];
      int y2 = polygon[(i + 1) % numVertices][1];

      // Check if the scanline intersects with the edge
      if (((y1 <= y && y2 > y) || (y1 > y && y2 <= y)) && y1 != y2) {
        int xIntersection = x1 + (y - y1) * (x2 - x1) / (y2 - y1);
        // Add the intersection (x, y) pair to the array if there's space
        if (intersectionCount < 10) {
          intersections[intersectionCount][0] = xIntersection;  // x value
          intersections[intersectionCount][1] = y;              // y value (same as scanline y)
          intersectionCount++;
        }
      }
    }

    // Sort the intersection points by x (bubble sort for simplicity)
    for (int i = 0; i < intersectionCount - 1; i++) {
      for (int j = i + 1; j < intersectionCount; j++) {
        if (intersections[i][0] > intersections[j][0]) {
          // Swap the points (x, y)
          int tempX = intersections[i][0];
          int tempY = intersections[i][1];
          intersections[i][0] = intersections[j][0];
          intersections[i][1] = intersections[j][1];
          intersections[j][0] = tempX;
          intersections[j][1] = tempY;
        }
      }
    }

    // Fill the space between each pair of intersections
    for (int i = 0; i < intersectionCount; i += 2) {
      int xStart = intersections[i][0];
      int xEnd = intersections[i + 1][0];
      gfx->drawFastHLine(xStart, y, (xEnd - xStart), color);
    }
  }
}


// Function to fill a polygon using a scanline algorithmvoid fillPolygon(int polygon[][2], int numVertices, uint16_t color) {
void fill_polygon_sl(int polygon[][2], int numVertices, uint16_t color) {
  // Find the min and max Y values (bounding box)
  int minY = polygon[0][1], maxY = polygon[0][1];
  for (int i = 0; i < numVertices; i++) {
    int y = polygon[i][1];
    if (y < minY) minY = y;
    if (y > maxY) maxY = y;
  }

  // Loop over all scanlines from minY to maxY
  for (int y = minY; y <= maxY; y++) {
    // Declare a 2D array for intersections (with each row being an (x, y) pair)
    int intersections[10];  // Max 10 intersections per scanline, each with x, y pair
    int intersectionCount = 0;

    for (int i = 0; i < numVertices; i++) {
      int x1 = polygon[i][0];
      int y1 = polygon[i][1];
      int x2 = polygon[(i + 1) % numVertices][0];
      int y2 = polygon[(i + 1) % numVertices][1];

      // Check if the scanline intersects with the edge
      if (((y1 <= y && y2 > y) || (y1 > y && y2 <= y)) && y1 != y2) {
        int xIntersection = x1 + (y - y1) * (x2 - x1) / (y2 - y1);
        // Add the intersection (x, y) pair to the array if there's space
        if (intersectionCount < 10) {
          intersections[intersectionCount] = xIntersection;  // x value
          intersectionCount++;
        }
      }
    }

    // Sort the intersection points by x (select sort for simplicity)
    for (int i = 0; i < intersectionCount - 1; i++) {
      for (int j = i + 1; j < intersectionCount; j++) {
        if (intersections[i] > intersections[j]) {
          // Swap the points (x, y)
          int tempX = intersections[i];
          intersections[i] = intersections[j];
          intersections[j] = tempX;
        }
      }
    }

    // Fill the space between each pair of intersections
    for (int i = 0; i < intersectionCount; i += 2) {
      int xStart = intersections[i];
      int xEnd = intersections[i + 1];
      gfx->drawFastHLine(xStart, y, (xEnd - xStart), color);
    }
  }
}
