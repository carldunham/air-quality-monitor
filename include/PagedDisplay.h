#ifndef _PAGED_DISPLAY_H_
#define _PAGED_DISPLAY_H_

#include <stdlib.h>
#include <vector>

#include <Adafruit_SSD1306.h>

#include "GFXCanvas.h"


class PagedDisplay {

public:
  static const int PAGE_WIDTH = 128;
  static const int PAGE_HEIGHT = 32;

  PagedDisplay(int rows, int cols);
  PagedDisplay(const PagedDisplay&) = delete;
  virtual ~PagedDisplay(void);

  virtual PagedDisplay& operator=(const PagedDisplay&) = delete;

  virtual GFXCanvas1 *getPage(int row, int col);
  virtual GFXCanvas1 *getOverlay() { return &this->_overlay; }

  virtual bool display(void);
  virtual bool displayPage(int row, int col, int scrollSpeed=0);
  virtual bool scrollUp(int scrollSpeed, bool wrap);
  virtual bool scrollDown(int scrollSpeed, bool wrap);
  virtual bool scrollLeft(int scrollSpeed, bool wrap);
  virtual bool scrollRight(int scrollSpeed, bool wrap);

private:
  int _rows, _cols;
  int _row, _col;
  std::vector<std::vector<GFXCanvas1 *>> _pages;
  GFXCanvas1 _overlay;
  Adafruit_SSD1306 _display;  // Could make this generic, but may want device-specific functionality
  bool _display_lock;  // Note: mgos on esp32 only uses one core, and mgos_rlock is a noop. So this should be sufficient
};

#endif // _PAGED_DISPLAY_H_
