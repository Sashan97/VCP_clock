### Idea
**Swap Arduino Nano to ESP-12F to enable time syncronization with time server.**

### Pros
- RTC clock module is redundant and can be removed - ESP-12F has RTC memery.
- Bluetooth module is redundant and can be removed - web page will be used to adjust settings.

### Cons
- ESP-12F is 3.3V, so the voltage regulator is necessary.
- Also logic level converter will be necessary to work with WS2812B RGB strips.

### Possible solutions
- Voltage regulator - AMS1117 or MP2315
- Logic level converter - 74HCT125
- WiFiManager.h library to manage Wi-Fi credentials