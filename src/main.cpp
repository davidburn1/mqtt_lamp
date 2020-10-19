#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include "LittleFS.h"
#include <Adafruit_NeoPixel.h>

#include <ArduinoJson.h>

#define LED LED_BUILTIN
#define PIXELS_PIN D5


struct settings_t {
  char mqttServer[32] = "test.mosquitto.org";
  uint16_t mqttPort = 1883;
  char mqttUser[32] = "";
  char mqttPassword[32] = "";

  uint8_t lampState = 0;
  //uint32_t hue = 0;
};

char charBuffer[1024];

settings_t settings; 

uint32_t lastMsg = 0;
uint32_t timer10s = 0;
uint32_t timer10m = 0;





WiFiClient client;
ESP8266WebServer webServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);
PubSubClient mqttClient(client);



#include "pixels.h"
#include "wifi.h"

void mqttCallback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (uint i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  if ((char)payload[0] == '1') {
    digitalWrite(LED, 1); //
  } 
  if ((char)payload[0] == '0') {
    digitalWrite(LED, 0); 
  }
 
  Serial.println();
  Serial.println("-----------------------");
}









void setup() {
  pinMode(LED, OUTPUT);
  pinMode(PIXELS_PIN, OUTPUT);
  digitalWrite(LED, HIGH); //off

  Serial.begin(115200);
  Serial.println("");

  pixelsSetup();

  //EEPROM.begin(512); //4096 should be the maximum allowed to allocate
  //EEPROM.get(0,settings); //

  WiFi.setOutputPower(0.0); // 0 dBm
  WiFi.mode(WIFI_AP_STA);     // access point and station at the same time
  Serial.print("Starting soft AP: \t");
  WiFi.softAP("lamp");
  Serial.println(WiFi.softAPIP());

  startWebServer();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  Serial.print("Connecting to WiFi..");
  WiFi.begin();

  mqttClient.setServer(settings.mqttServer, settings.mqttPort);
  mqttClient.setCallback(mqttCallback);  
}





void loop() {
  delay(1);
  //if (!client.connected()) {
  //  MQTTReconnect();
  //}

  //mqttClient.loop();


  webSocket.loop();
  webServer.handleClient();

  pixelLoop();

  // if (millis() - timer10s > 10000) {
  //   timer10s += 10000;
  //   //checkForFirmwareUpdates();
  // }

  if (millis() - timer10m > 600000) {
    timer10m += 600000;

    //shut down soft AP
    WiFi.mode(WIFI_STA);
  }
  


// button press

//     val = digitalRead(inPin);  // read input value
//   if (val == HIGH) {         // check if the input is HIGH (button released)
//     digitalWrite(ledPin, LOW);  // turn LED OFF
//   } else {
//     digitalWrite(ledPin, HIGH);  // turn LED ON
//   }
// }




      // char c[] = {(char)Serial.read()};
      // webSocket.broadcastTXT(c, sizeof(c));


  // if (millis() - lastMsg > 2000) {
  //   lastMsg += 2000;
   
  //  char msg[50];
  //   snprintf (msg, 50, "hello world #%ld", millis());
  //   Serial.print("Publish message: ");
  //   Serial.println(msg);
  //   mqttClient.publish("esp/test", msg);
  // }

}

