
#include "common/cs_dbg.h"
#include "mgos_config.h"

#include "mgos_mqtt.h"

#include "publish.h"
#include "pms5003.h"

static const struct mgos_config_app_mqtt *mqtt_info = nullptr;

void publish_init() {
  mqtt_info = mgos_sys_config_get_app_mqtt();
  if (mqtt_info == nullptr) {
    LOG(LL_WARN, ("Unable to find app MQTT config!"));
  }
}

const char * const MQTT_MESSAGE_TEMPLATE =
  "{"
  "  pm:{"
  "    raw:{"
  "      %Q:%d,"
  "      %Q:%d,"
  "      %Q:%d"
  "    },"
  "    adj:{"
  "      %Q:%d,"
  "      %Q:%d,"
  "      %Q:%d"
  "    }"
  "  },"
  "  prtcls:{"
  "    %Q:%d,"
  "    %Q:%d,"
  "    %Q:%d,"
  "    %Q:%d,"
  "    %Q:%d,"
  "    %Q:%d"
  "  }"
  "}";

void publish_data(struct pms5003_data *pms_data) {

  if (mqtt_info == nullptr) return;

  bool ok = mgos_mqtt_pubf(mqtt_info->topic, mqtt_info->qos, false,
    MQTT_MESSAGE_TEMPLATE,
    "1.0", pms_data->pm10_standard,
    "2.5", pms_data->pm25_standard,
    "10", pms_data->pm100_standard,
    "1.0", pms_data->pm10_env,
    "2.5", pms_data->pm25_env,
    "10", pms_data->pm100_env,
    ">0.3", pms_data->particles_03um,
    ">0.5", pms_data->particles_05um,
    ">1.0", pms_data->particles_10um,
    ">2.5", pms_data->particles_25um,
    ">5.0", pms_data->particles_50um,
    ">10.0", pms_data->particles_100um
  );

  LOG(ok ? LL_INFO : LL_WARN, ("%s published to MQTT", ok ? "Data" : "NOT"));
}
