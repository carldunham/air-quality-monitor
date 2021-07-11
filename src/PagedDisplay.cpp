
#include "common/cs_dbg.h"

#include "PagedDisplay.h"

PagedDisplay::PagedDisplay(int rows, int cols):
  _rows(rows), _cols(cols),
  _row(0), _col(0),
  // _pages(std::vector<std::vector<GFXCanvas1 *>>()),
  _overlay(GFXCanvas1(PagedDisplay::PAGE_WIDTH, PagedDisplay::PAGE_HEIGHT)),
  _display(Adafruit_SSD1306(-1 /* RST GPIO */, Adafruit_SSD1306::RES_128_32)),
  _display_lock(true)
{
  for (int i=0; i<this->_rows; i++) {
    this->_pages.push_back(std::vector<GFXCanvas1 *>(this->_cols, nullptr));
  }
  this->_display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);
  this->_display.clearDisplay();  // TODO: my own splash page
  this->_display.display();

  this->_display_lock = false;

  LOG(LL_INFO, ("PagedDisplay started"));
}

PagedDisplay::~PagedDisplay(void) {}

GFXCanvas1 *PagedDisplay::getPage(int row, int col) {
  if (row >= this->_rows || col >= this->_cols) {
    LOG(LL_WARN, ("invalid page: (%d, %d)", row, col));
    return nullptr;
  }
  // LOG(LL_INFO, ("this->_pages.size(): %d", this->_pages.size()));
  // LOG(LL_INFO, ("this->_pages[%d].size(): %d", row, this->_pages[row].size()));

  if (this->_pages[row][col] == nullptr) {
    this->_pages[row][col] = new GFXCanvas1(PagedDisplay::PAGE_WIDTH, PagedDisplay::PAGE_HEIGHT);
  }
  return this->_pages[row][col];
}

bool PagedDisplay::display(void) {
  return this->displayPage(this->_row, this->_col);
}

bool PagedDisplay::displayPage(int row, int col, int scrollSpeed) {
  if (row >= this->_rows || col >= this->_cols) {
    LOG(LL_WARN, ("invalid page: (%d, %d)", row, col));
    return false;
  }
  // LOG(LL_INFO, ("this->_pages.size(): %d", this->_pages.size()));
  // LOG(LL_INFO, ("this->_pages[%d].size(): %d", row, this->_pages[row].size()));

  if (this->_pages[row][col] == nullptr) {
    return false;
  }
  this->_display.clearDisplay();
  this->_display.drawBitmap(0, 0, this->_pages[row][col]->getBuffer(), PagedDisplay::PAGE_WIDTH, PagedDisplay::PAGE_HEIGHT, WHITE);
  this->_display.drawBitmap(0, 0, this->_overlay.getBuffer(), PagedDisplay::PAGE_WIDTH, PagedDisplay::PAGE_HEIGHT, WHITE);
  this->_display.display();
  this->_row = row;
  this->_col = col;

  return true;
}

bool PagedDisplay::scrollUp(int scrollSpeed, bool wrap) {
  int row = this->_row-1;

  if (row < 0) {

    if (!wrap) {
      return true;
    }
    row = this->_rows - 1;
  }
  return displayPage(row, this->_col, scrollSpeed);  // TODO: if scrollSpeed != 0, do more optimal scrolling
}

bool PagedDisplay::scrollDown(int scrollSpeed, bool wrap) {
  int row = this->_row+1;

  if (row >= this->_rows) {

    if (!wrap) {
      return true;
    }
    row = 0;
  }
  return displayPage(row, this->_col, scrollSpeed);
}

bool PagedDisplay::scrollLeft(int scrollSpeed, bool wrap) {
  int col = this->_col-1;

  if (col < 0) {

    if (!wrap) {
      return true;
    }
    col = this->_cols - 1;
  }
  return displayPage(this->_row, col, scrollSpeed);
}

bool PagedDisplay::scrollRight(int scrollSpeed, bool wrap) {
  int col = this->_col+1;

  if (col >= this->_cols) {

    if (!wrap) {
      return true;
    }
    col = 0;
  }
  return displayPage(this->_row, col, scrollSpeed);
}

// if (mgos_set_timer(delay, 0, handle_scroll_end_timeout, nullptr) == MGOS_INVALID_TIMER_ID) {
//   LOG(LL_WARN, ("Unable to set timer to stop scrolling"));
//   display->stopscroll();
//   display_lock = false;
//   return;
// }
// LOG(LL_INFO, ("scrolling..."));

// static void handle_scroll_end_timeout(void *arg) {
//   if (display != nullptr) {
//     display->stopscroll();
//     display->dim(false);
//     display->invertDisplay(false);
//     display_lock = false;
//     LOG(LL_INFO, ("scrolling stopped."));
//   }
// }
