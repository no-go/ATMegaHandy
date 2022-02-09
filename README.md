# ATMegaHandy

Improved phone with:

- SIM800L GSM module
- arduino pro mini 8MHz 3.3V
- 3 buttons (press: connect to ground)
  - next button (`BUTTON1` on Pin 4)
  - change button (`BUTTON2` on Pin 3)
  - mode button (`BUTTON4` on Pin 2)
- a red (indicate take call, Pin A0) and a white LED (Pin 9, blink: indicate RING)
- vibration module/pad (indicate RING, Pin 8)
- SD1306 I2C oled display

The `BUTTON3` and `pinA`, `pinB` from rotation encoder are not used. I tried
to move the wood phone code (2 buttons, 1 poti, spi display) to my first
hardware version (metal box).

It was a kick-ass code and a prove of concept to:

- take call / hang up
- dial
- send default sms messages
- store/change 4 phone numbers (store in ATmega EEPROM)
- read sms (store up to 6, delete all)
- read date and time
- see signal strength
- see power of battery
- LED+VIBRATION indicates incoming call (I use a level converter)
- play/record sound

## Usage

Mode button switches through:

- edit stored number 1-4, dail or send sms
- normal (auto switches to that mode on incoming call: RING pin on modem goes to LOW in A6)
- continoues date/time requests 
- continoues signal strength requests
- continoues battery power requests
- audio recorder

### Mode Details

edit/dial/sms mode

- mode button: select phone nummber 1-4 to change
- next button: select digit 1,2,3.... dial, send sms
- change button: change digit 0-9, space, + or select dial/hangup or send sms

normal mode

- next button
  - read sms 1,2,3,4,5,6, 1,2, ... (loop on press)
  - incoming sms stored incrementing on index 1,2,...
  - only the first 6 sms are accessable
  - and press change button: delete all sms (index back to 1)
- change button
  - on incomming call: accept
  - hang up (exit a call)

signal strength mode

- next button (long press): init gsm modem
- change button: gsm modem info

power mode

- next/change button: select sms text for edit/dail/sms mode

record mode

- next button: select function rec, play, pause, stop reord
- change button: execute function
- automatic jump selection between...
  - rec, rec stop, play
  - play, pause

## Bugs

- sometimes init on startup fails a bit (use next button in signal stength mode)
- echo off in init fails at beginning
- it is hard to jump into signal strength mode
- mode press and mode switch has a big (2sec) delay sometimes
- missing circuit here
