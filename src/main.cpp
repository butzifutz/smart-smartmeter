#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "secrets.h"
#include <WiFiClient.h>

String data = "";
String start = "1b1b1b1b01010101";
String end = "1b1b1b1b1a";
String verbrauchMarker = "77070100010800ff64";
String rxs; 
StaticJsonDocument<500> doc;
long lastValue = 5900 ; //because I'm to lazy to use the checksum
int LED_BUILTIN = 2;
const char* Endpoint = "https://eu-central-1.aws.data.mongodb-api.com/app/smartmeter_incoming-nzfqu/endpoint/incoming_webhook?secret=0this1one2is3super4secret5";

void setup(){
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(9600);
  delay(500);
  Serial.println("Connecting to ");
  Serial.println(" ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("(.)");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_BUILTIN, LOW);
}




void getDevice(){
  //byte mac[6];
  //WiFi.macAddress(mac);//The chip ID is essentially its MAC address(length: 6 bytes).
  //Serial.printf("***ESP8266 Chip ID = %04X%08X\n", chipid);//print High 2 bytes

  doc["device"]["IP"] = WiFi.localIP().toString();
  doc["device"]["RSSI"] = String(WiFi.RSSI());
  doc["device"]["Type"] = "ESP32";
  doc["device"]["chipid"] = String(WiFi.macAddress());
}

int readSmartmeter()
  { 
    int counter = 0;
    data = "";
    Serial.println("start reading data");
    while (1){      
      /*do stuff,to read the value*/
      if (Serial.available()>0){
        //1. read a byte  
        int rx = Serial.read();             
        //2. add leading zero to value <16
        if (rx < 16){ 
          rxs = "0" + String(rx, HEX);
        }
        else {
          rxs = String(rx, HEX);
        }
        
        //3. append to data
        data += rxs;
        //4. search for position of 'end' in bytestring
        int pos = data.indexOf(end);
        //5. if found continue
        if (pos != -1){
          Serial.println("found end");
          for (int i=0; i<3; i++){ //read 3 more bytes
            int rx = Serial.read();
            if (rx < 16){
              rxs = "0" + String(rx, HEX);
            }
            else {
              rxs = String(rx, HEX);
            }
            data += rxs;            
          }
          //6. search for position of verbrauch
          pos = data.indexOf(verbrauchMarker);
          //7. if found continue
          if (pos != -1){
            pos += 36; // evtl /2        
            String verbrauch = data.substring(pos, pos + 16); //
            Serial.print("Verbrauch: ");
            int str_len = verbrauch.length()+1;
            char input[str_len];
            verbrauch.toCharArray(input, str_len);
            int x;
            char *endptr;

            x = strtol(input, &endptr, 16);

            Serial.println(x);
            if (x / lastValue > 1.01 ){ //check is the new and old value differ by more than 1%
              return 0;
              break;
            }
            else{
              lastValue = x; 
              String unit = "kWh";
              doc["sensors"]["smartmeter"]["value"] = x;
              doc["sensors"]["smartmeter"]["unit"] = unit;
              return 1;
              break;
            }
          }
          Serial.println(data);
          data = "";
        }  
        if (counter>300){
          return 0;
          break;}
    }

    // write into json
    
    
    }
    
  
  }

void postData()
  // push data to mongodb
  {
    if(WiFi.status()== WL_CONNECTED){

      HTTPClient http;

      if (http.begin(Endpoint)) {  // HTTPS
        /* Headers Required for Data API*/
        http.addHeader("Content-Type", "application/json");
        //https.addHeader("api-key", AtlasAPIKey);
      
        String json;
        serializeJson(doc, json);

        Serial.println(json);

        int httpResponseCode = http.POST(json);
        Serial.println(httpResponseCode);
        if (httpResponseCode < 0){
          Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
        }
        }
      
      }
      
  }

void loop() {
  Serial.println("reading device info:");
  getDevice();

  Serial.println("reading Smartmeter:");
  int a = readSmartmeter();
  if (a==1){
    Serial.println("posting data:");
    postData();
    Serial.println("waiting:");
    delay(600000); //wait 5s later change to execution in setuploop with sleep and wake up ...

  }
  else{
    Serial.println("no new data, not posting");
    delay(10000);
  }
  
  
}