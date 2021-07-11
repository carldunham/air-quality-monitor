#ifndef _GFX_CANVAS_H_
#define _GFX_CANVAS_H_

#include <Adafruit_GFX.h>

// cribbed from https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_GFX.h

/// A GFX 1-bit canvas context for graphics
class GFXCanvas1 : public Adafruit_GFX {
 public:
  GFXCanvas1(uint16_t w, uint16_t h);
  ~GFXCanvas1(void);
  void     drawPixel(int16_t x, int16_t y, uint16_t color),
           fillScreen(uint16_t color);
  uint8_t *getBuffer(void);
 private:
  uint8_t *buffer;
};

#endif // _GFX_CANVAS_H_
