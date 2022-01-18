#include <Wire.h> //I2C Arduino Library
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include <EEPROM.h>

#define RESET_EEPROM false

// ------------------------------------------------------

#define SPI_SCK    13
#define SPI_MISO   12
#define SPI_SDA    11
#define SPI_SS     10

#define BUTTON1    A3
#define BUTTON2    4  // blue
#define BUTTON3    3  // red
#define BUTTON4    2  // yellow

#define LEDRED    A0
#define LEDWHITE  9  // unused
#define VIBRATO   8  // unused (only on startup)

#define pinA A1
#define pinB A2

#define SERIAL_SPEED  9600

// HARDWARE I2C: A4 -> SDA, A5 -> SCL
#define PIN_RESET  4 // dummy
Adafruit_SSD1306 oled(PIN_RESET);

bool      logi[8] = {0,0,0,0, 0,0,0,0};
int      tick = 0;
bool  changes = false;
byte      valA = 0;
byte      valB = 0;
byte      valAold = 0;
byte      valBold = 0;

bool valRed = false;

bool numberselect = false;
char numbers[21]  = "+49 1570 000 00 00  "; // and char 21 is numbers[20] = '\0'
byte numberi      = 0;

char buff;
int buffid = 0;

int vcc = 3780;
uint16_t counter = 0;
uint16_t eepromID = 0;

void head() {
  oled.fillRect(0, 0, 20, 8, BLACK);
  oled.drawRect(0, 0, 18, 8, WHITE);
  oled.fillRect(18, 2, 3, 4, WHITE);
  oled.fillRect(2, 2, map(vcc, 3700, 4350, 2, 14), 4, WHITE);
}

void setup() {
  Serial.begin(SERIAL_SPEED);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);

  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  pinMode(BUTTON4, INPUT_PULLUP);
  digitalWrite(BUTTON3, HIGH);
  digitalWrite(BUTTON4, HIGH);
  
  pinMode(LEDRED, OUTPUT);
  pinMode(LEDWHITE, OUTPUT);
  pinMode(VIBRATO, OUTPUT);
  
  oled.begin();
  Serial.println(F("atz")); // reset
  oled.clearDisplay();
  oled.setTextColor(WHITE, BLACK);
  oled.setTextSize(2);
  oled.setCursor(8, 20);
  oled.print(F("TelEiFone"));
  oled.display();

  if (RESET_EEPROM == true) {
    for (eepromID=0; eepromID<6; ++eepromID) {
      EEPROM.put(21*eepromID, numbers);
    }
    eepromID=0;
  }
  
  EEPROM.get(21*eepromID, numbers);
  
  digitalWrite(LEDRED, LOW);
  digitalWrite(LEDWHITE, LOW);
  
  Serial.println(F("ate0")); // echo off
  delay(800);
  
  // store incoming sms and returns: +CMTI: "SM",1\n
  Serial.println(F("at+cnmi=2,1,0,0,0"));
  
  // not store incoming sms and returns the SMS data:
  // +CMT: "+4915xxxyyyyy4","","22/01/13,21:49:52+04"
  // sms text
  //Serial.println(F("at+cnmi=1,2,0,0,0"));
  
  delay(800);
  
  // info/status/hello
  digitalWrite(VIBRATO, HIGH);
  Serial.println(F("ati"));
  delay(500);  
  digitalWrite(VIBRATO, LOW);
  
  // at+cops? get network cooperator
  // at+cops=? list networks cooperator
  // at+cbc  0,80,4025 (0=not charge,80%,4025mV)

  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(0, 8);
}

void loop() {
  counter = (counter+1)%32000;
  
  changes = false;
  valA = digitalRead(pinA);
  valB = digitalRead(pinB);

  if (valA != valAold || valB != valBold) {
    if (valA && valB) {
      tick=0;
      for (int i=0;i<8;++i) {
        logi[i] = false;
      }
    }
    logi[tick] = valA;
    tick = (tick+1)%8;
    logi[tick] = valB;
    tick = (tick+1)%8;
    valAold = valA;
    valBold = valB;
    changes = true;
  }


  // rotation encoder -----------------
  
  if (changes) {      
    if (
      logi[0] == 1 && logi[1] == 1 && logi[2] == 0 && logi[3] == 1 &&
      logi[4] == 0 && logi[5] == 0 && logi[6] == 1 && logi[7] == 0
    ) {
      // anti clockwise
      if (numberselect) {
        if (numbers[numberi] == '+') {
          numbers[numberi] = '9';
        } else if (numbers[numberi] == ' ') {
          numbers[numberi] = '+';
        } else if (numbers[numberi] == '0') {
          numbers[numberi] = ' ';
        } else {
          numbers[numberi]--;
        }
        oled.clearDisplay();
        head();
        oled.setCursor(26, 0);
        oled.print(numbers);
        oled.setCursor(26+6*numberi, 8);
        oled.print(F("-"));
        oled.display();
      }
    }
    if (
      logi[0] == 1 && logi[1] == 1 && logi[2] == 1 && logi[3] == 0 &&
      logi[4] == 0 && logi[5] == 0 && logi[6] == 0 && logi[7] == 1
    ) {
      // clockwise
      if (numberselect) {
        if (numbers[numberi] == '9') {
          numbers[numberi] = '+';
        } else if (numbers[numberi] == '+') {
          numbers[numberi] = ' ';
        } else if (numbers[numberi] == ' ') {
          numbers[numberi] = '0';
        } else {
          numbers[numberi]++;
        }
        oled.clearDisplay();
        head();
        oled.setCursor(26, 0);
        oled.print(numbers);
        oled.setCursor(26+6*numberi, 8);
        oled.print(F("-"));
        oled.display();
      }
    }
  }


  // rotation encoder pressed --------------------------- (next digit)
  
  if (digitalRead(BUTTON1) == LOW) {
    if (numberselect) {
      delay(100);
      numberi = (numberi+1)%20;
      oled.clearDisplay();
      head();
      oled.setCursor(26, 0);
      oled.print(numbers);
      oled.setCursor(26+6*numberi, 8);
      oled.println(F("-"));
      oled.print(F("Addr. id: "));
      oled.print(eepromID+1);
      oled.display();
    } else {
      // -------------------------------------- load next addr id
      EEPROM.put(21*eepromID, numbers);
      eepromID = (eepromID+1)%6;
      EEPROM.get(21*eepromID, numbers);
      oled.clearDisplay();
      head();
      oled.setCursor(26, 0);
      oled.print(F("Addr. id: "));
      oled.print(eepromID+1);
      oled.setCursor(0, 16);
      oled.print(numbers);
      oled.display();
    }
  }


  // blue button ------------------------------------------------ (sms mode !) ---------
  
  if (digitalRead(BUTTON2) == LOW) {
    Serial.println(F("at+cmgf=1")); // sms text mode
    if (numberselect) {
      // select and send sms
      EEPROM.put(21*eepromID, numbers);
      numberselect = false;
      oled.setCursor(0, 8);
      oled.println(F("Send sms:"));
      oled.println(F(" ja       kann nicht"));
      oled.println(F(" nein     bis gleich"));
      oled.println(F(" ohje     ca. +15min"));
      oled.println(F(" hm. ok   bin spaet"));
      oled.println(F(" haha     0 problemo"));
      oled.print  (F(" ruf mich mal an"));
      oled.display();
      uint8_t mid=0;
      
      while (digitalRead(BUTTON1) == HIGH) {
        // -----------------------------------  rotation encoder button: send sms
        
        if (digitalRead(BUTTON2) == LOW) { // 2x blue button: break/not send --------
          delay(200);
          mid=100;
          oled.clearDisplay();
          oled.setCursor(0, 16);
          buffid=0;
          head();
          oled.display();
          break;
        }

        if (digitalRead(BUTTON3) == LOW) { // -------------- red button
          delay(100);
          if (mid<6) oled.setCursor(0, 16+mid*8); else oled.setCursor(54, 16+(mid-6)*8);
          oled.print(F(" "));
          if (mid>0) {
            mid=mid-1;
          }
        }
        if (digitalRead(BUTTON4) == LOW) { // --------------- yellow button
          delay(100);
          if (mid<6) oled.setCursor(0, 16+mid*8); else oled.setCursor(54, 16+(mid-6)*8);
          oled.print(F(" "));
          if (mid<10) {
            mid=mid+1;
          }
        }
        
        if (mid<6) oled.setCursor(0, 16+mid*8); else oled.setCursor(54, 16+(mid-6)*8);
        oled.print(F(">"));
        oled.display();
      }

      if (mid<100) {
        // ----------------------------- send sms
        oled.clearDisplay();
        head();
        oled.setCursor(0, 8);
        buffid=0;
        oled.println(F("send..."));
        Serial.println(F("at+cmgf=1")); // sms text mode
        delay(500);        
        Serial.print(F("AT+CMGS=\""));
        for (numberi=0; numberi<20; ++numberi) {
          if (numbers[numberi] != ' ') Serial.print(numbers[numberi]);
        }
        numberi=0;
        Serial.println(F("\""));
        delay(500);
        switch(mid) {
          case 0:
            Serial.print(F("ja"));
            break;
          case 1:
            Serial.print(F("nein"));
            break;
          case 2:
            Serial.print(F("ohje"));
            break;
          case 3:
            Serial.print(F("hm. okay."));
            break;
          case 4:
            Serial.print(F("haha :-D"));
            break;
          case 5:
            Serial.print(F("Ruf mich mal an!"));
            break;
          case 6:
            Serial.print(F("ich kann nicht"));
            break;
          case 7:
            Serial.print(F("bis gleich"));
            break;
          case 8:
            Serial.print(F("ca. +15min"));
            break;
          case 9:
            Serial.print(F("bin spaet"));
            break;
          case 10:
            Serial.print(F("null problemo"));
            break;
          default:
            Serial.print(F("hm..."));
        }
        delay(500);
        Serial.print((char)26); // ctrl+z
        delay(500);
        
        oled.display();
      }
      
    } else {
      delay(1000);
      if (digitalRead(BUTTON2) == LOW) {
        // --------------------------------- long press!
        oled.setCursor(0, 16);
        oled.println(F("del all sms!"));
        oled.display();
        // ---------------------------------> delete all sms
        Serial.println(F("at+cmgd=1,4")); 
      } else {
        // --------------------------------- read all sms
        Serial.println(F("at+cmgl=\"ALL\",1"));
      }
    }
  }



  // red button --------------------------------------------------
  
  if (digitalRead(BUTTON3) == LOW) {
    valRed = !valRed;
    digitalWrite(LEDRED, valRed);
    oled.clearDisplay();
    head();
    oled.setCursor(0, 8);
    delay(400);
    if (valRed) {
      if (numberselect) {
        // --------------------------------- dial
        EEPROM.put(21*eepromID, numbers);
        numberselect = false;
        oled.println(F("Dial..."));
        oled.display();
        // dial
        Serial.print(F("atd"));
        for (numberi=0; numberi<20; ++numberi) {
          if (numbers[numberi] != ' ') Serial.print(numbers[numberi]);
        }
        numberi = 0;
        Serial.println(F(";"));
        delay(300);
        Serial.println(F("atm"));
      } else {
        // --------------------------------- accept call
        oled.println(F("Accept Call..."));
        oled.display();
        Serial.println(F("ata"));
      }
    } else {
      // --------------------------------- hang up
      oled.println(F("Hang up..."));
      oled.display();
      Serial.println(F("ath"));      
    }
  }


  // yellow button ----------------------------- (select a address/number)

  if (digitalRead(BUTTON4) == LOW) {
    numberselect = !numberselect;
    delay(400);
    oled.clearDisplay();
    head();
    oled.setCursor(26, 0);
    if (numberselect) {
      oled.print(numbers);
      oled.setCursor(26+6*numberi, 8);
      oled.print(F("-"));      
    } else {
      EEPROM.put(21*eepromID, numbers);
    }
  }


  // log: read/print gsm answers -------------------------------
  
  while (Serial.available()) {
    buff = (char) Serial.read();
    if (buff != '\n' && buff != '\r') {
      oled.print(buff);
    } else {
      oled.print(' ');      
    }
    buffid++;
  }


  // log: display refresh ---------------------- 

  if (counter%300==100) {
    
    if (buffid > (21*6)) {
      oled.setCursor(0, 16);
      buffid=0;
    }
    oled.display();
  }  


  // request/update power and net quality ----------------------
  
  if (counter==20000) { 
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    delay(5);
    ADCSRA |= _BV(ADSC); // Start conversion
    while (bit_is_set(ADCSRA,ADSC)); // measuring
    vcc = ADCL; 
    vcc |= ADCH<<8; 
    vcc = 1126400L / vcc;
    if (vcc<3700) vcc = 3700;
    if (numberselect == false) {
      head();
      Serial.println(F("at+csq")); // net quality
    }
  }
}

