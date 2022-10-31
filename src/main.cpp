#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <HTTPClient.h>
#inlcude "secrets.h"


String data = "";
String start = "1b1b1b1b01010101";
String end = "1b1b1b1b1a";
String verbrauchMarker = "77070100010800ff64";
String rxs; 
StaticJsonDocument<500> doc;

void setup(){
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_BUILTIN, LOW);
}







void loop() {

  getDevice();
  readSmartmeter();
  postData();
  delay(5000); //wait 5s later change to execution in setuploop with sleep and wake up ...

}

void getDevice(){
  uint64_t chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("***ESP8266 Chip ID = %04X%08X\n",(uint16_t)(chipid>>32),(uint32_t)chipid);//print High 2 bytes
  char buffer[200];
  sprintf(buffer, "%04X%08X",(uint16_t)(chipid>>32),(uint32_t)chipid);

  doc["device"]["IP"] = WiFi.localIP().toString();
  doc["device"]["RSSI"] = String(Wifi.RSSI());
  doc["device"]["chipid"] = buffer;
}

int readSmartmeter()
  {
    /*do stuff,to read the value*/
    if (Serial.available()>0){
    int rx = Serial.read();
    if (rx < 16){
      rxs = "0" + String(rx, HEX);
    }
    else {
      rxs = String(rx, HEX);
    }

    data += rxs;
    
    int pos = data.indexOf(end);
    
    if (pos != -1){
      for (int i=0; i<3; i++){
        int rx = Serial.read();
        if (rx < 16){
          rxs = "0" + String(rx, HEX);
        }
        else {
          rxs = String(rx, HEX);
        }

        data += rxs;
        
      }
      pos = data.indexOf(verbrauchMarker);
      if (pos != -1){
        pos += 36; // evtl /2        
        String verbrauch = data.substring(pos, pos + 16); //
        Serial.print("Verbrauch: ");
        Serial.println(verbrauch);
      }
    Serial.println(data);
    data = "";
    }  
  }

  // write into json
  doc["sensors"]["smartmeter"] = data;
  return data;
  
  }

void postData()
  // push data to mongodb
  {
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      http.begin(serverName);
      http.addHeader("Content-Type", "application/json");

      String json;
      serializeJson(doc, json);

      Serial.println(json);
      int httpResponseCode = http.POST(json);
      Serial.println(httpResponseCode);
      }
  }
