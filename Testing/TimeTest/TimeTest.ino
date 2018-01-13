/*
 * will use https://github.com/stelgenhof/NTPClient.git
 */

#include <ESP8266WiFi.h>

const char* ssid = "ford_prefect_2.4";
const char* password = "annabanana";

void setup() {
  Serial.begin(115200); 
  Serial.println(); 
  initWifi();
}

void loop() {
  Serial.println(getTime());
  delay(5000);
}
  
String getTime() {
  WiFiClient client;
  while (!!!client.connect("google.com", 80)) {
    Serial.println("connection failed, retrying...");
  }

  client.print("HEAD / HTTP/1.1\r\n\r\n");
 
  while(!!!client.available()) {
     yield();
  }

  while(client.available()){
    if (client.read() == '\n') {    
      if (client.read() == 'D') {    
        if (client.read() == 'a') {    
          if (client.read() == 't') {    
            if (client.read() == 'e') {    
              if (client.read() == ':') {    
                client.read();
                String theDate = client.readStringUntil('\r');
                client.stop();
                return theDate;
              }
            }
          }
        }
      }
    }
  }
}

void initWifi() {
   Serial.print("Connecting to ");
   Serial.print(ssid);
   if (WiFi.SSID() != String(ssid)) {
      WiFi.begin(ssid, password);
   }

   while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
   }
  Serial.print("\nWiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
}
