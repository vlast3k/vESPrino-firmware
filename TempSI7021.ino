void onTempRead() {
  float tmp = si7021->readTemp();
  float hum = si7021->readHumidity();
  SERIAL << "Humidity : " << hum << " %\t";
  SERIAL << "Temp : "     << tmp << " C" << endl;
  
  String s = String("sndiot ") + tmp;
  sndIOT(s.c_str());
}

void dumpTemp() {
  SERIAL << "Temp : "     << si7021->readTemp() << " C" << endl;  
}

