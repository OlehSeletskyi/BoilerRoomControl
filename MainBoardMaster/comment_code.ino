//void turnTap(unsigned int rotation)
//{
//  //  Serial.println(rotation);
//  unsigned int median = readMedian(Hall_2, 40);
//  //  Serial.print("median: ");
//  //    Serial.println(median);
//  unsigned int angle_of_rotation = map(median, 205, 860, 90, 0);
//  //      Serial.print("median: ");
////  Serial.println(median);
//  //      Serial.print("angle_of_rotation: ");
//  //        Serial.println(angle_of_rotation);
//
//  if (angle_of_rotation < rotation)
//  {
//    analogWrite(Mot2_R, 255);
//    delay(60);
//  }
//  else
//  {
//    digitalWrite(Mot2_R, 0);
//  }
//
//  if (angle_of_rotation > rotation)
//  {
//    analogWrite(Mot2_L, 255);
//    delay(60);
//  }
//  else
//  {
//    digitalWrite(Mot2_L, 0);
//  }
//
//  //  disp2.float_dot(angle_of_rotation, 2);
//  send_data.angle_of_rotation = angle_of_rotation;
//}


//int readMedian (int pin, int samples)
//{
//  // массив для хранения данных
//  int raw[samples];
//  // считываем вход и помещаем величину в ячейки массива
//  for (int i = 0; i < samples; i++) {
//    raw[i] = analogRead(pin);
//  }
//  // сортируем массив по возрастанию значений в ячейках
//  int temp = 0; // временная переменная
//
//  for (int i = 0; i < samples; i++) {
//    for (int j = 0; j < samples - 1; j++) {
//      if (raw[j] > raw[j + 1]) {
//        temp = raw[j];
//        raw[j] = raw[j + 1];
//        raw[j + 1] = temp;
//      }
//    }
//  }
//  // возвращаем значение средней ячейки массива
//  return raw[samples / 2];
//}
