This project is a Binary Coded Decimal (BCD) clock with an alarm function, built using miniature incandescent lamps to visually represent time. The clock is based on an Arduino Pro Mini microcontroller and a DS3231 real-time clock (RTC) module for precise timekeeping. The project combines basic timekeeping functionality with a BCD display system, using common components like shift registers and transistors to drive the bulbs.

## Key Features
### BCD Time Display

Time is represented using incandescent bulbs arranged in Binary Coded Decimal format. Each set of bulbs corresponds to the hours, minutes, and seconds in binary. Each column represents one digit.
### Enclosure

The enclosure is partially 3D-printed, while the indicator chassis holding the bulbs is made from aluminum for better heat dissipation.
### Alarm Function

The clock includes an alarm, with a buzzer that activates at the set time. 

### Automatic Display Shutdown

To save power and extend the life of the bulbs, the clock has an automatic display shutdown feature. After a set period, the indicator turns off. The display can be reactivated by pressing a button.
### Power Supply

The clock is powered by a 9V source, with a voltage regulator providing a stable 5V to the components.

## Setup
The clock has 3 set modes. To switch between set modes, press SET key:

_Time (regular mode) -> Time adjust (SET lamp is on) -> Shutdown timer adjust (SET + SLEEP lamps are on) -> Alarm adjust (SET + ALARM lamps are on) -> Time (regular mode)_

- In **time adjust** mode, use **HOURS** and **MINUTES** buttons to set the time.

- In **shutdown timer adjust** mode, use **HOURS** or **MINUTES** button to adjust the shutdown time (traverses in loop betweeen `sleepDelayOptions`)

- In **alarm adjust** mode, use **HOURS** and **MINUTES** buttons to adjust alarm time. Use SNOOZE button to arm the alarm (ALARM lamp is on) or disarm it (ALARM lamp in flashing).
Alarm can also be armed/disarmed by double-pressing the snooze button in regular time display mode.

- When alarm fires, press **SNOOZE** button to postpone the alarm by `SNOOZE_TIME_MINUTES`, or double-press it to disarm the alarm completely.
