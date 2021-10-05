/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG
//#define BLYNK_HEARTBEAT      10

#define ESP_ADDRR 1
#define MEGA_ADDRR 2

#define BUS_BUFFER 30

#include <SoftwareSerial.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "GBUS.h"
#include <RCSwitch.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

// #include "passwords.h"

RCSwitch mySwitch = RCSwitch();
SoftwareSerial mySerial(5, 4); // RX, TX
GBUS bus(&mySerial, ESP_ADDRR, BUS_BUFFER);
WidgetTerminal terminal(V0);
BlynkTimer timer;
WidgetRTC rtc;
 
const int KNOWN_SSID_COUNT = sizeof(KNOWN_SSID) / sizeof(KNOWN_SSID[0]); 

enum DataType {
  Ack = 1,
  Request,
  But1,
  But2,

  But3,
  But4,
  SetTemp3,
  SetHeatTemp,

  Sensors,
  WdPing,
  Time
};

struct DATA_STRUCTURE
{
  uint16_t dataType;

  uint8_t temperature[10];
  uint8_t setTemp_3;
  uint8_t setHeatTemp;
  uint8_t hourSync;
  uint8_t minSync;
  
  bool stateRel_1;
  bool stateRel_2;
  bool stateRel_3;
  bool stateRel_4;

  bool alarmState;

};

time_t tm = 0;

DATA_STRUCTURE rxData;
DATA_STRUCTURE data;

BLYNK_CONNECTED()
{
  // Blynk.syncVirtual(V1, V2, V3, V4, V7, V8);
  rtc.begin();
}

BLYNK_WRITE(V1)
{
  data.stateRel_1 = param.asInt();
  setBit(data.dataType, DataType::But1);
  Serial.println("Virtual V1" + String(data.stateRel_1));
}

BLYNK_WRITE(V2)
{
  data.stateRel_2 = param.asInt();
  setBit(data.dataType, DataType::But2);
  Serial.println("Virtual V2" + String(data.stateRel_2));
}

BLYNK_WRITE(V3)
{
  data.stateRel_3 = param.asInt();
  setBit(data.dataType, DataType::But3);
  Serial.println("Virtual V3" + String(data.stateRel_3));
}

BLYNK_WRITE(V4)
{
  data.stateRel_4 = param.asInt();
  setBit(data.dataType, DataType::But4);
  Serial.println("Virtual V4" + String(data.stateRel_4));
}

BLYNK_WRITE(V7)
{
  data.setTemp_3 = param.asInt();
  setBit(data.dataType, DataType::SetTemp3);
  Serial.println("Virtual V7" + String(data.setTemp_3));
}

BLYNK_WRITE(V8)
{
  data.setHeatTemp = param.asInt();
  setBit(data.dataType, DataType::SetHeatTemp);
  Serial.println("Virtual V8" + String(data.setHeatTemp));
}

void sendDataToMainBoard()
{
  Serial.println("DATA TYPE -> " + String(data.dataType));
  if (data.dataType > 0)
  {
    bus.sendData(MEGA_ADDRR, data);  
  }
}

//void sendPingToMainBoard()
//{
//  if (!Blynk.connected())
//  {
//    if (amountOfRetries < 30)
//    {
//      txData.dataType = WD_PING;
//      bus.sendData(MEGA_ADDRR, txData);
//
//      amountOfRetries++;
//      Serial.println("Not connected to Blynk server");
//      Blynk.connect();  // try to connect to server with default timeout
//    }
//  }
//  else
//  {
//    Serial.println("Connected");
//    amountOfRetries = 0;
//    txData.dataType = WD_PING;
//    bus.sendData(MEGA_ADDRR, txData);
//  }
//}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void syncTime()
{
  data.hourSync = hour();
  data.minSync = minute();
  setBit(data.dataType, DataType::Time);
  Serial.println("Sync Time");
}

void setup()
{
  boolean wifiFound = false;
  int i, n;

  Serial.begin(9600);
  mySerial.begin(15000);

  // ----------------------------------------------------------------
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  // --
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("Setup done");

  // ----------------------------------------------------------------
  // WiFi.scanNetworks will return the number of networks found
  // ----------------------------------------------------------------
  Serial.println(F("Scan start"));
  int nbVisibleNetworks = WiFi.scanNetworks();
  Serial.println(F("Scan Completed"));
  if (nbVisibleNetworks == 0)
  {
    Serial.println(F("No networks found. Reset to try again"));
    while (true); // no need to go further, hang in there, will auto launch the Soft WDT reset
  }

  // ----------------------------------------------------------------
  // if you arrive here at least some networks are visible
  // ----------------------------------------------------------------
  Serial.print(nbVisibleNetworks);
  Serial.println(" network(s) found");

  // ----------------------------------------------------------------
  // check if we recognize one by comparing the visible networks
  // one by one with our list of known networks
  // ----------------------------------------------------------------

  for (i = 0; i < nbVisibleNetworks; ++i)
  {
    Serial.println(WiFi.SSID(i)); // Print current SSID
    for (n = 0; n < KNOWN_SSID_COUNT; n++)  // walk through the list of known SSID and check for a match
    {
      if (strcmp(KNOWN_SSID[n], WiFi.SSID(i).c_str()))
      {
        Serial.print(F("\tNot matching "));
        Serial.println(KNOWN_SSID[n]);
      }
      else  // we got a match
      {
        wifiFound = true;
        break; // n is the network index we found
      }
    } // end for each known wifi SSID
    if (wifiFound) break; // break from the "for each visible network" loop
  }

  if (!wifiFound)
  {
    Serial.println(F("No Known Network identified. Reset to try again"));
    while (true);
  }

  const char* ssid = (KNOWN_SSID[n]);
  const char* pass = (KNOWN_PASSWORD[n]);

  ArduinoOTA.setPort(8266);
  ArduinoOTA.begin();

  // Setup a function to be called every second
  timer.setInterval(1000L, sendDataToMainBoard);
  //  timer.setInterval(2000L, sendPingToMainBoard);
  //  timer.setInterval(20000L, printWifiStatus);
  Blynk.begin(auth, ssid, pass);

  setBit(data.dataType, DataType::Request);

  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
  timer.setInterval(10000L, syncTime);
}

void loop()
{
  timer.run();
  Blynk.run();
  bus.tick();

  //  ArduinoOTA.handle();

  if (bus.gotData())
  {
    // Serial.println("gotData");
    bus.readData(rxData);
    // Serial.println("TYPE: " + String(rxData.dataType));

    if (readBit(rxData.dataType, DataType::Ack))
    {
      clearBit(rxData.dataType, DataType::Ack);
      Serial.println("FIRST" + String (data.dataType));
      Serial.println("SECOND" + String (rxData.dataType));
      
      data.dataType ^= rxData.dataType;
      Serial.println("RESULT" + String (data.dataType));
    }
    else 
    {
       Serial.println("syncData");
       syncData();
       setBit(rxData.dataType, DataType::Ack);
       bus.sendData(MEGA_ADDRR, rxData);
       clearBit(rxData.dataType, DataType::Ack);
    }
  }
}

void setBit(uint16_t &n, int k)
{
  n = (n | (1 << (k - 1)));
}

void clearBit(uint16_t &n, int k)
{
  n = (n & (~(1 << (k - 1))));
}

bool readBit(uint16_t b, int bitNumber) {
  return (b & (1 << bitNumber - 1)) != 0;
}

void syncData() 
{
  if (readBit(rxData.dataType, DataType::But1)) 
  {
    data.stateRel_1 = rxData.stateRel_1;
    Blynk.virtualWrite(V1, data.stateRel_1);
    Serial.println("Sync1");
  }

  if (readBit(rxData.dataType, DataType::But2)) 
  {
    data.stateRel_2 = rxData.stateRel_2;
    Blynk.virtualWrite(V2, data.stateRel_2);
    Serial.println("Sync2");
  }

  if (readBit(rxData.dataType, DataType::But3)) 
  {
    data.stateRel_3 = rxData.stateRel_3;
    Blynk.virtualWrite(V3, data.stateRel_3);
    Serial.println("Sync3");
  }

  if (readBit(rxData.dataType, DataType::But4)) 
  {
    data.stateRel_4 = rxData.stateRel_4;
    Blynk.virtualWrite(V4, data.stateRel_4);
    Serial.println("Sync4");
  }

  if (readBit(rxData.dataType, DataType::SetTemp3)) 
  {
    data.setTemp_3 = rxData.setTemp_3;
    Serial.println("setTemp_3");
  }

  if (readBit(rxData.dataType, DataType::SetHeatTemp)) 
  {
    data.setHeatTemp = rxData.setHeatTemp;
    Serial.println("setHeatTemp");
  } 

  if (readBit(rxData.dataType, DataType::Sensors)) 
  {
    // data.temperature[0] = rxData.temperature[0];
    Blynk.virtualWrite(V20, rxData.temperature[0]);
    Serial.println("Sensors");
  }  
}
