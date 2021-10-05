void sensorsSetup()
{
  sensors.begin();
  countSensors = sensors.getDeviceCount();
  sensorsUnique = new DeviceAddress[countSensors];
  Serial.print("Found sensors: ");
  Serial.println(countSensors);

  sensorsUnique = new DeviceAddress[countSensors];
 
  // определяем в каком режиме питания подключены сенсоры
  if (sensors.isParasitePowerMode()) {
    Serial.println("Mode power is Parasite");
  } else {
    Serial.println("Mode power is Normal");
  }
 
  // делаем запрос на получение адресов датчиков
  for (int i = 0; i < countSensors; i++) {
    sensors.getAddress(sensorsUnique[i], i);
  }
  // выводим полученные адреса
  for (int i = 0; i < countSensors; i++) {
    Serial.print("Device ");
    Serial.print(i);
    Serial.print(" Address: ");
    printAddress(sensorsUnique[i]);
    Serial.println();
  }
  Serial.println();
  // устанавливаем разрешение всех датчиков в 12 бит
  for (int i = 0; i < countSensors; i++) {
    sensors.setResolution(sensorsUnique[i], 11);
  } 
}

void readTemp()
{
  sensors.requestTemperatures();
  
  // считываем данные из регистра каждого датчика по очереди
  for (int i = 0; i < countSensors; i++) 
  {
    data.temperature[i] = sensors.getTempCByIndex(i);
  }
  
  // выводим температуру в Serial-порт по каждому датчику
//  for (int i = 0; i < countSensors; i++) 
//  {
//    Serial.print("Device ");
//    Serial.print(i);
//    Serial.print(" Temp C: ");
//    Serial.print(data.temperature[i]);
//    Serial.println();
//  }
    disp1.displayInt(data.temperature[0]);

    setBit(data.dataType, DataType::Sensors);
}

void printAddress(DeviceAddress deviceAddress){
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
