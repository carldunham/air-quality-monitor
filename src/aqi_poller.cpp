
#include <cstdio>

#include "common/cs_dbg.h"
#include "mgos_mongoose.h"
#include "mgos_config.h"
#include "mgos_event.h"
#include "mgos_timers.h"
#include "frozen.h"

#include "location.h"
#include "PagedDisplay.h"
#include "pages.h"

#include "aqi_poller.h"


// TODO: consider moving to config?
static const char * const API_URL_FORMAT_STRING = "http://www.airnowapi.org/aq/observation/latLong/current/?format=application/json&latitude=%.4f&longitude=%.4f&distance=%d&API_KEY=%s";
static const int MAX_URL_SIZE = strlen(API_URL_FORMAT_STRING) + 64;  // should be more than enough
static const int POLL_INTERVAL_MS = 5 * 60 * 1000;  // five minutes

static char url[MAX_URL_SIZE];

static const int MAX_LOCATION_LEN = 32; // better too long than too short, will just get clipped. also, should be set in pages?

static mgos_timer_id polling_timer_id = MGOS_INVALID_TIMER_ID;

class DataElement {
public:
  DataElement(void) : name(nullptr), name_len(0), area(nullptr), area_len(0), state(nullptr), state_len(0), quality(-1), value(-1) {}
  DataElement(const DataElement&) = delete;
  virtual ~DataElement(void) = default;

  virtual DataElement& operator=(const DataElement&) = default;

  void Reset(void) {
    this->name = nullptr;
    this->name_len = 0;
    this->area = nullptr;
    this->area_len = 0;
    this->state = nullptr;
    this->state_len = 0;
    this->quality = -1;
    this->value = -1;
    this->_location[0] = '\0';
  }

  const char *location() {
    if (strlen(this->_location) == 0 && (this->area_len + this->state_len) > 0) {
      snprintf(this->_location, sizeof(this->_location), "%.*s, %.*s", this->area_len, this->area, this->state_len, this->state);
    }
    return this->_location;
  }

  const char *name;
  int name_len;
  const char *area;
  int area_len;
  const char *state;
  int state_len;
  int quality;
  int value;

private:
  char _location[MAX_LOCATION_LEN] = "";
};

class PollState {
public:
  PollState(void) : cur(), ozone(), pm25() {}
  PollState(const PollState&) = delete;
  virtual ~PollState(void) = default;

  virtual PollState& operator=(const PollState&) = default;

  int depth = 0;

  DataElement cur;
  DataElement ozone;
  DataElement pm25;
};

static void handle_location_change(int ev, void *ev_data, void *userdata);
static void polling_timer_cb(void *userdata);
static void http_event_cb(struct mg_connection *c, int ev, void *ev_data, void *userdata);
static void json_cb(void *callback_data, const char *name, size_t name_len, const char *path, const struct json_token *token);

void aqi_poller_init(PagedDisplay *display) {
  mgos_event_add_handler(LOCATION_CHANGE_EVENT, handle_location_change, (void *)display);
}

void handle_location_change(int ev, void *ev_data, void *userdata) {
  if (ev_data != nullptr) {
    struct mgos_config_app_loc *locinfo = (struct mgos_config_app_loc *)ev_data;

    snprintf(url, sizeof(url), API_URL_FORMAT_STRING, locinfo->lat, locinfo->lon, locinfo->radius, mgos_sys_config_get_app_airnow_api_key());

    // start polling
    polling_timer_id = mgos_set_timer(POLL_INTERVAL_MS, MGOS_TIMER_REPEAT|MGOS_TIMER_RUN_NOW, polling_timer_cb, userdata);

    if (polling_timer_id == MGOS_INVALID_TIMER_ID) {
      LOG(LL_WARN, ("Unable to set polling timer"));
    }
  }
  else if (polling_timer_id != MGOS_INVALID_TIMER_ID) {
    mgos_clear_timer(polling_timer_id);
    polling_timer_id = MGOS_INVALID_TIMER_ID;
  }
}

static void polling_timer_cb(void *userdata) {
  LOG(LL_INFO, ("Connecting to API URL: %s", url));

  // poll
  if (!mg_connect_http(mgos_get_mgr(), http_event_cb, userdata, url, nullptr, nullptr)) {
    LOG(LL_WARN, ("Unable to connect to API URL: %s", url));
  }
}

static void http_event_cb(struct mg_connection *c, int ev, void *ev_data, void *userdata) {
  struct http_message *hm = (struct http_message *)ev_data;
  PagedDisplay *display = (PagedDisplay *)userdata;

  switch (ev) {
    case MG_EV_CONNECT:
      LOG(LL_INFO, ("Connected: status=%d", *(int *)ev_data));
      break;
    case MG_EV_HTTP_REPLY:
      LOG(LL_DEBUG, ("Reply received, %d bytes", hm->body.len));
      // c->flags |= MG_F_CLOSE_IMMEDIATELY;
      {
        PollState ps;

        if (json_walk(hm->body.p, hm->body.len, json_cb, (void *)&ps) < 0) {
          LOG(LL_WARN, ("Unable to parse JSON from AirNow: %.*s", hm->body.len, hm->body.p));
        }
        LOG(LL_INFO, ("Ozone: value=%d, quality=%d", ps.ozone.value, ps.ozone.quality));
        LOG(LL_INFO, ("PM2.5: value=%d, quality=%d", ps.pm25.value, ps.pm25.quality));

        if (display != nullptr) {
          int quality_level = ps.pm25.quality;
          const char *quality_location = ps.pm25.location();

          if (ps.ozone.quality > ps.pm25.quality) {
            quality_level = ps.ozone.quality;
            quality_location = ps.ozone.location();
          }
          pages_update_reported_data(display, ps.pm25.value, ps.ozone.value, quality_level);
          pages_update_reported_locations(display, ps.ozone.location(), ps.pm25.location(), quality_location);
        }
      }
      break;
    case MG_EV_CLOSE:
      LOG(LL_DEBUG, ("Closed connection"));
      break;
  }
}

static void json_cb(void *callback_data, const char *name, size_t name_len, const char *path, const struct json_token *token) {
  struct PollState *ps = (struct PollState *)callback_data;

  switch (token->type) {
    case JSON_TYPE_STRING:
      LOG(LL_DEBUG, ("string: depth=%d, name='%.*s' value='%.*s' path='%s'", ps->depth, name_len, name, token->len, token->ptr, path));

      if (strncmp(name, "ParameterName", name_len) == 0) {
        ps->cur.name = token->ptr;
        ps->cur.name_len = token->len;
      }
      else if (strncmp(name, "ReportingArea", name_len) == 0) {
        ps->cur.area = token->ptr;
        ps->cur.area_len = token->len;
      }
      else if (strncmp(name, "StateCode", name_len) == 0) {
        ps->cur.state = token->ptr;
        ps->cur.state_len = token->len;
      }
      break;
    case JSON_TYPE_NUMBER:
      LOG(LL_DEBUG, ("number: depth=%d, name='%.*s' value='%.*s' path='%s'", ps->depth, name_len, name, token->len, token->ptr, path));
      if (strncmp(name, "AQI", name_len) == 0) {
        ps->cur.value = atoi(token->ptr);  // counts on atoi() getting a good non-numeric following the number (like ',' or '}')
      }
      else if (strncmp(name, "Number", name_len) == 0) {
        ps->cur.quality = atoi(token->ptr);  // ditto
      }
      break;
    case JSON_TYPE_OBJECT_START:
      LOG(LL_DEBUG, ("object start: depth=%d, name='%.*s', path='%s'", ps->depth, name == nullptr ? 4 : name_len, name == nullptr ? "NULL" : name, path));
      ps->depth++;
      break;
    case JSON_TYPE_OBJECT_END:
      LOG(LL_DEBUG, ("object end: depth=%d, name='%.*s', path='%s'", ps->depth, name == nullptr ? 4 : name_len, name == nullptr ? "NULL" : name, path));
      if (--ps->depth == 0) {
        if (strncmp(ps->cur.name, "O3", ps->cur.name_len) == 0) {
          ps->ozone = ps->cur;
          LOG(LL_INFO, ("Stashed O3, value=%d, location=%s", ps->ozone.value, ps->ozone.location()));
        }
        else if (strncmp(ps->cur.name, "PM2.5", ps->cur.name_len) == 0) {
          ps->pm25 = ps->cur;
          LOG(LL_INFO, ("Stashed PM2.5, value=%d, location=%s", ps->pm25.value, ps->pm25.location()));
        }
        else if (ps->cur.name == nullptr) {
          LOG(LL_WARN, ("No metric found, ignoring"));
        }
        else {
          LOG(LL_WARN, ("Ignoring extra metric %.*s", ps->cur.name_len, ps->cur.name));
        }
        ps->cur.Reset();
      }
      break;
    default:
      // ignore
      break;
  }
}
