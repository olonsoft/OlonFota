#pragma once

#ifdef ESP8266
  #include <ESP8266httpUpdate.h>
  #define ESP_UPD ESPhttpUpdate
#elif defined(ESP32)
  #include <httpUpdate.h>
  #define ESP_UPD httpUpdate
#endif

namespace Olon {

enum class FOTAEvent_t {
  UpdateStart,
  UpdateFinished,
  UpdateError
};

class FOTA {
 public:
  typedef std::function<void(FOTAEvent_t, char*)> FOTAEventCb;
  typedef std::function<void(int, int)> FOTAProgressEventCb;
  FOTA();
  void   init();
  bool   updateFirmware(String bin_url);
  bool   updateFileSystem(String fs_url);
  int    getErrorNumber();
  String getErrorString();
  void   registerFOTAEventCallback(FOTAEventCb callback);
  void   registerFOTAProgressEventCallback(FOTAProgressEventCb callback);

 private:
  uint16_t _port = 80;
  String   _url;
  String   _host;
  //
  int      _errorNumber;
  String   _errorString;

  static FOTA* instance;
  void         doEventCallback(FOTAEvent_t event, char* text);
  void         doProgressEventCallback(int current, int total);
  String       getPayload();

  FOTAEventCb _FOTAEventCb = nullptr;
  FOTAProgressEventCb _FOTAProgressEventCb = nullptr;
};

} // namespace