#include "pin_def.h"

#define PID_INTEGER

#define INIT_ADDR 1023
#define INIT_KEY 50

#define ESP_ADDRR 1
#define MEGA_ADDRR 2

#include "GyverEncoder.h"
#include "GyverTM1637.h"
#include "GyverPID.h"
//#include <microDS18B20.h>
#include <BlynkSimpleEthernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//#include <LiquidCrystal_I2C.h>
//#include <LiquidMenu.h>
#include "GyverButton.h"
#include "GBUS.h"
#include <GyverWDT.h>
#include <EEPROM.h>
#include <WidgetRTC.h>

GBUS bus(&Serial2, MEGA_ADDRR, 30);

BlynkTimer timer;
GyverTM1637 disp1(LCD1_CLK, LCD1_DIO);
GyverTM1637 disp2(LCD2_CLK, LCD2_DIO);
Encoder enc1(ENC_CLK, ENC_DT, ENC_SW);

OneWire oneWire(DS18B20);
DallasTemperature sensors(&oneWire);
DeviceAddress *sensorsUnique;
int countSensors;

GButton butt_1(BUT_1);
GButton butt_2(BUT_2);
GButton butt_3(BUT_3);
GButton butt_4(BUT_4);


bool stateAlarm = 0;
unsigned long long time_alarm_ON = 0;
unsigned long long time_zumer = 0;

byte heaterPower = 0;

bool pidHeaterOn = false;
bool pidServoOn = false;

float mainTemp;

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

enum EepromAddrr {
  AddrrBut1 = 1,
  AddrrBut2,
  AddrrBut3,
  AddrrBut4,
  AddrrSetTemp3,
  AddrrSetHeatTemp,
};

struct DATA_STRUCTURE {
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

DATA_STRUCTURE rxData;
DATA_STRUCTURE data;

void isr() {
  enc1.tick(); 
}

void sendDataToCommBoard() {
  if (data.dataType > 0) 
  {
    bus.sendData(ESP_ADDRR, data);
  }
}

//LiquidLine welcome_line1(1, 0, "LiquidMenu ", LIQUIDMENU_VERSION);
//LiquidLine welcome_line2(1, 1, "Hello Menu I2C");
//LiquidScreen welcome_screen(welcome_line1, welcome_line2);
//LiquidMenu menu(lcd);

ISR(WATCHDOG) {
  Serial.println("RESET WIFI MODULE");
  pinMode(WATCHDOG_PIN_ESP, OUTPUT);
  //  digitalWrite(WATCHDOG_PIN_ESP, LOW);
  Watchdog.enable(INTERRUPT_RESET_MODE, WDT_PRESCALER_1024);
  Watchdog.reset();
  pinMode(WATCHDOG_PIN_ESP, INPUT);
}

void setup() {
  Serial.begin(9600);
  Serial2.begin(15000);

  pinMode(ZUMER, OUTPUT);

  /**** DISPLAY *****/
  disp1.clear();
  disp1.brightness(7); 
  disp2.clear();
  disp2.brightness(7); 

  /**** BUTTONS *****/
  buttonsSetup();

  /**** TEMP SENSOSRS *****/
  sensorsSetup();

  /**** PID *****/
  pidSetup();

  /**** ENCODER *****/
  enc1.setDirection(REVERSE);
  enc1.setType(TYPE2);
  attachInterrupt(0, isr, CHANGE);

  /**** Watchdog *********/
  pinMode(WATCHDOG_PIN_ESP, INPUT);
  Watchdog.enable(INTERRUPT_RESET_MODE, WDT_PRESCALER_1024);

  /****** EEPROM ********/
  if (EEPROM.read(INIT_ADDR) != INIT_KEY) 
  { 
    EEPROM.write(INIT_ADDR, INIT_KEY);   

    EEPROM.put(EepromAddrr::AddrrBut1, 0);  
    EEPROM.put(EepromAddrr::AddrrBut2, 0);  
    EEPROM.put(EepromAddrr::AddrrBut3, 0);
    EEPROM.put(EepromAddrr::AddrrBut4, 0);

    EEPROM.put(EepromAddrr::AddrrSetTemp3, 35);
    EEPROM.put(EepromAddrr::AddrrSetHeatTemp, 40);
  }
  readEeprom();

  /****** TIMERS ********/
  timer.setInterval(5000L, readTemp);
  timer.setInterval(5000L, Alarm);
  timer.setInterval(1000L, sendDataToCommBoard);
}

void loop() {
  timer.run();
  Watchdog.reset();
  buttonsLoop();
  bus.tick();
  enc1.tick();

  pidLoop();
  alarmLoop();

  if (bus.gotData()) 
  {
    Serial.println("gotData");
    bus.readData(rxData);
    Serial.println("TYPE: " + String(rxData.dataType));

    if (readBit(rxData.dataType, DataType::Ack))
    {
      Serial.println("Ack");
      clearBit(rxData.dataType, DataType::Ack);
      data.dataType ^= rxData.dataType;
    }
    else 
    {
       Serial.println("syncData");
       syncData();
       setBit(rxData.dataType, DataType::Ack);
       bus.sendData(ESP_ADDRR, rxData);
       clearBit(rxData.dataType, DataType::Ack);
    }
  }

  if (enc1.isLeft()) {
    if (heaterPower < 100) {
      heaterPower++;
    }
  }
  if (enc1.isRight()) {
    if (heaterPower > 0) {
      heaterPower--;
    }
  }
  //    disp2.displayInt(heaterPower);

  if (enc1.isClick()) {
    if (stateAlarm == 1) {
      stateAlarm = 0;
      digitalWrite(ZUMER, LOW);
    }
  }
}

void readEeprom()
{
  EEPROM.get(EepromAddrr::AddrrBut1, data.stateRel_1);
  digitalWrite(BLED_1, !data.stateRel_1);
    digitalWrite(REL_1, !data.stateRel_1);

  EEPROM.get(EepromAddrr::AddrrBut2, data.stateRel_2);
  digitalWrite(BLED_2, !data.stateRel_2);
   digitalWrite(REL_2, !data.stateRel_2);

  EEPROM.get(EepromAddrr::AddrrBut3, data.stateRel_3);
  digitalWrite(BLED_3, !data.stateRel_3); 
   digitalWrite(REL_3, !data.stateRel_3);
  pidServoOn = data.stateRel_3;


  EEPROM.get(EepromAddrr::AddrrBut4, data.stateRel_4);
  digitalWrite(BLED_4, !data.stateRel_4);

  
  EEPROM.get(EepromAddrr::AddrrSetTemp3, data.setTemp_3);
  
  EEPROM.get(EepromAddrr::AddrrSetHeatTemp, data.setHeatTemp);

  updatePid();
}

void setBit(uint16_t &n, int k) {
  n = (n | (1 << (k - 1)));
}

void clearBit(uint16_t &n, int k) {
  n = (n & (~(1 << (k - 1))));
}

bool readBit(uint16_t b, int bitNumber) {
  return (b & (1 << bitNumber - 1)) != 0;
}

void syncData() 
{
  if (readBit(rxData.dataType, DataType::Request)) 
  {
    setBit(data.dataType, DataType::But1);
    setBit(data.dataType, DataType::But2);
    setBit(data.dataType, DataType::But3);
    setBit(data.dataType, DataType::But4);

    setBit(data.dataType, DataType::SetTemp3);
    setBit(data.dataType, DataType::SetHeatTemp);
  }

  if (readBit(rxData.dataType, DataType::But1)) {
    data.stateRel_1 = rxData.stateRel_1;
    digitalWrite(BLED_1, !data.stateRel_1);
    EEPROM.put(EepromAddrr::AddrrBut1, data.stateRel_1);
     digitalWrite(REL_1, !data.stateRel_1);

      Serial.println("Sync1");
  }

  if (readBit(rxData.dataType, DataType::But2)) {
    data.stateRel_2 = rxData.stateRel_2;
    digitalWrite(BLED_2, !data.stateRel_2);
    EEPROM.put(EepromAddrr::AddrrBut2, data.stateRel_2);
     digitalWrite(REL_2, !data.stateRel_2);

     Serial.println("Sync2");
  }

  if (readBit(rxData.dataType, DataType::But3)) {
    data.stateRel_3 = rxData.stateRel_3;
    digitalWrite(BLED_3, !data.stateRel_3);
    EEPROM.put(EepromAddrr::AddrrBut3, data.stateRel_3);
     digitalWrite(REL_3, !data.stateRel_3);

     Serial.println("Sync3");
    pidServoOn = data.stateRel_3;
  }

  if (readBit(rxData.dataType, DataType::But4)) {
    data.stateRel_4 = rxData.stateRel_4;
    digitalWrite(BLED_4, !data.stateRel_4);
    EEPROM.put(EepromAddrr::AddrrBut4, data.stateRel_4);

    //  Serial.println("Sync4");
  }

  if (readBit(rxData.dataType, DataType::SetTemp3)) {
    data.setTemp_3 = rxData.setTemp_3;
    EEPROM.put(EepromAddrr::AddrrSetTemp3, data.setTemp_3);
    updatePid();
  }

  if (readBit(rxData.dataType, DataType::SetHeatTemp)) {
    data.setHeatTemp = rxData.setHeatTemp;
    EEPROM.put(EepromAddrr::AddrrSetHeatTemp, data.setHeatTemp);
    updatePid();
  }

  if (readBit(rxData.dataType, DataType::Time)) {
    data.hourSync = rxData.hourSync;
    data.minSync = rxData.minSync;
    setTime(data.hourSync, data.minSync, 0, 0, 0, 0);
    disp2.displayClockScroll(data.hourSync, data.minSync, 35);

    String currentTime = String(hour()) + ":" + minute() + ":" + second();
    Serial.print("Current time: ");
    Serial.println(currentTime);
  }
}
