#include <NeoPixelBus.h>

//NeoPixelBus strip = NeoPixelBus(1, D4);
#define colorSaturation 255
RgbColor red =    RgbColor(255, 0, 0);
RgbColor pink =   RgbColor(255, 0, 128);
RgbColor lila =   RgbColor(255, 0, 255);
RgbColor violet = RgbColor(128, 0, 255);
RgbColor blue =   RgbColor(0, 0, 255);
RgbColor mblue =  RgbColor(0, 128, 255);
RgbColor cyan =   RgbColor(0, 255, 255);
RgbColor green =  RgbColor(0, 255, 0);
RgbColor yellow = RgbColor(255, 255, 0);
RgbColor orange = RgbColor(255, 100, 0);
RgbColor white = RgbColor(colorSaturation);
RgbColor black = RgbColor(0);


void initLight() {
  strip->Begin();
}

void processCommand(String color) {
  //Serial << "color: " << color;
  //color.trim();
  Serial << "Received Command: " << color << endl;
  RgbColor c;
  if (color == "red")         c = red;
  else if (color == "blue")   c = blue;
  else if (color == "green")  c = green;
  else if (color == "yellow") c = yellow;
  else if (color == "orange") c = orange;
  else if (color == "white")  c = white;
  else if (color == "black")  c = black;
  else if (color == "pink")   c = pink;
  else if (color == "lila")   c = lila;
  else if (color == "violet") c = violet;
  else if (color == "mblue")  c = mblue;
  else if (color == "cyan")   c = cyan;
  else {
    uint32_t data = strtol(color.c_str(), NULL, 0);
    //Serial << " receveid int: " << data << endl;
    c = RgbColor((data & 0xFF0000) >> 16, (data & 0x00FF00) >> 8, (data & 0x0000FF));    
  }

  strip->SetPixelColor(0, c);
  strip->Show();
}


