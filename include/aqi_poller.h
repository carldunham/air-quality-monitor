#ifndef _AQI_POLLER_H_
#define _AQI_POLLER_H_

#include "mgos_event.h"

#include "PagedDisplay.h"

#define AQI_POLLER_EVENT_BASE MGOS_EVENT_BASE('A', 'Q', 'I')

enum aqi_poller_event {
  AQI_POLLER_DATA_EVENT = AQI_POLLER_EVENT_BASE,
};

void aqi_poller_init(PagedDisplay *display);

#endif // _AQI_POLLER_H_
