
#include "common/cs_dbg.h"

#include "mgos_timers.h"
#include "mgos_wifi.h"

#include "wifi_monitor.h"
#include "PagedDisplay.h"
#include "pages.h"


const int CHECK_INTERVAL_MS = 2000;

static void wifi_check_timer_cb(void *arg);
static int rssi_to_level(int rssi_neg);


bool wifi_monitor_init(PagedDisplay *display) {
  if (mgos_set_timer(CHECK_INTERVAL_MS, MGOS_TIMER_REPEAT|MGOS_TIMER_RUN_NOW, wifi_check_timer_cb, display) == MGOS_INVALID_TIMER_ID) {
    LOG(LL_WARN, ("Unable to set timer to check wifi strength"));
    return false;
  }
  return true;
}

static void wifi_check_timer_cb(void *arg) {
  if (arg != nullptr) {
    PagedDisplay *display = (PagedDisplay *)arg;
    pages_update_wifi_strength(display, rssi_to_level(mgos_wifi_sta_get_rssi()));
  }
}

static int rssi_to_level(int rssi_neg) {
  LOG(LL_INFO, ("rssi=%d", rssi_neg));

  // 100% arbitrary, so cribbed from https://www.speedguide.net/faq/how-does-rssi-dbm-relate-to-signal-quality-percent-439

  if (rssi_neg == 0 || rssi_neg < -96) {
    return 0;
  }
  else if (rssi_neg < -85) {
    return 1;
  }
  else if (rssi_neg < -75) {
    return 2;
  }
  else if (rssi_neg < -55) {
    return 3;
  }
  else {
    return 4;
  }
}
