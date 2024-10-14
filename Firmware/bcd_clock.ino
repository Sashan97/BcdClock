#include <RTClib.h>

// Shift register
#define ds_pin 4
#define shcp_pin 6
#define stcp_pin 5

// Keyboard
#define snoozeButton 8 
#define setButton 9 
#define hoursButton 10
#define minutesButton 11

RTC_DS3231 rtc;

unsigned long previousMillis = 0;
const long interval = 1000;

// Variables for button states
int snoozeButtonState = LOW;
int setButtonState = LOW;
int hoursButtonState = LOW;
int minutesButtonState = LOW;

// Variables for previous button states
int lastSnoozeButtonState = LOW;
int lastSetButtonState = LOW;
int lastHoursButtonState = LOW;
int lastMinutesButtonState = LOW;

// Timing variables
unsigned long lastDebounceTimeSnooze = 0; // Timing for debounce
unsigned long lastDebounceTimeSet = 0;    // Timing for debounce
unsigned long lastDebounceTimeHours = 0;  // Timing for debounce
unsigned long lastDebounceTimeMinutes = 0; // Timing for debounce

// Debounce time
const unsigned long debounceDelay = 50;

void setup() {
  pinMode(ds_pin, OUTPUT);
  pinMode(stcp_pin, OUTPUT);
  pinMode(shcp_pin, OUTPUT);
  pinMode(13, OUTPUT);

  // Initialize button pins as input
  pinMode(snoozeButton, INPUT); // Pin for Snooze Button
  pinMode(setButton, INPUT);     // Pin for Set Button
  pinMode(hoursButton, INPUT);   // Pin for Hours Button
  pinMode(minutesButton, INPUT); // Pin for Minutes Button
  
  Serial.begin(9600);
  rtc.begin();
  rtc.adjust(DateTime(2024,10,4,22,23,30)); // Настройка времени
}

byte convertToBCD(int seconds) {
  int tens = seconds / 10;  // Получаем старшую цифру (десятки)
  int ones = seconds % 10;  // Получаем младшую цифру (единицы)

  // Формируем BCD: старшая цифра (десятки) смещается на 4 бита влево
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

void handleSnoozeButtonPress() {
    Serial.println("Snooze button pressed.");
}

void handleSetButtonPress() {
    Serial.println("Set button pressed.");
}

void handleHoursButtonPress() {
    Serial.println("Hours button pressed.");
}

void handleMinutesButtonPress() {
    Serial.println("Minutes button pressed.");
}

void handleButton(int buttonPin, int &buttonState, int &lastButtonState, unsigned long &lastDebounceTime, void (*buttonAction)()) {
    int reading = digitalRead(buttonPin); // Read the current state

    // Check if the button state has changed
    if (reading != lastButtonState) {
        lastDebounceTime = millis(); // Reset debounce timer
    }
    
    // If the state has stabilized beyond debounce delay, check for changes
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != buttonState) {
            buttonState = reading; // Update button state
            // Only take action if the button is pressed (HIGH)
            if (buttonState == HIGH) {
                buttonAction(); // Call the specific action function
            }
        }
    }
    
    lastButtonState = reading; // Save reading for the next loop
}

void loop() {
  unsigned long currentMillis = millis();

  handleButton(snoozeButton, snoozeButtonState, lastSnoozeButtonState, lastDebounceTimeSnooze, handleSnoozeButtonPress);
  handleButton(setButton, setButtonState, lastSetButtonState, lastDebounceTimeSet, handleSetButtonPress);
  handleButton(hoursButton, hoursButtonState, lastHoursButtonState, lastDebounceTimeHours, handleHoursButtonPress);
  handleButton(minutesButton, minutesButtonState, lastMinutesButtonState, lastDebounceTimeMinutes, handleMinutesButtonPress);

  // Если прошло 1000 миллисекунд
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    displayCurrentTime();  // Отображаем текущее время
  }
}
