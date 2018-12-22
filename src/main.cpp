
#include <Arduino.h>

#include "common/cs_dbg.h"
#include "mgos_app.h"
#include "mgos_config.h"

#include "mgos_timers.h"
#include "mgos_uart.h"
#include "mgos_gpio.h"
#include "mgos_system.h"
#include "mgos_event.h"

#include "PagedDisplay.h"
#include "pms5003.h"
#include "pages.h"
#include "location.h"
#include "aqi_poller.h"
#include "wifi_monitor.h"

#define UART_NO 2

static void handle_button(int pin, void *arg);
static void handle_location_change(int ev, void *ev_data, void *userdata);
static void uart_dispatcher(int uart_no, void *arg);

PagedDisplay *display = nullptr;

enum mgos_app_init_result mgos_app_init(void) {
  display = new PagedDisplay(3, 2);

  pages_init(display);

  display->displayPage(1, 0);

  location_init();
  mgos_event_add_handler(LOCATION_CHANGE_EVENT, handle_location_change, nullptr);

  aqi_poller_init(display);
  wifi_monitor_init(display);

  mgos_gpio_set_button_handler(mgos_sys_config_get_app_display_buttons_a(), MGOS_GPIO_PULL_UP, MGOS_GPIO_INT_EDGE_NEG, 50, handle_button, nullptr);
  mgos_gpio_set_button_handler(mgos_sys_config_get_app_display_buttons_b(), MGOS_GPIO_PULL_UP, MGOS_GPIO_INT_EDGE_NEG, 50, handle_button, nullptr);
  mgos_gpio_set_button_handler(mgos_sys_config_get_app_display_buttons_c(), MGOS_GPIO_PULL_UP, MGOS_GPIO_INT_EDGE_NEG, 50, handle_button, nullptr);

  struct mgos_uart_config ucfg;
  mgos_uart_config_set_defaults(UART_NO, &ucfg);
  ucfg.baud_rate = 9600;
  ucfg.num_data_bits = 8;
  ucfg.parity = MGOS_UART_PARITY_NONE;
  ucfg.stop_bits = MGOS_UART_STOP_BITS_1;
  if (!mgos_uart_configure(UART_NO, &ucfg)) {
    return MGOS_APP_INIT_ERROR;
  }
  // TODO: wait 30 seconds for the fan to spin up before reading?
  mgos_uart_set_dispatcher(UART_NO, uart_dispatcher, nullptr /* arg */);
  mgos_uart_set_rx_enabled(UART_NO, true);

  LOG(LL_INFO, ("UART%d configured", UART_NO));

  return MGOS_APP_INIT_SUCCESS;
}

static void handle_button(int pin, void *arg) {
  LOG(LL_DEBUG, ("pin=%d", pin));

  if (display != nullptr) {

    if (pin == mgos_sys_config_get_app_display_buttons_a()) {
      display->scrollUp(0, false);
    }
    else if (pin == mgos_sys_config_get_app_display_buttons_b()) {
      display->scrollRight(0, true);
    }
    else if (pin == mgos_sys_config_get_app_display_buttons_c()) {
      display->scrollDown(0, false);
    }
    else {
      LOG(LL_WARN, ("unexpected pin: %d", pin));
      return;
    }
  }
  (void) arg;
}

static void handle_location_change(int ev, void *ev_data, void *userdata) {
  const char *loc = "---";

  if (ev_data != nullptr) {
    struct mgos_config_app_loc *locinfo = (struct mgos_config_app_loc *)ev_data;
    loc = locinfo->desc;
    LOG(LL_INFO, ("Location is now '%s'", loc));
  }
  pages_update_location(display, loc);
}

static struct pms5003_data pms_data;

static void uart_dispatcher(int uart_no, void *arg) {
  if (read_pms_data(uart_no, &pms_data)) {
    LOG(LL_DEBUG, ("\n---------------------------------------"));
    LOG(LL_DEBUG, ("Concentration Units (standard)"));
    LOG(LL_DEBUG, ("PM 1.0: %d\t\tPM 2.5: %d\t\tPM 10: %d", pms_data.pm10_standard, pms_data.pm25_standard, pms_data.pm100_standard));
    LOG(LL_DEBUG, ("---------------------------------------"));
    LOG(LL_DEBUG, ("Concentration Units (environmental)"));
    LOG(LL_INFO, ("PM 1.0: %d\t\tPM 2.5: %d\t\tPM 10: %d", pms_data.pm10_env, pms_data.pm25_env, pms_data.pm100_env));
    LOG(LL_DEBUG, ("---------------------------------------"));
    LOG(LL_DEBUG, ("Particles >  0.3um / 0.1L air: %d", pms_data.particles_03um));
    LOG(LL_DEBUG, ("Particles >  0.5um / 0.1L air: %d", pms_data.particles_05um));
    LOG(LL_DEBUG, ("Particles >  1.0um / 0.1L air: %d", pms_data.particles_10um));
    LOG(LL_DEBUG, ("Particles >  2.5um / 0.1L air: %d", pms_data.particles_25um));
    LOG(LL_DEBUG, ("Particles >  5.0um / 0.1L air: %d", pms_data.particles_50um));
    LOG(LL_DEBUG, ("Particles > 10.0um / 0.1L air: %d", pms_data.particles_100um));
    LOG(LL_DEBUG, ("---------------------------------------"));

    pages_update_measured_data(display, pms_data.pm10_env, pms_data.pm25_env, pms_data.pm100_env);
  }
  (void) arg;
}
