#include <Wire.h> //I2C Arduino Library
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"


// extern
#define MYPWM1    5
#define MYPWM2    6
#define MYIO1     7
#define SPI_SCK    13
#define SPI_MISO   12
#define SPI_SDA    11
#define SPI_SS     10
#define MYIO2    A6
#define MYIO3    A7

#define BUTTON1    A3
#define BUTTON2    4
#define BUTTON3    3
#define BUTTON4    2

#define LEDRED    A0
#define LEDWHITE  9
#define VIBRATO   8

#define pinA A1
#define pinB A2

#define SERIAL_SPEED  9600

// HARDWARE I2C: A4 -> SDA, A5 -> SCL
#define PIN_RESET  4 // dummy
Adafruit_SSD1306 oled(PIN_RESET);

bool      logi[8] = {0,0,0,0, 0,0,0,0};
int      tick = 0;
bool  changes = false;
int       valA = 0;
int       valB = 0;
int       valAold = 0;
int       valBold = 0;

bool valRed     = false;
byte valWhite   = 0;
bool valVibrato = false;

int  vcc;
uint16_t counter = 0; 

void readVcc() {
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(5);
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
  vcc = ADCL; 
  vcc |= ADCH<<8; 
  vcc = 1126400L / vcc;  
}

void setup() {
  Serial.begin(SERIAL_SPEED);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);

  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  pinMode(BUTTON4, INPUT_PULLUP);

  pinMode(LEDRED, OUTPUT);
  pinMode(VIBRATO, OUTPUT);
  pinMode(LEDWHITE, OUTPUT);

  oled.begin();
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.display(); 
  
  digitalWrite(LEDRED, valRed);
  digitalWrite(VIBRATO, valVibrato);
  digitalWrite(LEDWHITE, valWhite);
  Serial.println(F("at+cmgf=1"));
  
  digitalWrite(VIBRATO, HIGH);
  delay(400);
  digitalWrite(VIBRATO, LOW);

  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  oled.setCursor(10, 20);
  oled.print(F("DIY Phone"));
  oled.display();
  delay(2000);
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.setTextSize(1);
  oled.display();
}

void loop() {
  counter = (counter+1)%12000;
  
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
  
  if (changes) {      
    if (
      logi[0] == 1 && logi[1] == 1 && logi[2] == 0 && logi[3] == 1 &&
      logi[4] == 0 && logi[5] == 0 && logi[6] == 1 && logi[7] == 0
    ) {
      oled.clearDisplay();
    }
    if (
      logi[0] == 1 && logi[1] == 1 && logi[2] == 1 && logi[3] == 0 &&
      logi[4] == 0 && logi[5] == 0 && logi[6] == 0 && logi[7] == 1
    ) {
      valWhite+=16;
      analogWrite(LEDWHITE, valWhite);
    }
  }
  if (digitalRead(BUTTON1) == LOW) {
    valRed = !valRed;
    digitalWrite(LEDRED, valRed);
    digitalWrite(LEDWHITE, LOW);
    oled.clearDisplay();
    oled.setCursor(0, 0);
    delay(500);
    if (valRed) {
      Serial.println(F("ata")); // accept call
    } else {
      Serial.println(F("ath")); // hang up      
    }
  }
  if (digitalRead(BUTTON2) == LOW) {
    delay(400);
    readVcc();
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.print(vcc);
    oled.print(F(" mV "));
    Serial.println(F("at+csq"));   // net quality
    Serial.println(F("at+cclk?")); // time
    Serial.println(F("at+cmgf=1"));
  }
  if (digitalRead(BUTTON3) == LOW) {
    oled.clearDisplay();
    oled.setCursor(0, 0);
    delay(500);
    if (digitalRead(BUTTON3) == LOW && digitalRead(BUTTON4) == LOW) {
      delay(500);
      if (digitalRead(BUTTON3) == LOW && digitalRead(BUTTON4) == LOW) {
        Serial.println(F("at+cmgd=1,4")); // delete all sms
      }
    } else {
      Serial.println(F("at+cmgl=\"REC UNREAD\"")); // read new sms and mark them as read     
    }
  }
  if (digitalRead(BUTTON4) == LOW) {
    oled.clearDisplay();
    oled.setCursor(0, 0);
    delay(500);
    Serial.println(F("at+cmgl=\"REC READ\"")); // read old sms
  }

  if (Serial.available()) {
    oled.print((char) Serial.read());
  }

  
  if (counter%900==0) {
    oled.display();
  }
  
}


