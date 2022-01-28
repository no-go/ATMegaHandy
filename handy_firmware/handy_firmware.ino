#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MsTimer2.h>

#define POTI    A3
#define BUTTON1 A2
#define BUTTON2 12

Adafruit_SSD1306 oled(A1, -1, A0);

char text[169] = 
    "                     " // 21 chars
    "                     "
    "                     "
    "                     "
    "                     "
    "                     "
    "                     "
    "                     "; // 8 lines (+ \0)
int txtid = 0;
char c = 0;
bool hangedup = true;
int index = 0;
int sensorValue = 0;

void clearText() {
  for (txtid = 0; txtid<169; ++txtid) text[txtid] = ' ';
  txtid = 0;
}

void doRefresh() {
  oled.clearDisplay();
  oled.setCursor(0,0);

  if (sensorValue < 200) {
    // power -------------------------------------------
    oled.fillRect(42, 40, 40, 14, BLACK);
    oled.drawRect(42, 40, 38, 14, WHITE);
    oled.fillRect(80, 44, 4, 6, WHITE);
    oled.fillRect(44, 42, map(text[51], '1', '9', 2, 34), 10, WHITE);
    oled.print(text);
    
  } else if (sensorValue < 400) {
    oled.print(text);
    /* SIGNAL -------------------------------------------
     * 0 and 1 = -115 or less and -111 dBm
     * 2 .. 30 = -111 .. -54 (3 is ca -111 +2dBm)
     * 31      = -52 or better
     * 99 = fail
     * 
     * Android(LTE):
     *          -128 no 
     *          -119 (1 Balken)
     *          -109 (2 Balken)
     *           -99 (3 Balken)
     *           -89 (4 Balken)
     */
    oled.drawRect(58, 42, 14, 20, WHITE);
    
    if (text[50] == ',') {
      // 0..9 -> below -97
      oled.drawLine(60,60,60,57, WHITE);
    } else if (text[49] == '1') {
      // -97 or better
      oled.drawLine(60,60,60,57, WHITE);
      oled.drawLine(62,60,62,55, WHITE);
      if (text[50] > '4') {
        // -85 or better
        oled.drawLine(64,60,64,53, WHITE);
      }
    } else if (text[49] == '2') {
      // -75 or better
      oled.drawLine(60,60,60,57, WHITE);
      oled.drawLine(62,60,62,55, WHITE);
      oled.drawLine(64,60,64,53, WHITE);
      oled.drawLine(66,60,66,50, WHITE);
      if (text[50] > '4') {
        // -55 or better
        oled.drawLine(68,60,68,48, WHITE);
      }      
    } else if (text[49] == '3') {
      // -54 or better
      oled.drawLine(60,60,60,57, WHITE);
      oled.drawLine(62,60,62,55, WHITE);
      oled.drawLine(64,60,64,53, WHITE);
      oled.drawLine(66,60,66,50, WHITE);
      oled.drawLine(68,60,68,48, WHITE);
      oled.drawLine(70,60,70,44, WHITE);
    }
    
  } else if (sensorValue < 600) {
    // date and time --------------------------------
    text[42] = '2'; text[43] = '0';
    text[52] = ' ';
    text[61] = ' '; text[62] = ' '; text[63] = ' ';
    oled.print(text);
    oled.setTextSize(3);
    oled.setCursor(0,40);
    oled.print(text[53]); oled.print(text[54]);
    oled.print(text[55]);
    oled.print(text[56]); oled.print(text[57]);
    oled.setTextSize(2);
    oled.setCursor(100,40);
    oled.print(text[59]); oled.print(text[60]);
    oled.setTextSize(1);
    
  } else {
    // normal mode (accept, hangup, sms read) --------
    oled.print(text);
  }
  oled.display();
}

void setup() {
  Serial.begin(57600);
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0,0);
  oled.print(text);
  oled.display();
  
  delay(5000);
  // echo off
  Serial.println(F("ate0"));
  delay(3000);
  // direct store sms
  Serial.println(F("at+cnmi=2,1,0,0,0"));
  delay(1000);
  //Enable auto network time sync
  Serial.println(F("at+clts=1"));
  delay(1000);
  MsTimer2::set(500, doRefresh);
  MsTimer2::start();
}


void loop() {
  sensorValue = analogRead(POTI);

  if (sensorValue < 200) {

    delay(2000);
    Serial.println(F("at+cbc"));
    clearText();
    text[0] = 'P'; text[1] = 'O'; text[2] = 'W'; text[3] = 'E'; text[4] = 'R';
    txtid = 42;
    
  } else if (sensorValue < 400) {

    delay(2000);
    Serial.println(F("at+csq"));
    clearText();
    text[0] = 'S'; text[1] = 'I'; text[2] = 'G'; text[3] = 'N'; text[4] = 'A'; text[5] = 'L';
    txtid = 42;
    
  } else if (sensorValue < 600) {

    delay(2000);
    Serial.println(F("at+cclk?"));
    clearText();
    text[0] = 'D'; text[1] = 'A'; text[2] = 'T'; text[3] = 'E';
    txtid = 35;
    
  } else {
  
    if (digitalRead(BUTTON2) == LOW) {
      delay(200);
      
      if (digitalRead(BUTTON2) == LOW) {
        if (hangedup) {
          oled.clearDisplay();
          oled.setCursor(0,0);
          oled.print(F("ACCEPT."));
          clearText();
          oled.display();
          Serial.println(F("ata"));       
        } else {
          oled.clearDisplay();
          oled.setCursor(0,0);
          oled.print(F("HANG UP."));
          clearText();
          oled.display();
          Serial.println(F("ath"));
        }
        hangedup = !hangedup;
        delay(1000);
      }
    }
    
    if (digitalRead(BUTTON1) == LOW) {
      delay(200);
  
      if (digitalRead(BUTTON2) == LOW) {
        oled.clearDisplay();
        oled.setCursor(0,0);
        oled.print(F("DEL ALL."));
        clearText();
        oled.display();
        Serial.println(F("at+cmgd=1,4"));
        delay(1000);
      }
      
      if (digitalRead(BUTTON1) == LOW) {
        Serial.println(F("at+cmgf=1"));
        clearText();
        text[167] = '1' + index;
        delay(600);
        // read
        Serial.print("at+cmgr=");
        Serial.println(index+1);
        index = (index+1)%6;
      }
    }
  }
    
  while ( Serial.available() ) {
    c = Serial.read();
    if (c == '\r') continue;
    if (c == '\0') c = ' ';
    if (c == '\n') c = ' ';
    text[txtid] = c;
    txtid = (txtid+1)%169;
  }
}
