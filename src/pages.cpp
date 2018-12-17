
// #include <Arduino.h>

#include "common/cs_dbg.h"

#include "pages.h"

// We have 32 pixels in height to deal with
#define LABEL_TEXT_SIZE 1
#define LABEL_LINE_HEIGHT 8
#define DATA_TEXT_SIZE 3
#define DATA_LINE_HEIGHT 24
#define QUALITY_TEXT_SIZE_LARGE 2
#define QUALITY_LINE_HEIGHT_LARGE 16
#define QUALITY_TEXT_SIZE_SMALL 1
#define QUALITY_LINE_HEIGHT_SMALL 8
#define LOCATION_TEXT_SIZE 1
#define LOCATION_LINE_HEIGHT 8

// position the elements on the screen
#define STATUS_BAR_COUNT 4
#define STATUS_BAR_WIDTH 2
#define STATUS_BAR_GUTTER 1
#define STATUS_BAR_TOTAL_WIDTH ((STATUS_BAR_COUNT * STATUS_BAR_WIDTH) + (STATUS_BAR_GUTTER * (STATUS_BAR_COUNT-1)))
#define STATUS_X (PagedDisplay::PAGE_WIDTH-STATUS_BAR_TOTAL_WIDTH)
#define STATUS_Y 0

#define LABEL_X 0
#define LABEL_Y ((DATA_LINE_HEIGHT - LABEL_LINE_HEIGHT) / 2)

#define DATA_X (PagedDisplay::PAGE_WIDTH/3)
#define DATA_Y 0

#define QUALITY_X 0
#define QUALITY_Y_LARGE ((DATA_LINE_HEIGHT - QUALITY_LINE_HEIGHT_LARGE) / 2)
#define QUALITY_Y_SMALL ((DATA_LINE_HEIGHT - QUALITY_LINE_HEIGHT_SMALL) / 2)

#define LOCATION_X 0
#define LOCATION_Y (PagedDisplay::PAGE_HEIGHT - LOCATION_LINE_HEIGHT)

// Areas to clear
#define DATA_AREA_X 0
#define DATA_AREA_Y 0
#define DATA_AREA_WIDTH PagedDisplay::PAGE_WIDTH
#define DATA_AREA_HEIGHT (PagedDisplay::PAGE_HEIGHT - LOCATION_LINE_HEIGHT)

#define LOCATION_AREA_X LOCATION_X
#define LOCATION_AREA_Y LOCATION_Y
#define LOCATION_AREA_WIDTH PagedDisplay::PAGE_WIDTH
#define LOCATION_AREA_HEIGHT LOCATION_LINE_HEIGHT


static void print_pms_data(GFXCanvas1 *page, const char *label, int data);
static void print_pms_text(GFXCanvas1 *page, const char *text, bool small);


void pages_init(PagedDisplay *display) {
  pages_update_wifi_strength(display, 0);
  pages_update_measured_data(display, -1, -1, -1);
  pages_update_reported_data(display, -1, -1, -1);

  for (int row=0; row<3; row++) {
    GFXCanvas1 *page = display->getPage(row, 0);

    page->setTextSize(LOCATION_TEXT_SIZE);
    page->setCursor(LOCATION_X, LOCATION_Y);

    page->print("measured");
  }
  pages_update_location(display, "---");
}

void pages_update_measured_data(PagedDisplay *display, int32_t pm10, int32_t pm25, int32_t pm100) {
  print_pms_data(display->getPage(0, 0), "PM1.0", pm10);
  print_pms_data(display->getPage(1, 0), "PM2.5", pm25);
  print_pms_data(display->getPage(2, 0), "PM10", pm100);

  display->display();
}

void pages_update_reported_data(PagedDisplay *display, int32_t pm25, int32_t o3, int qualityLevel) {
  const char *qualityName = "---";
  bool qualitySmall = false;

  // large can fit 10 chars: "1234567890"
  // small can fit 21: "123456789012345678901"

  switch (qualityLevel) {
    case 1:
      qualityName = "Good";
      break;
    case 2:
      qualityName = "Moderate";
      break;
    case 3:
      qualityName = "Unhealthy for Some";
      qualitySmall = true;
      break;
    case 4:
      qualityName = "Unhealthy";
      break;
    case 5:
      qualityName = "Very Unhealthy";
      qualitySmall = true;
      break;
    case 6:
      qualityName = "Hazardous";
      break;
  }
  print_pms_data(display->getPage(0, 1), "O3", o3);
  print_pms_data(display->getPage(1, 1), "PM2.5", pm25);
  print_pms_text(display->getPage(2, 1), qualityName, qualitySmall);

  display->display();
}

void pages_update_wifi_strength(PagedDisplay *display, int level) {
  GFXCanvas1 *overlay = display->getOverlay();

  // Wifi strength meter
  overlay->fillScreen(BLACK);

  if (level <= 0) {
    LOG(LL_INFO, ("no wifi signal or connection"));
    return;
  }
  LOG(LL_INFO, ("WIFI level=%d", level));

  switch (level) {
    default:
      overlay->fillRect(STATUS_X+(STATUS_BAR_WIDTH+STATUS_BAR_GUTTER)*3, STATUS_Y+0, STATUS_BAR_WIDTH, 4, WHITE);
      // fallthrough
    case 3:
      overlay->fillRect(STATUS_X+(STATUS_BAR_WIDTH+STATUS_BAR_GUTTER)*2, STATUS_Y+1, STATUS_BAR_WIDTH, 3, WHITE);
      // fallthrough
    case 2:
      overlay->fillRect(STATUS_X+(STATUS_BAR_WIDTH+STATUS_BAR_GUTTER)  , STATUS_Y+2, STATUS_BAR_WIDTH, 2, WHITE);
      // fallthrough
    case 1:
      overlay->fillRect(STATUS_X,                                        STATUS_Y+3, STATUS_BAR_WIDTH, 1, WHITE);
      // fallthrough
  }

  display->display();
}

void pages_update_location(PagedDisplay *display, const char *location) {

  for (int i=0; i<3; i++) {
    update_page_location(display->getPage(i, 1), location);
  }
  display->display();
}

void pages_update_reported_locations(PagedDisplay *display, const char *o3_location, const char *pm25_location, const char *quality_location) {
  update_page_location(display->getPage(0, 1), o3_location);
  update_page_location(display->getPage(1, 1), pm25_location);
  update_page_location(display->getPage(2, 1), quality_location);

  display->display();
}

static void update_page_location(GFXCanvas1 *page, const char *location) {
  page->fillRect(LOCATION_AREA_X, LOCATION_AREA_Y, LOCATION_AREA_WIDTH, LOCATION_AREA_HEIGHT, BLACK);

  page->setTextSize(LOCATION_TEXT_SIZE);
  page->setCursor(LOCATION_X, LOCATION_Y);

  page->print(location);
}

static void print_pms_data(GFXCanvas1 *page, const char *label, int data) {
  page->fillRect(DATA_AREA_X, DATA_AREA_Y, DATA_AREA_WIDTH, DATA_AREA_HEIGHT, BLACK);
  page->setTextColor(WHITE);
  page->setTextWrap(false);

  page->setTextSize(LABEL_TEXT_SIZE);
  page->setCursor(LABEL_X, LABEL_Y);

  page->print(label);

  page->setTextSize(DATA_TEXT_SIZE);
  page->setCursor(DATA_X, DATA_Y);

  if (data < 0) {
    page->print(" ---");
  }
  else if (data > 999) {
    page->print(">999");
  }
  else {
    page->printf(" %d", data);
  }
}

static void print_pms_text(GFXCanvas1 *page, const char *text, bool small) {
  page->fillRect(DATA_AREA_X, DATA_AREA_Y, DATA_AREA_WIDTH, DATA_AREA_HEIGHT, BLACK);
  page->setTextColor(WHITE);
  page->setTextWrap(false);

  int size = small ? QUALITY_TEXT_SIZE_SMALL : QUALITY_TEXT_SIZE_LARGE;
  int x = QUALITY_X;
  int y = small ? QUALITY_Y_SMALL : QUALITY_Y_LARGE;

  page->setTextSize(size);
  page->setCursor(x, y);

  page->print(text);
}

  // test pattern
  // for (int16_t y=0, w=1; y < 32; y += w, w <<= 1) {
  //   LOG(LL_INFO, ("y=%d, w=%d", y, w));
  //
  //   for (int16_t x=0; x < 128; x += w<<1) {
  //     page->fillRect(x, y, w, w, WHITE);
  //   }
  // }
