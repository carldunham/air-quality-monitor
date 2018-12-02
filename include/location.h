#ifndef _LOCATION_H_
#define _LOCATION_H_

#include "mgos_event.h"

#define LOCATION_EVENT_BASE MGOS_EVENT_BASE('L', 'O', 'C')

enum location_event {
  LOCATION_CHANGE_EVENT = LOCATION_EVENT_BASE,
};

void location_init();

#endif // _LOCATION_H_
