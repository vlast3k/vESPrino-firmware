void onTempRead() {
  float tmp = si7021->readTemp();
  float hum = si7021->readHumidity();
  Serial << "Humidity : " << hum << " %\t";
  Serial << "Temp : "     << tmp << " C";

  String s = String("sndiot ") + tmp;
  sndIOT(s.c_str());
}

