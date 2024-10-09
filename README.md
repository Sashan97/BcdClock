> Project is currently in development.

This project is a Binary Coded Decimal (BCD) clock with an alarm function, built using miniature incandescent lamps to visually represent time. The clock is based on an Arduino Pro Mini microcontroller and a DS3231 real-time clock (RTC) module for precise timekeeping. The project combines basic timekeeping functionality with a BCD display system, using common components like shift registers and transistors to drive the bulbs.

## Key Features
### BCD Time Display

Time is represented using incandescent bulbs arranged in Binary Coded Decimal format. Each set of bulbs corresponds to the hours, minutes, and seconds in binary.
### Enclosure

The enclosure is partially 3D-printed, while the indicator chassis holding the bulbs is made from aluminum for better heat dissipation.
### Alarm Function

The clock includes an alarm, with a buzzer that activates at the set time. 

### Automatic Display Shutdown

To save power and extend the life of the bulbs, the clock has an automatic display shutdown feature. After a set period, the indicator turns off. The display can be reactivated by pressing a button.
### Power Supply

The clock is powered by a 9V source, with a voltage regulator providing a stable 5V to the components.
