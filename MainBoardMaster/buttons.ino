const uint8_t boilerStartTime = 23;
const uint8_t boilerStopTime = 7;

void buttonsSetup()
{
  pinMode(BLED_1, OUTPUT);
  pinMode(BLED_2, OUTPUT);
  pinMode(BLED_3, OUTPUT);
  pinMode(BLED_4, OUTPUT);
  pinMode(REL_1, OUTPUT);
  pinMode(REL_2, OUTPUT);
  pinMode(REL_3, OUTPUT);
  pinMode(REL_4, OUTPUT);

  digitalWrite(BLED_1, HIGH);
  digitalWrite(REL_1, HIGH);
  digitalWrite(BLED_2, HIGH);
  digitalWrite(REL_2, HIGH);
  digitalWrite(BLED_3, HIGH);
  digitalWrite(REL_3, HIGH);
  digitalWrite(BLED_4, HIGH);
  digitalWrite(REL_4, HIGH);

  butt_1.setTimeout(300);
  butt_2.setTimeout(300);
  butt_3.setTimeout(300);
  butt_4.setTimeout(1000);

  butt_1.setType(LOW_PULL);
  butt_2.setType(LOW_PULL);
  butt_3.setType(LOW_PULL);
  butt_4.setType(LOW_PULL);

  butt_1.setDirection(NORM_OPEN);
  butt_2.setDirection(NORM_OPEN);
  butt_3.setDirection(NORM_OPEN);
  butt_4.setDirection(NORM_OPEN);

  butt_1.setTickMode(AUTO);
  butt_2.setTickMode(AUTO);
  butt_3.setTickMode(AUTO);
  butt_4.setTickMode(AUTO);
}

void buttonsLoop()
{
  if (butt_1.isClick())
  {
    data.stateRel_1 = digitalRead(BLED_1);
    digitalWrite(BLED_1, !data.stateRel_1);
     digitalWrite(REL_1, !data.stateRel_1);
    
    setBit(data.dataType, DataType::But1);
    
    EEPROM.put(EepromAddrr::AddrrBut1, data.stateRel_1);  
    
    Serial.println("Click 1 -> " + String(data.stateRel_1));
  }

  if (butt_2.isClick())
  {
    data.stateRel_2 = digitalRead(BLED_2);
    digitalWrite(BLED_2, !data.stateRel_2);
     digitalWrite(REL_2, !data.stateRel_2);

    setBit(data.dataType, DataType::But2);

    EEPROM.put(EepromAddrr::AddrrBut2, data.stateRel_2);

    Serial.println("Click 2");
  }

  if (butt_3.isClick())
  {
    data.stateRel_3 = digitalRead(BLED_3);
    digitalWrite(BLED_3, !data.stateRel_3);
     digitalWrite(REL_3, !data.stateRel_3);
    pidServoOn = data.stateRel_3;
    setBit(data.dataType, DataType::But3);

    EEPROM.put(EepromAddrr::AddrrBut3, data.stateRel_3);

    Serial.println("Click 3");
  }

  if (butt_4.isClick())
  {
    data.stateRel_4 = digitalRead(BLED_4);

    digitalWrite(BLED_4, !data.stateRel_4);

    setBit(data.dataType, DataType::But4);

    EEPROM.put(EepromAddrr::AddrrBut4, data.stateRel_4);

    Serial.println("Click 4 " + String(data.stateRel_4));
  }
  
  if (((data.hourSync >= boilerStopTime) && (data.hourSync < boilerStartTime) || (data.stateRel_4 == false)) &&  pidHeaterOn)
  {
    Serial.println("Off");
    digitalWrite(REL_4, HIGH);
    pidHeaterOn = false;
  }
  else if (data.stateRel_4 && (!pidHeaterOn))
  {
    Serial.println("On");
    digitalWrite(REL_4, LOW);
    pidHeaterOn = true;
  }
}
