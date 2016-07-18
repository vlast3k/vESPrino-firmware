#include <Arduino.h>

#ifndef VTHING_H801_LED
#include <NeoPixelBus.h>
#include <Streaming.h>
#define SERIAL Serial
extern NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod>  *strip;
//NeoPixelBus strip = NeoPixelBus(1, D4);
#define colorSaturation 255



void initLight() {
  strip->Begin();
}



void processCommand(String color) {
  //SERIAL << "color: " << color;
  //color.trim();
  SERIAL << F("Received Command: ") << color << endl;
  RgbColor c;
//  if (color == "red")         c = red;
//  else if (color == "blue")   c = blue;
//  else if (color == "green")  c = green;
//  else if (color == "yellow") c = yellow;
//  else if (color == "orange") c = orange;
//  else if (color == "white")  c = white;
//  else if (color == "black")  c = black;
//  else if (color == "pink")   c = pink;
//  else if (color == "lila")   c = lila;
//  else if (color == "violet") c = violet;
//  else if (color == "mblue")  c = mblue;
//  else if (color == "cyan")   c = cyan;
  //else {
    uint32_t data = strtol(color.c_str(), NULL, 0);
    //SERIAL << " receveid int: " << data << endl;
    c = RgbColor((data & 0xFF0000) >> 16, (data & 0x00FF00) >> 8, (data & 0x0000FF));
 // }

  strip->SetPixelColor(0, c);
  strip->Show();
}

#endif
