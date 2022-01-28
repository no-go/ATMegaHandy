# ATMegaHandy

Improved phone with SIM800L GSM module, arduino pro mini, 2 buttons,
25k POTI and SD1306 SPI oled display.

It is a kick-ass code and a prove of concept to:

- take call / hang up
- read sms
- read date and time
- see signal strength
- see power of battery
- LED indicates incoming call

Usage:

- Poti switchs 4 modes
  - normal
  - continoues date/time requests 
  - continoues signal strength requests
  - continoues battery power requests
- normal mode
  - button1
    - read sms 1,2,3,4,5,6, 1,2, ... (loop on press)
    - incoming sms stored incrementing on index 1,2,...
    - only the first 6 sms are accessable
    - press button2: delete all sms (index back to 1)
  - button2
    - on incomming call: accept
    - hang up (exit a call)

Not Implemented: **It is not possible to dial a number or send sms**.

## Sketch

![Photo](sketch.jpg)
