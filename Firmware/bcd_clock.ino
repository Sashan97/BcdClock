#include <RTClib.h>

// Shift register
#define ds_pin 4
#define shcp_pin 6
#define stcp_pin 5

// Mode indication
#define alarmLamp 7
#define setLamp 8
#define sleepLamp 9

// Keyboard
#define snoozeButton 10
#define setButton 11
#define hoursButton 12
#define minutesButton 13

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

// Sleep delay in milliseconds. If inactive for this amount of seconds, display will shut down
long sleepDelay = 30000;
long sleepDelayOptions[] = { 10000, 30000, 60000, 90000, 120000, 300000, 600000};
int sleepDelayOptionIndex = 1;

long activityMillis = 0;

enum : byte { REGULAR, SET_TIME, SET_SLEEP_DELAY, SET_ALARM_TIME } setMode = REGULAR;

void setup() {
  pinMode(ds_pin, OUTPUT);
  pinMode(stcp_pin, OUTPUT);
  pinMode(shcp_pin, OUTPUT);

  // Initialize mode indicator pins as output
  pinMode(alarmLamp, OUTPUT);
  pinMode(setLamp, OUTPUT);
  pinMode(sleepLamp, OUTPUT);

  // Initialize button pins as input
  pinMode(snoozeButton, INPUT); // Pin for Snooze Button
  pinMode(setButton, INPUT);     // Pin for Set Button
  pinMode(hoursButton, INPUT);   // Pin for Hours Button
  pinMode(minutesButton, INPUT); // Pin for Minutes Button

  Serial.begin(9600);
  rtc.begin();
  rtc.adjust(DateTime(2024,10,15,22,23,30)); // Настройка времени
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

void changeSleepDelay()
{
  sleepDelayOptionIndex++;

  if (sleepDelayOptionIndex > 6)
    sleepDelayOptionIndex = 0;

  sleepDelay = sleepDelayOptions[sleepDelayOptionIndex];
  displayDelay();
}

void displayDelay() {
  int delayInSeconds = sleepDelay / 1000;
  int minutes = delayInSeconds / 60;
  int seconds;

  if (minutes > 0)
    seconds = delayInSeconds - (minutes * 60);
  else
    seconds = delayInSeconds;

  display(0, minutes, seconds);
}

void handleSetButtonPress() {
    setMode = (setMode + 1) % 4;
	
	// this will trigger the display refresh on the next loop iteration
    if (previousMillis > 1000)
      previousMillis-=1000;

    if (setMode != REGULAR)
      digitalWrite(setLamp, HIGH);
    else
      digitalWrite(setLamp, LOW);

    if (setMode == SET_SLEEP_DELAY)
      digitalWrite(sleepLamp, HIGH);
    else
      digitalWrite(sleepLamp, LOW);

}

void handleHoursButtonPress() {
  if (setMode == SET_TIME)
  {
    DateTime current = rtc.now();

    int hours = current.hour();
    hours++;
    if (hours == 24) hours = 0;

    DateTime newDateTime(
      current.year(), 
      current.month(), 
      current.day(),
      hours,
      current.minute(), 
      current.second()
    );

    rtc.adjust(newDateTime);
    display(newDateTime.hour(), newDateTime.minute(), 0);
  }
  else if (setMode == SET_SLEEP_DELAY)
    changeSleepDelay();
}

void handleMinutesButtonPress() {
  if (setMode == SET_TIME)
  {
    DateTime current = rtc.now();

    int minutes = current.minute();
    minutes++;
    if (minutes == 60) minutes = 0;

    DateTime newDateTime(
      current.year(), 
      current.month(), 
      current.day(),
      current.hour(),
      minutes, 
      current.second()
    );

    rtc.adjust(newDateTime);
    display(newDateTime.hour(), newDateTime.minute(), 0);
  }
  else if (setMode == SET_SLEEP_DELAY)
    changeSleepDelay();

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
              activityMillis = millis();
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

  if (setMode == REGULAR) {
    if (currentMillis - activityMillis >= sleepDelay)
      display(0, 0, 0);
    else {
      // Refresh time once per second
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        displayCurrentTime();
      }
    }
  }
  else if (setMode == SET_TIME) {
    // Refresh time once per second, but remove seconds
     if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      DateTime now = rtc.now();
      display(now.hour(), now.minute(), 0);
    } 
  }
  else if (setMode == SET_SLEEP_DELAY) {
    displayDelay();
  }
}