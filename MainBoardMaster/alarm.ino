#define TIME_ALARM 300000 // 5 min * 60 sec * 1000 msec 
#define TIME_REPEAT_ALARM 1000 // 1 sec
#define TEMP_ALARM 30 // 

void Alarm()
{
  if (mainTemp >= TEMP_ALARM && (millis() - time_alarm_ON) >= TIME_REPEAT_ALARM)
  {
    time_alarm_ON = millis();
    stateAlarm = 1;
    //    Blynk.email("WARNING!!!" ,"High temperature");
  }
  if (stateAlarm == 1 && (millis() - time_alarm_ON) >= TIME_ALARM)
  {
    stateAlarm = 0;
    digitalWrite(ZUMER, LOW);
  }
}

void alarmLoop()
{
  if (stateAlarm == 1 && (millis() - time_zumer) >= 100)
  {
    digitalWrite(ZUMER, !digitalRead(ZUMER));
    time_zumer = millis();
  }
}
