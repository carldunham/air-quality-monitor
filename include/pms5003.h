#ifndef _PMS5003_H_
#define _PMS5003_H_

// based on https://github.com/adafruit/Adafruit_Learning_System_Guides/blob/master/PMS5003_Air_Quality_Sensor/PMS5003_Arduino/PMS5003_Arduino.ino

struct pms5003_data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};

bool read_pms_data(int uart_no, struct pms5003_data *data);

#endif // _PMS5003_H_
