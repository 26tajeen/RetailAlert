#include "RetailAlert.h"

using namespace RetailAlert;

SYSTEM_THREAD(ENABLED);

// PUB-SUB constants
constexpr char* DEVICE_NAME = "RETAIL_ALERT";
constexpr char* CARRY_MSSG = "CARRY";
constexpr char* ASSISTANCE_MSSG = "ASSISTANCE";
constexpr char* CHANGE_NEEDED_MSSG = "CHANGE";
constexpr char* BUZZER_MSSG = "BUZZ";

// function declarations
void eventHandler(const char* event, const char* data);

/*
Create ButtonPressAction objects
Constructor expects a pointer to function that executes on button press for example:
ButtonPressAction someButtonName(someFunction);
here we are just using lambdas */
ButtonPressAction carryButton([](){
  Particle.publish(DEVICE_NAME, CARRY_MSSG, 60, PRIVATE);
});

ButtonPressAction assistanceButton([](){
  Particle.publish(DEVICE_NAME, ASSISTANCE_MSSG, 60, PRIVATE);
});

ButtonPressAction changeNeededButton([](){
  Particle.publish(DEVICE_NAME, CHANGE_NEEDED_MSSG, 60, PRIVATE);
});

/*
create IndicatorLED objects
constructor expects the LED pin */
IndicatorLED needChangeLed(D1);

/*
This create ToggleSwitchAction
the constructor takes two function pointers, "closedAction" and "openAction" for example:
ToggleSwitchAction toggleName(someClosedFunction, someOpenFunction);
here we are just using two lambdas instead */
ToggleSwitchAction shoplifterToggle(
  [](){
    Serial << "ShoplifterToggle Closed!\n";
    String deviceID = Particle.deviceID();
    Particle.publish(DEVICE_NAME, String(BUZZER_MSSG) + "_ON#" + deviceID, 60, PRIVATE);
  },
  [](){
    Serial << "ShoplifterToggle Opened!\n";
    String deviceID = Particle.deviceID();
    Particle.publish(DEVICE_NAME, String(BUZZER_MSSG) + "_OFF#" + deviceID, 60, PRIVATE);
  }
);

// create Buzzer on pin
// constructor expects the output pin for the relay
Buzzer buzzer(D7);

void timerCallback (void){
  Serial << "Timer Expired!!\n";
}
// constructor takes the time in milliseconds, callback function and true if you want it to repeat.
MillisTimer myTimer(10000, timerCallback, false);

void setup()
{
  //Particle.subscribe(DEVICE_NAME, eventHandler, MY_DEVICES);
  Serial.begin(9600);

  uint32_t startMillis = millis();
  while (millis() - startMillis < 5000){
  }

  carryButton.bindToPin(D2);
  assistanceButton.bindToPin(D3);
  changeNeededButton.bindToPin(D4);
  needChangeLed.begin();
  shoplifterToggle.bindToPin(D5);
  buzzer.begin();
  myTimer.begin();
  myTimer.start(false);
}

void loop()
{
  MillisTimer::processTimers();
  ButtonPressAction::update();
  ToggleSwitchAction::update();
  buzzer.buzz();

}

void eventHandler(const char* event, const char* data)
{
  if(strcmp(data, CHANGE_NEEDED_MSSG) == 0)
  {
    Serial << "Change Needed\n";
    needChangeLed.on();
  }
  else if (strcmp(data, ASSISTANCE_MSSG) == 0)
  {
    Serial << "Assistance Needed\n";
  }
  else if (strcmp(data, CARRY_MSSG) == 0)
  {
    Serial << "Carry Needed\n";
  }
  else if(strstr(data, BUZZER_MSSG))
  {
    Serial << "Shoplifter Buzzer!!\n";
    Serial << "data: " << data << "\n";
    char mssgBuffer[128];
    strcpy (mssgBuffer, data);
    strtok(mssgBuffer, "#");
    String callerDeviceID = (strtok(NULL, "#"));
    Serial << "Device ID: " << callerDeviceID << "\n";
    if (strstr(data, "ON"))
    {
      buzzer.alarmOn(callerDeviceID);
      Serial << "Requested Alarm On for this Device...\n\n";
    }
    else if (strstr(data, "OFF"))
    {
      buzzer.alarmOff(callerDeviceID);
      Serial << "Requested Alarm Off for this Device...\n\n";
    }
  }
}