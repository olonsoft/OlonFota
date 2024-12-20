#include "OlonFOTA.h"

#ifdef OLON_DEBUG_FOTA
  #define LOGD(tag, format, ...) {                              \
      Serial.printf("\033[0;36m%6lu [D] [%s] ", millis(), tag); \
      Serial.printf(format "\033[0m\n", ##__VA_ARGS__); }
  #define LOGI(tag, format, ...) {                              \
      Serial.printf("\033[0;32m%6lu [I] [%s] ", millis(), tag); \
      Serial.printf(format "\033[0m\n", ##__VA_ARGS__); }
  #define LOGW(tag, format, ...) {                              \
      Serial.printf("\033[0;33m%6lu [W] [%s]", millis(), tag);  \
      Serial.printf(format "\033[0m\n", ##__VA_ARGS__); }
  #define LOGE(tag, format, ...) {                              \
      Serial.printf("\033[0;31m%6lu [E] [%s] ", millis(), tag); \
      Serial.printf(format "\033[0m\n", ##__VA_ARGS__); }
  #define LOGBULK(...) Serial.printf(PSTR("%s"), ##__VA_ARGS__);
#else
  #define LOGD(tag, format, ...)
  #define LOGI(tag, format, ...)
  #define LOGW(tag, format, ...)
  #define LOGE(tag, format, ...)
  #define LOGBULK(...)
#endif

Olon::FOTA* Olon::FOTA::instance = nullptr;

static const char* const TAG = "FOTA";

Olon::FOTA::FOTA() {
  instance = this;
}

void Olon::FOTA::init() {

  // Add optional callback notifiers

  ESP_UPD.rebootOnUpdate(false);
  ESP_UPD.closeConnectionsOnUpdate(false);

  ESP_UPD.onStart([]() {
    char buffer[100];
    snprintf_P(buffer, sizeof(buffer), PSTR("Updating FW to: %s"), instance->_url.c_str());
    instance->doEventCallback(FOTAEvent_t::UpdateStart, buffer);
  });

  ESP_UPD.onProgress([](int cur, int total) {
    LOGD(TAG, "Updating %d of %d", cur, total);
    instance->doProgressEventCallback(cur, total);
  });

  ESP_UPD.onEnd([]() {
    char buffer[20];
    snprintf_P(buffer, sizeof(buffer), PSTR("Update Finished"));
    instance->doEventCallback(FOTAEvent_t::UpdateFinished, buffer);
  });

  ESP_UPD.onError([](int err) {
    LOGD(TAG, "Update error: %d", err);
    char buffer[30];
    snprintf_P(buffer, sizeof(buffer), PSTR("Update error: %d"), err);
    instance->_errorNumber = err; // ESP_UPD.getLastError();
    instance->_errorString = ESP_UPD.getLastErrorString();
    instance->doEventCallback(FOTAEvent_t::UpdateError, buffer);
  });

}

void Olon::FOTA::registerFOTAEventCallback(FOTAEventCb callback) {
  if (callback) {
    _FOTAEventCb = callback;
  }
}

void Olon::FOTA::registerFOTAProgressEventCallback(FOTAProgressEventCb callback){
  if (callback) {
    _FOTAProgressEventCb = callback;
  }
}

void Olon::FOTA::doEventCallback(FOTAEvent_t event, char* text) {
  LOGD(TAG, "%s", text);
  _FOTAEventCb(event, text);
}

void Olon::FOTA::doProgressEventCallback(int current, int total) {
  _FOTAProgressEventCb(current, total);
}

bool Olon::FOTA::updateFirmware(String bin_url) {
  _url = bin_url;

  WiFiClient client;

  ESP_UPD.setLedPin(LED_BUILTIN, LOW);

  t_httpUpdate_return ret = ESP_UPD.update(client, _url);

  return (ret == HTTP_UPDATE_OK);
}

bool Olon::FOTA::updateFileSystem(String FOTA_fs_url) {
  _url = FOTA_fs_url;
  WiFiClient client;
  bool result = false;

  ESP_UPD.rebootOnUpdate(false);

#ifdef ESP32
  t_httpUpdate_return ret = ESP_UPD.updateSpiffs(client, FOTA_fs_url);
#elif defined(ESP8266)
  t_httpUpdate_return ret = ESP_UPD.updateFS(client, FOTA_fs_url);
#endif

  if (ret != HTTP_UPDATE_OK) {
    result = false;
    _errorNumber = ESP_UPD.getLastError();
    _errorString = ESP_UPD.getLastErrorString();
  } else {
    result = true;
  }
  return result;
}

int Olon::FOTA::getErrorNumber() {
  return _errorNumber;
}

String Olon::FOTA::getErrorString() {
  return _errorString;
}

