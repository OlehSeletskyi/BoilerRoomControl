#define PID_OPTIMIZED_I

#include <ServoSmooth.h>
#include "GyverPID.h"

ServoSmooth servo;
GyverPID pidServo(15.2, 1.2, 0.3);
GyverPID pidHeater(15.2, 0.82, 0.7);
int period = 1000;

bool heaterOff = false;
bool servoOff = false;

void pidSetup()
{
  pinMode(PID_HEATER, OUTPUT);
  
  pidServo.setDirection(NORMAL);
  pidServo.setMode(ON_ERROR);
  pidServo.setLimits(0, 110);
  pidServo.setpoint = 30;
  pidServo.setDt(period);

  pidHeater.setDirection(NORMAL);
  pidHeater.setMode(ON_ERROR);
  pidHeater.setLimits(0, 255);
  pidHeater.setpoint = 30;
  pidHeater.setDt(period);

  servo.attach(SERVO_PIN, 600, 2400);
  servo.smoothStart();
  servo.setSpeed(50);   // ограничить скорость
  servo.setAccel(0);    // установить ускорение (разгон и торможение)
  servo.setAutoDetach(false); // отключить автоотключение (detach) при достижении целевого угла (по умолчанию включено)
}

void pidLoop()
{
  static uint32_t tmr;
  servo.tick();
 
  if (millis() - tmr >= period)
  {
    tmr = millis();
    if (pidHeaterOn)
    {
      heaterPid();
      heaterOff = false;  
    }
    else if (!heaterOff)
    {
      analogWrite(PID_HEATER, 0);
      heaterOff = true;
    }

    if (pidServoOn)
    {
       servoPid();
       servoOff = false;  
    }
    else if (!servoOff)
    {
      servo.setTargetDeg(0);
      servoOff = true;
    }
    
    //        Serial.print(pidHeater.input); Serial.print(' ');
    //        Serial.print(pidHeater.output); Serial.print(' ');
    //        Serial.print(pidHeater.integral); Serial.print(' ');
    //        Serial.println(pidHeater.setpoint);
  }
  parsing();
}

void heaterPid()
{
  pidHeater.input = data.temperature[0];
  pidHeater.getResult();
  analogWrite(PID_HEATER, pidHeater.output);
  Serial.println("heaterPid -> " + String(pidHeater.output));
}

void servoPid()
{
  pidServo.input = data.temperature[0];
  pidServo.getResult();
  servo.setTargetDeg(pidServo.output);
  Serial.println("servoPid -> " + String(pidServo.output));
}

void updatePid()
{
   pidHeater.setpoint = data.setHeatTemp;
   pidServo.setpoint = data.setTemp_3;
   Serial.println("pidHeaterUpdate -> " + String(data.setHeatTemp));
   Serial.println("servoPidUpdate -> " + String(data.setTemp_3));
}

void parsing()
{
  if (Serial.available() > 1)
  {
    char incoming = Serial.read();
    float value = Serial.parseFloat();
    switch (incoming) {
      case 'p': pidHeater.Kp = value; break;
      case 'i': pidHeater.Ki = value; break;
      case 'd': pidHeater.Kd = value; break;
      case 's': pidHeater.setpoint = value; break;
    }
  }
}
