
// based on https://github.com/adafruit/Adafruit_Learning_System_Guides/blob/master/PMS5003_Air_Quality_Sensor/PMS5003_Arduino/PMS5003_Arduino.ino

#include <arpa/inet.h>

#include "common/cs_dbg.h"
#include "mgos_uart.h"

#include "pms5003.h"

static bool checksum(uint8_t *buf);


bool read_pms_data(int uart_no, struct pms5003_data *data) {
  // static struct mbuf lb = {0};
  // assert(uart_no == UART_NO);
  size_t rx_av = mgos_uart_read_avail(uart_no);
  if (rx_av < 32) return false;
  LOG(LL_INFO, ("%d bytes available on UART%d", rx_av, uart_no));

  uint8_t buf[32];
  bool found = false;

  while (!found && (rx_av >= 32)) {
      if (mgos_uart_read(uart_no, &buf, 1) < 1) {
        LOG(LL_WARN, ("error reading header"));
        return false;
      }
      rx_av--;
      if (buf[0] != 0x42) {
        LOG(LL_DEBUG, ("skipping buf[0]=0x%02x", buf[0]));
        continue;
      }

      if (mgos_uart_read(uart_no, &buf[1], 1) < 1) {
        LOG(LL_WARN, ("error reading subheader"));
        return false;
      }
      rx_av--;
      if (buf[1] != 0x4d) {
        LOG(LL_DEBUG, ("skipping buf[1]=0x%02x", buf[1]));
        continue;
      }
      found = true;
  }
  // LOG(LL_DEBUG, ("found=%d, rx_av=%d", found, rx_av));

  if (!found || (rx_av < 30)) return false;

  if ((mgos_uart_read(uart_no, &buf[2], 30) < 30) || !checksum(buf)) {
      LOG(LL_WARN, ("error reading data"));
      return false;
  }

  // for (uint8_t i=0; i<32; i++) {
  //   LOG(LL_DEBUG, ("%02d: 0x%02x", i, buf[i]));
  // }

  uint16_t buf_u16[15];
  for (uint8_t i=0; i<15; i++) {
    buf_u16[i] = ntohs(*((uint16_t *)(&buf[2 + i*2])));
  }
  memcpy((void *)data, (void *)buf_u16, 30);

  return true;
}

static bool checksum(uint8_t *buf) {
  uint16_t sum = 0;
  uint16_t compsum = ntohs(*((uint16_t *)(&buf[30])));

  // it's just a sum (order doesn't matter)
  for (uint8_t i=0; i<30; i++) {
    sum += buf[i];
  }

  // check it
  if (sum != compsum) {
    LOG(LL_WARN, ("Checksum failure"));
    return false;
  }
  return true;
}
