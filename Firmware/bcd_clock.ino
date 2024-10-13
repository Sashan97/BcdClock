#include <RTClib.h>

#define ds_pin 4
#define shcp_pin 6
#define stcp_pin 5

RTC_DS3231 rtc;

unsigned long previousMillis = 0;
const long interval = 1000;
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

void display(int d1, int d2, int d3) {
  // for hours, shift one bit to the left since Q7 is unused
  byte hours = convertToBCD(d1) << 1;
  byte minutes = convertToBCD(d2);
  byte seconds = convertToBCD(d3);

  digitalWrite(stcp_pin, LOW);
  shiftOut(ds_pin, shcp_pin, LSBFIRST, hours);
  shiftOut(ds_pin, shcp_pin, LSBFIRST, minutes);
  shiftOut(ds_pin, shcp_pin, LSBFIRST, seconds);
  digitalWrite(stcp_pin, HIGH);
}

void displayCurrentTime() {
  DateTime now = rtc.now();
  display(now.hour(), now.minute(), now.second());
}

void loop() {
  unsigned long currentMillis = millis();

  // Если прошло 1000 миллисекунд
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    displayCurrentTime();
  }
}
