#include <RTClib.h>

// Settings
#define SNOOZE_TIME_MINUTES 1
#define ALARM_BUZZER_FREQUENCY_HZ 750
#define DEFAULT_ALARM_HOURS 22
#define DEFAULT_ALARM_MINUTES 24
#define DEFAULT_SLEEP_DELAY_MS 30000
#define DOUBLE_CLICK_THRESHOLD_MS 1000

#define buzzer_pin 3

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
long sleepDelay = DEFAULT_SLEEP_DELAY_MS;
long sleepDelayOptions[] = { 10000, 30000, 60000, 90000, 120000, 300000, 600000};
int sleepDelayOptionIndex = 1;

unsigned long activityMillis = 0;
unsigned long snoozeDoubleClickTime = 0;

// flag indicating if display should update ot the next loop iteration
bool refreshNeeded;

int hour;
int minute;
int second;

bool alarmArmed = false;
bool alarmFired = false;
bool toneActive = false;
int alarmHour = DEFAULT_ALARM_HOURS;
int alarmMinute = DEFAULT_ALARM_MINUTES;

enum : byte { REGULAR, SET_TIME, SET_SLEEP_DELAY, SET_ALARM_TIME } setMode = REGULAR;

void setup() {
  pinMode(buzzer_pin, OUTPUT);
  pinMode(ds_pin, OUTPUT);
  pinMode(stcp_pin, OUTPUT);
  pinMode(shcp_pin, OUTPUT);

  // Initialize mode indicator pins as output
  pinMode(alarmLamp, OUTPUT);
  pinMode(setLamp, OUTPUT);
  pinMode(sleepLamp, OUTPUT);

  // Initialize button pins as input
  pinMode(snoozeButton, INPUT);
  pinMode(setButton, INPUT);
  pinMode(hoursButton, INPUT);
  pinMode(minutesButton, INPUT);

  Serial.begin(9600);
  rtc.begin();
  rtc.adjust(DateTime(2024,10,15,22,23,30));
}

byte convertToBCD(int seconds) {
  int tens = seconds / 10;
  int ones = seconds % 10;

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

  unsigned long clickTime = millis();
  if (clickTime - snoozeDoubleClickTime < DOUBLE_CLICK_THRESHOLD_MS) {
    alarmArmed = !alarmArmed;
    if (!alarmArmed)
      digitalWrite(alarmLamp, LOW);
  }
  snoozeDoubleClickTime = clickTime;

  //TODO: set alarm to current + snooze time
  if (alarmFired) {
    alarmFired = false;
    noTone(buzzer_pin);

    alarmMinute += SNOOZE_TIME_MINUTES;
    while (alarmMinute >= 60) {
      alarmMinute -=60;
      alarmHour++;
      if (alarmHour == 24)
        alarmHour = 0;
    }
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

  refreshNeeded = false;

  handleButton(snoozeButton, snoozeButtonState, lastSnoozeButtonState, lastDebounceTimeSnooze, handleSnoozeButtonPress);
  handleButton(setButton, setButtonState, lastSetButtonState, lastDebounceTimeSet, handleSetButtonPress);
  handleButton(hoursButton, hoursButtonState, lastHoursButtonState, lastDebounceTimeHours, handleHoursButtonPress);
  handleButton(minutesButton, minutesButtonState, lastMinutesButtonState, lastDebounceTimeMinutes, handleMinutesButtonPress);

  unsigned long currentMillis = millis();
  // Refresh time once per second
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    DateTime current = rtc.now();
    hour = current.hour();
    minute = current.minute();
    second = current.second();

    refreshNeeded = true;

    if (alarmArmed && hour == alarmHour && minute == alarmMinute)
      alarmFired = true;
  }

  if (alarmFired && refreshNeeded) {
    activityMillis = currentMillis;
    if (!toneActive) {
      tone(buzzer_pin, ALARM_BUZZER_FREQUENCY_HZ);
      toneActive = true;
    }
    else {
      noTone(buzzer_pin);
      toneActive = false;
    }
  }

  if (setMode == REGULAR) {
    if (millis() - activityMillis >= sleepDelay) {
      display(0, 0, 0);
      digitalWrite(alarmLamp, LOW);
    }
    else {
      if (alarmArmed) {
        digitalWrite(alarmLamp, HIGH);
      }
      if (refreshNeeded) {
        display(hour, minute, second);
      }
    }
  }
  else if (setMode == SET_TIME) {
    if (refreshNeeded) {
      display(hour, minute, 0);
    }
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