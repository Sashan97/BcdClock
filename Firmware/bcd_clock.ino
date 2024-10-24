#include <RTClib.h>

#define DEFAULT_ALARM_HOURS 8
#define DEFAULT_ALARM_MINUTES 0
#define SNOOZE_TIME_SECONDS

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
const unsigned long debounceDelay = 20;

// Sleep delay in milliseconds. If inactive for this amount of seconds, display will shut down
long sleepDelay = 30000;
long sleepDelayOptions[] = { 10000, 30000, 60000, 90000, 120000, 300000, 600000};
int sleepDelayOptionIndex = 1;

long activityMillis = 0;

// flag indicating if display should update ot the next loop iteration
bool refreshNeeded;

bool alarmArmed = false;
bool alarmFired = false;
int alarmHour = DEFAULT_ALARM_HOURS;
int alarmMinute = DEFAULT_ALARM_MINUTES;

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

  displayBytes(hours, minutes, seconds);
}

void displayBytes(byte d1, byte d2, byte d3) {
  digitalWrite(stcp_pin, LOW);
  shiftOut(ds_pin, shcp_pin, LSBFIRST, d1);
  shiftOut(ds_pin, shcp_pin, LSBFIRST, d2);
  shiftOut(ds_pin, shcp_pin, LSBFIRST, d3);
  digitalWrite(stcp_pin, HIGH);
}

void displayCurrentTime() {
  DateTime now = rtc.now();
  display(now.hour(), now.minute(), now.second());
}

void handleSnoozeButtonPress() {
  refreshNeeded = true;

  //TODO: set alarm to current + snooze time
  if (alarmFired) {
    alarmArmed = false;
    alarmFired = false;
  }
  
  if (setMode == SET_ALARM_TIME)
    alarmArmed = !alarmArmed;
}

void changeSleepDelay() {
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

    refreshNeeded = true;

    if (setMode != REGULAR)
      digitalWrite(setLamp, HIGH);
    else
      digitalWrite(setLamp, LOW);

    if (setMode == SET_SLEEP_DELAY)
      digitalWrite(sleepLamp, HIGH);
    else
      digitalWrite(sleepLamp, LOW);

    if (setMode == SET_ALARM_TIME)
      digitalWrite(alarmLamp, HIGH);
    else
      digitalWrite(alarmLamp, LOW);

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
  else if (setMode == SET_ALARM_TIME) {
    alarmHour++;
    if (alarmMinute >= 60)
      alarmMinute = 0;
  }
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
  else if (setMode == SET_ALARM_TIME) {
    alarmMinute++;
    if (alarmMinute >= 60)
      alarmMinute = 0;
  }
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

  refreshNeeded = false;

  handleButton(snoozeButton, snoozeButtonState, lastSnoozeButtonState, lastDebounceTimeSnooze, handleSnoozeButtonPress);
  handleButton(setButton, setButtonState, lastSetButtonState, lastDebounceTimeSet, handleSetButtonPress);
  handleButton(hoursButton, hoursButtonState, lastHoursButtonState, lastDebounceTimeHours, handleHoursButtonPress);
  handleButton(minutesButton, minutesButtonState, lastMinutesButtonState, lastDebounceTimeMinutes, handleMinutesButtonPress);

  DateTime current;

  // Refresh time once per second
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    current = rtc.now();
    refreshNeeded = true;
  }

  if (alarmFired) {
    // TODO: start beeping
    Serial.println("Alarm!");
  }

  if (setMode == REGULAR) {
    if (alarmArmed)
      digitalWrite(alarmLamp, HIGH);

    if (currentMillis - activityMillis >= sleepDelay) {
      display(0, 0, 0);
    }
    else {
      if (refreshNeeded) {
        display(current.hour(), current.minute(), current.second());

        if (alarmArmed && current.hour() == alarmHour && current.minute() == alarmMinute)
          alarmFired = true;
      }
    }
  }
  else if (setMode == SET_TIME) {
    if (refreshNeeded)
      display(current.hour(), current.minute(), 0);
  }
  else if (setMode == SET_SLEEP_DELAY) {
    displayDelay();
  }
  else if (setMode == SET_ALARM_TIME) {
    display(alarmHour, alarmMinute, 0);

    // if alarm is not armed - alarm lamp blinks
    if (!alarmArmed) {
      if (refreshNeeded) {
        bool currentLampState = digitalRead(alarmLamp);
        digitalWrite(alarmLamp, !currentLampState);
      }
    }
    else
      digitalWrite(alarmLamp, HIGH);
  }
}