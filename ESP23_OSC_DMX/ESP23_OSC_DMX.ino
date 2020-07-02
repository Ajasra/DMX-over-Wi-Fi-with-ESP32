// - - - - -
// ESPDMX - A Arduino library for sending and receiving DMX using the builtin serial hardware port.
//
// Copyright (C) 2015  Rick <ricardogg95@gmail.com>
// This work is licensed under a GNU style license.
//
// Last change: Musti <https://github.com/IRNAS> (edited by Musti)
//
// Documentation and samples are available at https://github.com/Rickgg/ESP-Dmx
// Connect GPIO02 - TDX1 to MAX3485 or other driver chip to interface devices
// Pin is defined in library
// - - - - -

#include <ESPDMX.h>

DMXESPSerial dmx;

#include <ArduinoOSC.h>

OscWiFi osc;

const char* ssid = "";
const char* pwd = "";
const IPAddress ip(192, 168, 2, 198);
const IPAddress gateway(192, 168, 2, 1);
const IPAddress subnet(255, 255, 255, 0);

// for ArduinoOSC
const char* host = "192.168.2.196";
const int recv_port = 12000;
const int send_port = 13000;

int channels[512];
int prev_chan[512];

void setup() {
  Serial.begin(115200);

  delay(3000);

  Serial.println("starting...");

  //dmx.init();
  dmx.init(512,4);           // initialization for complete bus

  Serial.println("initialized...");
  delay(200);               // wait a while (not necessary)

  WiFi.begin(ssid, pwd);
    WiFi.config(ip, gateway, subnet);
    while (WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(500); }
    Serial.print("WiFi connected, IP = "); Serial.println(WiFi.localIP());
    osc.begin(recv_port);

    osc.subscribe("/chan/*", [](OscMessage& m)
    {
        //Serial.print(m.ip()); Serial.print(" ");
        int val = m.arg<int>(0);
        int channel_n = getValue(m.address(), '/', 2).toInt();
        //Serial.println(String(channel_n) + " " + String(val));
        channels[channel_n] = val;

    });

    for(int i = 0; i < 512; i++){
      channels[i] = 0;
      prev_chan[i] = 0;
    }
}

int dimmer = 0;
int color = 0;

void loop() {

  osc.parse(); // should be called

  //1   - pan
  //2   - microstep
  //3   - tilt
  //4   - microste tilt
  //5   - color
  //6   - gobo
  //7   - shutter
  //8   - dimmer
  //9   - sound
  //10  - auto
  //11  - reset

  for(int i = 0; i < 512; i++){
    if(channels[i] != prev_chan[i]){
      prev_chan[i] = channels[i];
      dmx.write(i+1, channels[i]);
    }
  }
  dmx.update();
 
  osc.send(host, send_port, "/code", int(random(200,300)) );

}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length();

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
