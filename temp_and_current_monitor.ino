/*************************************************************
  Blynk is a platform with iOS and Android apps to control
  ESP32, Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build mobile and web interfaces for any
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: https://www.blynk.io
    Sketch generator:           https://examples.blynk.cc
    Blynk community:            https://community.blynk.cc
    Follow us:                  https://www.fb.com/blynkapp
                                https://twitter.com/blynk_app

  Blynk library is licensed under MIT license
 *************************************************************
  Blynk.Edgent implements:
  - Blynk.Inject - Dynamic WiFi credentials provisioning
  - Blynk.Air    - Over The Air firmware updates
  - Device state indication using a physical LED
  - Credentials reset using a physical Button
 *************************************************************/

/* Fill in information from your Blynk Template here */
/* Read more: https://bit.ly/BlynkInject */
//#define BLYNK_TEMPLATE_ID           "TMPxxxxxx"
//#define BLYNK_TEMPLATE_NAME         "Device"

#define BLYNK_TEMPLATE_ID "TMPL3zBdKUhpc"
#define BLYNK_TEMPLATE_NAME "Temp n Current Probe"

#define BLYNK_FIRMWARE_VERSION "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_SPARKFUN_BLYNK_BOARD
#define USE_NODE_MCU_BOARD
//#define USE_WITTY_CLOUD_BOARD
//#define USE_WEMOS_D1_MINI

#include "BlynkEdgent.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EmonLib.h>

// Temprature Data wire is connected to GPIO 4
#define TEMP_ONE_WIRE_BUS 4
// Setup a oneWire instance to communicate with any OneWire devices
OneWire TempOneWire(TEMP_ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature tempSensors(&TempOneWire);

// Current Analog wire is connected to A0
#define CURRENT_SENSOR_PIN A0
#define CURRENT_THRESHOLD 28.0  // Set warning threshold below 30A

EnergyMonitor emon1;             // Create an EMON instance

// Timer function
BlynkTimer timer;

// This function is called every time the Virtual Pin 0 & 1 state changes
void sendCurrentTemp()
{
  // Set incoming value from pin V0 to a variable
  tempSensors.requestTemperatures(); 
  float tempC = tempSensors.getTempCByIndex(0);
  Serial.print("Temperature: ");
  Serial.println(tempC);

  float currentA = getCurrent();
  Serial.print("Current: ");
  Serial.println(currentA);

  // Update state
  Blynk.virtualWrite(V0, tempC);
  Blynk.virtualWrite(V1, currentA);  // current in Amps

  if (currentA > CURRENT_THRESHOLD) {
    // Show overload alert on Virtual Pin 4 (could be a LED or display)
    Blynk.virtualWrite(V4, 1);
    Serial.println("⚠️ Current Overload Detected!");
  } else {
    Blynk.virtualWrite(V4, 0);
  }
}

float getCurrent()
{
  float sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += emon1.calcIrms(1480); // 1480 samples for stable reading
  }
  float avg = sum / 10;
  return (avg < 0.1) ? 0.0 : avg;
  // return emon1.calcIrms(1480); // Without smoothing or deadzone limitter
}


void setup()
{
  Serial.begin(115200);
  delay(100);

  BlynkEdgent.begin();

  tempSensors.begin();

  // Calibration constant: 62 (62 was decide after testing, this may vary for other units) 
  // for SCT-013-030 with 1V output to 1V ADC (NodeMCU)
  // Adjust if needed after testing
  emon1.current(CURRENT_SENSOR_PIN, 62);

  // Setup a function to be called every second
  timer.setInterval(1000L, sendCurrentTemp);
}

void loop() {
  BlynkEdgent.run();
  timer.run();
}

