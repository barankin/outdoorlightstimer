/*
   A program that turns the lights on at sunset and turns them off at a certain time.
   And turns them on at a certain time and off after sunrise.

   Uses libraries
   1. https://github.com/stelgenhof/NTPClient.git
   2. https://github.com/dmkishi/Dusk2Dawn
   3. Arduino Time library

   and board manager
   1. https://github.com/esp8266/Arduino#installing-with-boards-manager
*/

#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <Dusk2Dawn.h>
#include <Time.h>

/***************************************************/
const int relayPin = D6;// the number of the Relay pin
const int ledPin =  LED_BUILTIN;// the number of the LED pin
const long interval = 10000;           // interval which to check the time (milliseconds)

const char *WIFI_SSID = "ford_prefect_2.4"; //  Your WiFi network SSID (name)
const char *WIFI_PSK = "annabanana";  // Your WiFi network PSK (password)

char *turnOffTime = "23:00"; //turn off at this time.
char *turnOnTime = "6:00"; //turn on at this time in the morning (and then turn off about 30 minutes after sunrise)
/***************************************************/

/***************************************************/
// variable for the state of the relay
int relayState = HIGH;
unsigned long previousMillis = 0;
/***************************************************/

int DST = 0; //DST is 1, DST off is 0

/***************************************************/
//Build global objects
//Set up sunset determination
Dusk2Dawn toronto(43.666468, -79.477260, -5);
/***************************************************/

/***************************************************/
//Internal function declarations
void onSTAGotIP(WiFiEventStationModeGotIP event);
void onSTADisconnected(WiFiEventStationModeDisconnected event);

int getHourFromString(char * timeString);
int getMinuteFromString(char * timeString);
/***************************************************/

void setup() {
  static WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;

  /***************************************************/
  //Set up serial
  Serial.begin(115200);
  Serial.println();
  Serial.println("Serial up and running");
  /***************************************************/

  /***************************************************/
  //Set up relay
  pinMode(relayPin, OUTPUT); //set relay pin to an output
  pinMode(ledPin, OUTPUT);
  digitalWrite(relayPin, relayState); //initialize relay pin to the relay state (LOW at startup)
  digitalWrite(ledPin,relayState);
  /***************************************************/

  /***************************************************/
  //Set up NTP events
  NTP.onSyncEvent([](NTPSyncEvent_t ntpEvent) {
    switch (ntpEvent) {
      case NTP_EVENT_INIT:
        break;
      case NTP_EVENT_STOP:
        break;
      case NTP_EVENT_NO_RESPONSE:
        Serial.printf("NTP server not reachable.\n");
        break;
      case NTP_EVENT_SYNCHRONIZED:
        //Serial.printf("Time number is %d\n",NTP.getLastSync());
        Serial.printf("Got NTP time: %s\n", NTP.getTimeDate(NTP.getLastSync()));
        break;
    }
  });

  gotIpEventHandler = WiFi.onStationModeGotIP(onSTAGotIP);
  disconnectedEventHandler = WiFi.onStationModeDisconnected(onSTADisconnected);
  /***************************************************/

  /***************************************************/
  // Connecting to a WiFi network
  Serial.printf("Connecting to WiFi network: %s \n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  /***************************************************/
}

void loop() {
  unsigned long currentMillis = millis();

  // Update time status every interval
  if ((currentMillis - previousMillis) > interval) {
    previousMillis = currentMillis;

    // Output only when time is set
    if (timeStatus() != timeSet) {
      return;
    }

    time_t currentTime = now(); //get the current time / date

    //get sunset for today (if not daylight savings)
    int sunset = toronto.sunset(year(currentTime), month(currentTime), day(currentTime), false);

    //get sunrise for today (if not daylight savings)
    int sunrise = toronto.sunrise(year(currentTime), month(currentTime), day(currentTime), false);

    char sunsetTimeString[6];
    Dusk2Dawn::min2str(sunsetTimeString, sunset);
    
    char sunriseTimeString[6];
    Dusk2Dawn::min2str(sunriseTimeString, sunrise);

    //int sunsetHourInt = getHourFromString(sunsetTimeString);
    //int sunsetHourInt = 12;
    //int sunsetMinuteInt = getMinuteFromString(sunsetTimeString);
    int sunsetMinutes = (60 * getHourFromString(sunsetTimeString)) + getMinuteFromString(sunsetTimeString);
    //int sunsetMinutes = 830;
    //int offHourInt = getHourFromString(turnOffTime);
    //int offMinuteInt = getMinuteFromString(turnOffTime);
    int offMinutes = (60 * getHourFromString(turnOffTime)) + getMinuteFromString(turnOffTime);
    //int offMinutes = 834;
    
    int sunriseMinutes = (60 * getHourFromString(sunriseTimeString)) + getMinuteFromString(sunriseTimeString);
    
    int onMinutes = (60 * getHourFromString(turnOnTime)) + getMinuteFromString(turnOnTime);

    int currentTimeMinutes = (60*hour(currentTime)) + minute(currentTime);
    
    Serial.printf("Sunset is at %d.\n", sunsetMinutes);
    Serial.printf("Current time is %d.\n", currentTimeMinutes);
    Serial.printf("Off time is at %d.\n", offMinutes);
    
    Serial.printf("On time is at %d.\n", onMinutes);
    Serial.printf("Current time is %d.\n", currentTimeMinutes);
    Serial.printf("Sunrise is at %d.\n", sunriseMinutes);

    if((currentTimeMinutes >= sunsetMinutes && currentTimeMinutes < offMinutes)|| (currentTimeMinutes>onMinutes && currentTimeMinutes < (sunriseMinutes+30))){
      relayState = LOW;
    }else{
      relayState = HIGH;
    }

    //Serial.printf("Time number is %d\n",NTP.getLastSync());
    //Serial.printf("Current time: %s - First synchronized at: %s.\n",
    //              NTP.getTimeDate(currentTime), NTP.getTimeDate(NTP.getFirstSync()));
    //Serial.printf("The hour is %d, the minute is %d and seconds are %d.\n",hour(currentTime),minute(currentTime),second(currentTime));
    //Serial.println();

    digitalWrite(relayPin, relayState);
    digitalWrite(ledPin, relayState);
    //Serial.println(relayState);
  }
}

/***************************************************/
//Event Handlers
// Event Handler when an IP address has been assigned
// Once connected to WiFi, start the NTP Client
void onSTAGotIP(WiFiEventStationModeGotIP event) {
  Serial.printf("Got IP: %s\n", event.ip.toString().c_str());
  NTP.init((char *)"pool.ntp.org", UTC_0500);
  //NTP.setPollingInterval(15); // Poll
}

// Event Handler when WiFi is disconnected
void onSTADisconnected(WiFiEventStationModeDisconnected event) {
  Serial.printf("WiFi connection (%s) dropped.\n", event.ssid.c_str());
  Serial.printf("Reason: %d\n", event.reason);
}
/***************************************************/

int getHourFromString(char * timeString) {
  char hourString[3];
  hourString[0] = timeString[0];
  hourString[1] = timeString[1];
  hourString[2] = timeString[5];

  return atoi(hourString);
}

int getMinuteFromString(char * timeString) {

  char minuteString[3];
  minuteString[0] = timeString[3];
  minuteString[1] = timeString[4];
  minuteString[2] = timeString[5];
  return atoi(minuteString);
}
