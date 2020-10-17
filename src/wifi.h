#include <Arduino.h>
#include <ESP8266HTTPUpdateServer.h>


ESP8266HTTPUpdateServer httpUpdater;



void jsonEncodeSettings(){
    StaticJsonDocument<512> doc;
    doc["lampState"] = settings.lampState;


    JsonObject jsonActivePattern = doc.createNestedObject("activePattern");
    jsonActivePattern["h"] = activePattern.hue;
    jsonActivePattern["s"] = activePattern.saturation;


    JsonArray jsonPresets = doc.createNestedArray("presets");
    for (uint8_t i=0; i<4; i++) {
      jsonPresets[i]["h"] = presets[i].hue;
      jsonPresets[i]["s"] = presets[i].saturation;
      jsonPresets[i]["v"] = presets[i].value;
    }

    serializeJson(doc, charBuffer);
}

void updateSettingsFromJSON(char json[]){
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, json);

    // Test if parsing succeeds.
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }




    if (doc.containsKey("lampState")) {
      settings.lampState = (uint8_t)doc["lampState"]; 
      digitalWrite(LED, !settings.lampState); 
    }  
    if (doc.containsKey("brightness")) {
      activePattern.value = (uint8_t)doc["brightness"]; 
    }  

    if (doc.containsKey("hue")) {
      activePattern.hue = (uint32_t)doc["hue"]; 
    }  
    if (doc.containsKey("saturation")) {
      activePattern.saturation = (uint8_t)doc["saturation"]; 
    }  



    if (doc.containsKey("selectPreset")) {
      Serial.println((uint8_t)doc["selectPreset"]);
      activePattern = presets[(uint8_t)doc["selectPreset"]]; 
    }  

    if (doc.containsKey("savePreset")) {
      Serial.println((uint8_t)doc["savePreset"]);
      presets[(uint8_t)doc["savePreset"]] = activePattern;
      jsonEncodeSettings();
      webSocket.broadcastTXT(charBuffer, strlen(charBuffer));
    }  


    changePattern();

}






void ajaxWifiStatus(void) {
  uint8_t n = WiFi.scanNetworks();

  StaticJsonDocument<512> doc;

  doc["wifiStatus"] = WiFi.status();
  doc["ssid"] = WiFi.SSID();
  doc["localIP"] = WiFi.localIP().toString();
  //doc["subnetMask"] = WiFi.subnetMask().toString();
  //doc["gatewayIP"] = WiFi.gatewayIP().toString();

  JsonArray wifiScan = doc.createNestedArray("wifiScan");

  for (uint8_t i=0; i<n; i++) {
    wifiScan[i]["ssid"] = WiFi.SSID(i);
    wifiScan[i]["rssi"] = WiFi.RSSI(i);
    wifiScan[i]["encr"] = WiFi.encryptionType(i);
  }

  String output = "";
  serializeJson(doc, output);
  Serial.println(output);

  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "application/json", output);
}

void changeWiFiSettings(){
  if (webServer.hasArg("ssid") && webServer.hasArg("pass")){  
    char ssid[32];
    char pass[32];
    webServer.arg("ssid").toCharArray(ssid, 32) ;
    webServer.arg("pass").toCharArray(pass, 32) ;

    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "application/json", "1");
    delay(1000);

    WiFi.begin(ssid, pass);
  }
}










void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  if (type == WStype_DISCONNECTED){ 
    Serial.printf("[%u] Disconnected!\n", num);
  } else if (type == WStype_CONNECTED) {              // if a new websocket connection is established
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
    jsonEncodeSettings();
    webSocket.broadcastTXT(charBuffer, strlen(charBuffer));
  } else if (type == WStype_TEXT){
    // for(int i = 0; i < length; i++) Serial.print((char) payload[i]);
    // Serial.println();

    updateSettingsFromJSON((char*)payload);
   }
}





void MQTTReconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT broker...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      mqttClient.publish("esp/test", "hello world");
      mqttClient.subscribe("esp/test/#"); //resubscribe

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}






void indexPage(void){
  webServer.send(200, "text/html", "test page");
}


void saveSettings(void){
  EEPROM.put(0, settings);
  EEPROM.commit();
}

void changeSettings(void){
  // if (webServer.hasArg("setTemp")){
  //   settings.ctrlMode = 1; // turn on the pid and maintain a fixed temperature value
  //   settings.temp = webServer.arg("setTemp").toFloat();
  //   myPID.SetMode(settings.ctrlMode); // turn on the pid 
  //   saveSettings();
  // }


  // if (webServer.hasArg("wifi_ssid") && webServer.hasArg("wifi_pass")){  
  //   webServer.arg("ssid").toCharArray(settings.ssid, 28) ;
  //   webServer.arg("pass").toCharArray(settings.pass, 28) ;
  //   WiFi.begin(settings.ssid, settings.pass);
  //   saveSettings();
  // }

  webServer.send(200, "text/html", "ok");
}



void serveFileFromSPIFFS(String path) {
  if(path.endsWith("/")) path += "index.htm";

  String contentType = "text/plain";
  if (path.endsWith(".htm")) contentType = "text/html";
  if (path.endsWith(".css")) contentType = "text/css";
  if (path.endsWith(".js")) contentType = "application/javascript";

  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    webServer.streamFile(file, contentType);
    file.close();
  } else {
    webServer.send(404, "text/plain", "404");
  }
}



void startWebServer(void){
  httpUpdater.setup(&webServer);

  SPIFFS.begin();


  //httpUpdater.setup(&webServer);
  webServer.on("/test", indexPage);
  webServer.on("/wifiStatus", ajaxWifiStatus);
  webServer.on("/changeWiFiSettings", changeWiFiSettings);

  //serveFileFromSPIFFS(webServer.uri()); 

  webServer.onNotFound([](){  // any other option
    serveFileFromSPIFFS(webServer.uri()); 
  });

  webServer.begin();
  //MDNS.begin("mqttLight");
}

