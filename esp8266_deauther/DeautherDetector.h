#ifndef DEAUTHERDETECTOR_H
#define DEAUTHERDETECTOR_H

#include <ESP8266WiFi.h>

// include ESP8266 Non-OS SDK functions
extern "C" {
#include "user_interface.h"
}

#include "led.h"

class DeautherDetector {
  public:
    void start();
    void stop();
    void update();
    void attack_stopped();
    void attack_started();

    int getAttackCounter();

    bool isDeauthDetected();
    bool isRunning();

  private:
    bool detected;
    bool running;
    
};

#endif
