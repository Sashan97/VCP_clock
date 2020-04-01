# VCP clock
Replica of Vilnius Central Post electronic clock

### CLOCK:
* 3D printed case
* Uses Arduino Nano, DS3231 RTC module and WS2812 LED strips
* Supports Bluetooth connection via HC06

### APP:
* Designed in Thunkable
* Allows to turn the clock on and off, adjust time and brightness and toggle lantern mode

### SERIAL PORT TERMINAL COMMUNICATION:

Some advanced clock settings can be changed through the serial port terminal via Bluetooth.

Command syntax:

set,arg1,arg2,arg3,arg4,arg5,arg6,arg7

* arg1 - power, 0 is off, 1 is on.
* arg2 - brightness, 0-255.
* arg3 - lantern mode, 0 is off, 1 is on.
* arg4 - color, 1 - original, 2 - red, 3 - green, 4 - blue.
* arg5 - auto mode, turns clock off and on at certain hours, 0 is off, 1 is on.
* arg6 - auto mode start hour, ignored if arg5 is 0.
* arg7 - auto mode stop our, ignored if arg5 is 0.

For example, sending **set,1,50,0,1,1,7,23** will turn the clock with the brightness 50, without the lantern mode, using original color. Automatic mode will shut down the clock at 23 and turn it on at 7 next day. Notice, that if the command will be sent between 23 and 7, time will not be shown.

Also you can override date and time via terminal by sending

time,arg1,arg2,arg3,arg4,arg5,arg6,arg7

* arg1 - hours 0-23
* arg2 - minutes
* arg3 - seconds
* arg4 - month 1-12
* arg5 - day
* arg6 - year (2 last digits)
* arg7 - day of week 1-7

For example, sending **time,14,21,0,3,4,20,2** will set the time and date to 14:21:00, 3rd of April, 2020, Tuesday.
