#include <Arduino.h>

struct pattern_t {
  uint8_t mode = 0;    //0=static

  //static
  uint16_t hue = 0;
  uint8_t saturation = 255;
  uint8_t value = 10;
};


pattern_t activePattern;
pattern_t presets[4];

uint32_t lastPatternUpdate = 0;





// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(24, PIXELS_PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.



void pixelsSetup(){

  pixels.begin();
  pixels.show(); 

  presets[0].hue = 0; //red
  presets[1].hue = 38000; //blue
  presets[2].hue = 57000; //pink
  presets[2].hue = 10000; //yellow
}


void changePattern(){
  pixels.clear();
  for(uint8_t i=0; i<24; i++) { 
    pixels.setPixelColor(i, pixels.ColorHSV(activePattern.hue, activePattern.saturation, activePattern.value*settings.lampState)); //hue, saturation, brightness
  }
  pixels.show();   

  lastPatternUpdate = millis();
}





void pixelLoop(){

  // send websocket mesage 1s after every change
  if (lastPatternUpdate !=0 && (millis()-lastPatternUpdate)>=1000) {
    lastPatternUpdate = 0;
    StaticJsonDocument<512> doc;
    JsonObject jsonActivePattern = doc.createNestedObject("activePattern");
    jsonActivePattern["h"] = activePattern.hue;
    jsonActivePattern["s"] = activePattern.saturation;
    serializeJson(doc, charBuffer);
    webSocket.broadcastTXT(charBuffer, strlen(charBuffer));
  }

}


//     for(uint8_t i=0; i<24; i++) { 
//       leds.setPixelColor(i, leds.ColorHSV(millis()*1.0, 255, 10)); //hue, saturation, brightness
// //    leds.setPixelColor(i, leds.Color(00, 0, 110));
//   }
//   leds.show();   