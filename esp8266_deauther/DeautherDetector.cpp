#include "DeautherDetector.h"

// ===== SETTINGS ===== //

#define LED 2              /* LED pin (2=built-in LED) */
#define LED_INVERT true    /* Invert HIGH/LOW for LED */
#define SERIAL_BAUD 9600 /* Baudrate for serial communication */
#define CH_TIME 140        /* Scan time (in ms) per channel */
#define PKT_RATE 5         /* Min. packets before it gets recognized as an attack */
#define PKT_TIME 1         /* Min. interval (CH_TIME*CH_RANGE) before it gets recognized as an attack */

const short channels[] = { 1,2,3,4,5,6,7,8,9,10,11,12,13/*,14*/ };

int ch_index { 0 };               // Current index of channel array
int packet_rate { 0 };            // Deauth packet counter (resets with each update)
int attack_counter { 0 };         // Attack counter
unsigned long update_time { 0 };  // Last update time
unsigned long ch_time { 0 };      // Last channel hop time

// ===== SETTINGS ===== //

int DeautherDetector::getAttackCounter()
{
  return attack_counter;
}

bool DeautherDetector::isDeauthDetected()
{
  if(detected)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool DeautherDetector::isRunning()
{
  if(running)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void DeautherDetector::attack_started() 
{
  //digitalWrite(LED, !LED_INVERT); // turn LED on
  led::setColor(255, 0, 0);
  
  Serial.println("Deauth Attack Detected !");
}

void DeautherDetector::attack_stopped() 
{
  //digitalWrite(LED, LED_INVERT); // turn LED off
  led::setColor(0, 255, 0);
  Serial.println("0 Deauth Attack Detected !");
}

void sniffer(uint8_t *buf, uint16_t len) 
{
  if (!buf || len < 28) return; // Drop packets without MAC header

  byte pkt_type = buf[12]; // second half of frame control field
  //byte* addr_a = &buf[16]; // first MAC address
  //byte* addr_b = &buf[22]; // second MAC address

  // If captured packet is a deauthentication or dissassociaten frame
  if (pkt_type == 0xA0 || pkt_type == 0xC0) {
    ++packet_rate;
  }
}

void DeautherDetector::start()
{
  if(!running)
  {
    Serial.begin(SERIAL_BAUD); // Start serial communication

    pinMode(LED, OUTPUT); // Enable LED pin
    led::setColor(0, 0, 255);
    delay(500);
    led::setColor(128, 128, 128);
    delay(500);
    led::setColor(0,0,255);
    delay(500);
    led::setColor(128, 128, 128);
    delay(500);
    led::setColor(0,0,255);

    WiFi.disconnect();                   // Disconnect from any saved or active WiFi connections
    wifi_set_opmode(STATION_MODE);       // Set device to client/station mode
    wifi_set_promiscuous_rx_cb(sniffer); // Set sniffer function
    wifi_set_channel(channels[0]);        // Set channel
    wifi_promiscuous_enable(true);       // Enable sniffer

    Serial.println("DeautherDetector loaded !");
    running = true;
  }
  else
  {
    Serial.println("DeautherDetector Unloaded !");
    digitalWrite(LED, LED_INVERT); // turn LED off
    led::setColor(0, 255, 0);
    running = false;
  }
}

void DeautherDetector::stop()
{
  Serial.println("DeautherDetector unloaded !");
  running = false;
}

void DeautherDetector::update()
{
  if(running)
  {
    unsigned long current_time = millis(); // Get current time (in ms)

    // Update each second (or scan-time-per-channel * channel-range)
    if (current_time - update_time >= (sizeof(channels)*CH_TIME)) {
      update_time = current_time; // Update time variable

      // When detected deauth packets exceed the minimum allowed number
      if (packet_rate >= PKT_RATE) {
        ++attack_counter; // Increment attack counter
      } else {
        if(attack_counter >= PKT_TIME) attack_stopped();
        attack_counter = 0; // Reset attack counter
      }

      // When attack exceeds minimum allowed time
      if (attack_counter == PKT_TIME) {
        attack_started();
      }

      Serial.print("Packets/s: ");
      Serial.println(packet_rate);

      packet_rate = 0; // Reset packet rate
    }

    // Channel hopping
    if (sizeof(channels) > 1 && current_time - ch_time >= CH_TIME) {
      ch_time = current_time; // Update time variable

      // Get next channel
      ch_index = (ch_index+1) % (sizeof(channels)/sizeof(channels[0]));
      short ch = channels[ch_index];

      // Set channel
      //Serial.print("Set channel to ");
      //Serial.println(ch);
      wifi_set_channel(ch);
    }
  }
}
