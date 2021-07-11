#ifndef _PAGES_H_
#define _PAGES_H_

#include "PagedDisplay.h"

void pages_init(PagedDisplay *display);
void pages_update_measured_data(PagedDisplay *display, int32_t pm10, int32_t pm25, int32_t pm100);
void pages_update_reported_data(PagedDisplay *display, int32_t pm25, int32_t o3, int qualityLevel);
void pages_update_wifi_strength(PagedDisplay *display, int level);
void pages_update_location(PagedDisplay *display, const char *location);
void pages_update_reported_locations(PagedDisplay *display, const char *o3_location, const char *pm25_location, const char *quality_location);

#endif // _PAGES_H_
