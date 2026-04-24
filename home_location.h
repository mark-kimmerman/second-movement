#pragma once

// Home coordinates used as a fallback when no location is stored in the
// filesystem. Lat/lon are signed int16_t in hundredths of a degree, matching
// the movement_location_t format in movement.h.
#define MOVEMENT_DEFAULT_LATITUDE  4240   // 42.40°
#define MOVEMENT_DEFAULT_LONGITUDE -7114  // -71.14°

