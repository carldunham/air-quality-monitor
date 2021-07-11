
#include "common/cs_dbg.h"
#include "mgos_event.h"
#include "mgos_net.h"
#include "mgos_wifi.h"

#include "location.h"

static void location_net_ev_handler(int ev, void *evd, void *arg);

void location_init() {
  mgos_event_register_base(LOCATION_EVENT_BASE, "locations");
  mgos_event_add_group_handler(MGOS_NET_EV_IP_ACQUIRED, location_net_ev_handler, nullptr);
}

static void location_net_ev_handler(int ev, void *evd, void *arg) {
  if (ev == MGOS_NET_EV_IP_ACQUIRED) {
    LOG(LL_INFO, ("IP Acquired"));

    char *ssid = mgos_wifi_get_connected_ssid();
    void *ev_data = nullptr;

    if (ssid != nullptr) {
      LOG(LL_INFO, ("Connected to SSID %s", ssid));

      if (mgos_sys_config_get_app_loc_ssid() != nullptr && strcmp(ssid, mgos_sys_config_get_app_loc_ssid()) == 0) {
        ev_data = (void *)mgos_sys_config_get_app_loc();
      }
      else if (mgos_sys_config_get_app_loc1_ssid() != nullptr && strcmp(ssid, mgos_sys_config_get_app_loc1_ssid()) == 0) {
        ev_data = (void *)mgos_sys_config_get_app_loc1();
      }
      else if (mgos_sys_config_get_app_loc2_ssid() != nullptr && strcmp(ssid, mgos_sys_config_get_app_loc2_ssid()) == 0) {
        ev_data = (void *)mgos_sys_config_get_app_loc2();
      }
      else {
        LOG(LL_WARN, ("Unable to find location for SSID %s", ssid));
      }
    }
    mgos_event_trigger(LOCATION_CHANGE_EVENT, ev_data);  // null ev_data means we don't know the location
  }
  else if (ev == MGOS_WIFI_EV_STA_DISCONNECTED) {
    mgos_event_trigger(LOCATION_CHANGE_EVENT, nullptr);
  }
  (void) evd;
  (void) arg;
}
