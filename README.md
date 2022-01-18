# ATMegaHandy

Improved phone with SIM800L GSM module, arduino pro mini, 3 buttons,
rotation encoder (+button) and SD1306 i2c oled display.

- store/select 6 telephone numbers (20 digits incl. space and +) in EEPROM
  - via press rotation encoder
- display lipo power (icon)
- log infos like
  - sms ready, call ready (power up)
  - every ca 10sec NET quality (4 bad ... 20+ good)
  - incomming call (RING)
  - incoming sms (SM: now id)
- print all stored sms (improvement) via blue button
- delete all stored sms (long press blue button)
- accept call/hang up (red LED, toggle via red button)
- select number/change stored number (toggle via yellow button)
  - press rotation encoder: next digit
  - rotate rotation encoder: change digit
  - dial/call number (red button)
  - select one of 11 template sms messages (blue button)
    - red/yellow button to change selecton
    - break by pressing blue button
    - send sms by pressing rotation encoder

It is a kick-ass code and a prove of concept to:

- normal telephone function (dial and take call)
- send and read sms

## Sketch

![Photo](sketch.jpg)

## todo

- circuit
- pictures