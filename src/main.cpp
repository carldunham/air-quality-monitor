
#include <Arduino.h>
#include <Adafruit_SSD1306.h>

#include "common/cs_dbg.h"
#include "mgos_app.h"
#include "mgos_timers.h"
#include "mgos_uart.h"

#include "pms5003.h"

#define UART_NO 2

#define SCREEN_HEIGHT 32
#define SCREEN_WIDTH 128

// We have 32 pixels in height to deal with
#define LABEL_TEXT_SIZE 1
#define LABEL_LINE_HEIGHT 8
#define DATA_TEXT_SIZE 3
#define DATA_LINE_HEIGHT 24

// position the elements on the screen
#define STATUS_BAR_COUNT 4
#define STATUS_BAR_WIDTH 2
#define STATUS_BAR_GUTTER 1
#define STATUS_BAR_TOTAL_WIDTH ((STATUS_BAR_COUNT * STATUS_BAR_WIDTH) + (STATUS_BAR_GUTTER * (STATUS_BAR_COUNT-1)))
#define STATUS_X (SCREEN_WIDTH-STATUS_BAR_TOTAL_WIDTH)
#define STATUS_Y 0

#define LABEL_X 0
#define LABEL_Y ((DATA_LINE_HEIGHT - LABEL_LINE_HEIGHT) / 2)

#define DATA_X (SCREEN_WIDTH/3)
#define DATA_Y 0

Adafruit_SSD1306 *display = nullptr;

static void uart_dispatcher(int uart_no, void *arg);
static void print_data(const char *label, int data);

enum mgos_app_init_result mgos_app_init(void) {
  display = new Adafruit_SSD1306(-1 /* RST GPIO */, Adafruit_SSD1306::RES_128_32);

  if (display != nullptr) {
    LOG(LL_INFO, ("Adafruit_SSD1306 created"));
    display->begin(SSD1306_SWITCHCAPVCC, 0x3C, false);
    // display->display();
  }
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
  mgos_uart_set_dispatcher(UART_NO, uart_dispatcher, NULL /* arg */);
  mgos_uart_set_rx_enabled(UART_NO, true);

  LOG(LL_INFO, ("UART%d configured", UART_NO));

  return MGOS_APP_INIT_SUCCESS;
}

static struct pms5003_data pms_data;
static int which = 1;

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

    if (display != nullptr) {

      switch (which) {
        case 0:
          print_data("PM1.0", pms_data.pm10_env);
          break;
        case 1:
          print_data("PM2.5", pms_data.pm25_env);
          break;
        case 2:
          print_data("PM10", pms_data.pm100_env);
          break;
        default:
          LOG(LL_WARN, ("Unknown selector for data value: %d", which));
          print_data("UNK", -1);
          break;
      }
      display->display();
    }
  }
  (void) arg;
}

static void print_data(const char *label, int data) {
  LOG(LL_DEBUG, ("Displaying %s: %d", label, data));

  display->clearDisplay();
  display->setTextColor(WHITE);
  display->setTextWrap(false);

  display->setTextSize(LABEL_TEXT_SIZE);
  display->setCursor(LABEL_X, LABEL_Y);

  display->print(label);

  display->setTextSize(DATA_TEXT_SIZE);
  display->setCursor(DATA_X, DATA_Y);

  if (data < 0) {
    display->print("----");
  }
  else if (data > 999) {
    display->print(">999");
  }
  else {
    display->printf(" %d", data);
  }

  // Wifi strength meter
  display->fillRect(STATUS_X,                                        STATUS_Y+3, STATUS_BAR_WIDTH, 1, WHITE);
  display->fillRect(STATUS_X+(STATUS_BAR_WIDTH+STATUS_BAR_GUTTER)  , STATUS_Y+2, STATUS_BAR_WIDTH, 2, WHITE);
  display->fillRect(STATUS_X+(STATUS_BAR_WIDTH+STATUS_BAR_GUTTER)*2, STATUS_Y+1, STATUS_BAR_WIDTH, 3, WHITE);
  display->fillRect(STATUS_X+(STATUS_BAR_WIDTH+STATUS_BAR_GUTTER)*3, STATUS_Y+0, STATUS_BAR_WIDTH, 4, WHITE);

  // test pattern
  // for (int16_t y=0, w=1; y < 32; y += w, w <<= 1) {
  //   LOG(LL_INFO, ("y=%d, w=%d", y, w));
  //
  //   for (int16_t x=0; x < 128; x += w<<1) {
  //     display->fillRect(x, y, w, w, WHITE);
  //   }
  // }
}
