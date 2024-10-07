#include <RTClib.h>

#define ds_pin 4
#define shcp_pin 6
#define stcp_pin 5

RTC_DS3231 rtc;

unsigned long previousMillis = 0;   // Переменная для хранения последнего времени
const long interval = 1000;         // Интервал в 1 секунду (1000 миллисекунд)
int secondsCounter = 0; 

void setup() {
  // put your setup code here, to run once:
  pinMode(ds_pin, OUTPUT);
  pinMode(stcp_pin, OUTPUT);
  pinMode(shcp_pin, OUTPUT);
  pinMode(13, OUTPUT);

  Serial.begin(9600);
  rtc.begin();
  rtc.adjust(DateTime(2024,10,4,22,23,30));
}

byte convertToBCD(int seconds) {
  int tens = seconds / 10;  // Получаем старшую цифру (десятки)
  int ones = seconds % 10;  // Получаем младшую цифру (единицы)

  // Формируем BCD: старшая цифра (т. е. десятки) смещается на 4 бита влево
  // и складывается с младшей цифрой
  byte bcd = (tens << 4) | ones;

  return bcd;
}

void loop() {
  unsigned long currentMillis = millis();   // Получение текущего времени

  // Если прошло 1000 миллисекунд
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Обновляем предыдущее время

    // Увеличиваем счетчик секунд
    secondsCounter++;
    DateTime now = rtc.now();
    
    byte hours = convertToBCD(now.hour());
    byte minutes = convertToBCD(now.minute());
    byte seconds = convertToBCD(now.second());

    Serial.print(hours, BIN);
    Serial.print(" ");
    Serial.print(minutes, BIN);
    Serial.print(" ");
    Serial.print(seconds, BIN);
    Serial.println();

    // Если прошло 60 секунд, сбрасываем счетчик
    if (secondsCounter >= 60) {
      secondsCounter = 0;
    }

    //byte bcd = convertToBCD(secondsCounter);
    //Serial.println(bcd, BIN);  // Выводим BCD в двоичном формате

    //digitalWrite(stcp_pin, LOW);
    //shiftOut(ds_pin, shcp_pin, LSBFIRST, bcd);
    //digitalWrite(stcp_pin, HIGH);
  }
}
