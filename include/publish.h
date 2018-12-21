#ifndef _PUBLISH_H_
#define _PUBLISH_H_

#include "pms5003.h"

void publish_init();
void publish_data(struct pms5003_data *pms_data);

#endif // _PUBLISH_H_
