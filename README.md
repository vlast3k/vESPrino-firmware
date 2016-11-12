# vESPrino Firmware

This is the firmware behind the vThing range of devices. It has a variety of sophisticated features targeted to decrease development and maintenance costs.

## Building
To reduce TCD efforts, the project is a real C++ project, with just a twist of Arduino in it. At some point of time the Arduino IDE and the limitation to only use INO files made the whole development quite complicated, so i decided to move it to Platform IO. So the steps to build it are as follows:

1. Install [PlatformIO IDE](http://platformio.org/).
2. Clone this repository and open in the IDE via File->Open Folder... (the root folder with the platformio.ini) file
3. Build it via PlaformIO->Build - this will download the necessary platform files and build the project
4. Upload it to your device. In platformio.ini, you can optionally specify BAUD Rate (e.g. upload_speed=256000), COM Port (e.g.upload_port=COM5) and [board type](http://docs.platformio.org/en/stable/platforms/espressif8266.html#boards) (e.g. board=nodemcuv2)

## Uploading latest built version

1. Download the fw from here: http://anker-bg.com/vlast3k/vesprino/latest.bin
2. Download the flashing tool for your OS from here: https://github.com/igrr/esptool-ck/releases/tag/0.4.9
3. and then execute something like (replace the comport)
```
esptool -vv -cd nodemcu -cb 256000 -cp "COM5" -cf latest.bin
```
