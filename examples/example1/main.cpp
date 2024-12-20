#include <Arduino.h>
#include <OlonFOTA.h>

Olon::FOTA fota;

void setup() {
  Serial.begin(115200);
  fota.init();
  fota.registerFOTAEventCallback([](Olon::FOTAEvent_t event, char* info) {
    switch (event) {
      case Olon::FOTAEvent_t::UpdateStart:
        Serial.println("Update started.");
        Serial.println(info);
        break;
      case Olon::FOTAEvent_t::UpdateFinished:
        Serial.println("Update finished.");
        Serial.println(info);
        break;
      case Olon::FOTAEvent_t::UpdateError:
        Serial.println("Update Error");
        Serial.println(info);
        break;
      default:
        break;
    }
  });

  fota.registerFOTAProgressEventCallback([](int current, int total){
    Serial.printf("Updating %d of %d\n", current, total);
  });

}

void loop() {

}