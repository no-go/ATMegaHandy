#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MsTimer2.h>
#include <EEPROM.h>

#define RESET_EEPROM false

char numbers[15] = "+4917780000000";

#define POTI    A3
#define BUTTON1 A2
#define BUTTON2 12

Adafruit_SSD1306 oled(A1, -1, A0);

// max is 19 !
#define SMSPRESET 17

#include <avr/pgmspace.h>
const char string_0[] PROGMEM = "ja";
const char string_1[] PROGMEM = "nein";
const char string_2[] PROGMEM = "mann mann mann";
const char string_3[] PROGMEM = "hm. seltsam.";
const char string_4[] PROGMEM = "haha hehehe";
const char string_5[] PROGMEM = "bitte mich anrufen";
const char string_6[] PROGMEM = "bin da.";
const char string_7[] PROGMEM = "kann nicht";
const char string_8[] PROGMEM = "bis gleich";
const char string_9[] PROGMEM = "komme ca 15min spaeter.";
const char string_A[] PROGMEM = "bin spaet dran";
const char string_B[] PROGMEM = "danke";
const char string_C[] PROGMEM = "nice";
const char string_D[] PROGMEM = "bin (noch) unterwegs.";
const char string_E[] PROGMEM = "Ist auch wichtig.";
const char string_F[] PROGMEM = "0 Problemo";
const char string_G[] PROGMEM = "knifflig. Mein Telefon ist aus Holz.";

const char* const smsTable[] PROGMEM = {
  string_0, string_1, string_2, string_3, string_4, string_5,
  string_6, string_7, string_8, string_9, string_A, string_B,
  string_C, string_D, string_E, string_F, string_G
};

char text[169] = 
    "Guten Morgen, Dave.  " // 21 chars
    "                     "
    "                     "
    "                     "
    "                     "
    "                     "
    "                     "
    "                     "; // 8 lines
uint8_t txtid = 0;
char c = 0;
bool hangedup = true;
uint8_t index = 0;
int sensorValue = 0;
uint8_t eepromId = 0;
uint8_t smsId = 0;
uint8_t pos = 0;
uint8_t recPlyPauSto = 0;
int vcc = 3780;

void clearText() {
  for (txtid = 0; txtid<169; ++txtid) text[txtid] = ' ';
  txtid = 0;
}

inline void initGsm() {
  // echo off
  Serial.println(F("atz"));
  delay(500);
  // echo off
  Serial.println(F("ate0"));
  delay(700);
  // direct store sms
  Serial.println(F("at+cnmi=2,1,0,0,0"));
  delay(1000);  
  // Enable auto network time sync
  Serial.println(F("at+clts=1"));
  delay(1000);  
}

void doRefresh() {
  oled.clearDisplay();
  oled.setCursor(0,0);

  if (sensorValue < 50) {
    // recorder ----------------------------------------
    oled.print(text);
    oled.setCursor(31,8);
    oled.fillRect(29, 6, 70, 12, BLACK);
    oled.print(F("rec > \" stp"));
    if (recPlyPauSto == 0) {
      oled.drawRect(29, 6, 22, 12, WHITE);
    } else if (recPlyPauSto == 1) {
      oled.drawRect(53, 6, 10, 12, WHITE);
    } else if (recPlyPauSto == 2) {
      oled.drawRect(65, 6, 10, 12, WHITE);
    } else {
      oled.drawRect(77, 6, 22, 12, WHITE);
    }
    
  } else if (sensorValue < 200) {
    // power -------------------------------------------
    oled.fillRect(42, 49, 40, 14, BLACK);
    oled.drawRect(42, 49, 38, 14, WHITE);
    oled.fillRect(80, 53, 4, 6, WHITE);
    oled.fillRect(44, 51, map(vcc, 3700, 4200, 2, 34), 10, WHITE);
    strcpy_P(
      &(text[63]),
      pgm_read_word( &(smsTable[smsId]) )
    );

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
    
  } else if (sensorValue < 800) {
    // normal mode (accept, hangup, sms read) --------
    oled.print(text);
    
  } else {
    // number mode ------------------------
    oled.print(F("NUMBER "));
    oled.print(1 + eepromId);
    oled.setCursor(0,16);
    numbers[14] = '\0';
    oled.print(numbers);
    if (pos < 14) {
      oled.setCursor(6*pos,19);
      oled.print(F("_"));
    } else if (pos == 14) {
      oled.setCursor(0,36);
      if (hangedup == true) {
        oled.print(F("DIAL?"));
      } else {
        oled.print(F("DOING A CALL..."));        
      }
    } else {
      oled.setCursor(0,36);
      oled.println(F("SMS?"));
      strcpy_P(
        &(text[63]),
        pgm_read_word( &(smsTable[smsId]) )
      );
      oled.print(&(text[63]));
    }
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

  if (RESET_EEPROM == true) {
    for (eepromId=0; eepromId<6; ++eepromId) EEPROM.put(eepromId*20, numbers);
    eepromId = 0;
  }
  EEPROM.get(eepromId*20, numbers);

  initGsm();
      
  MsTimer2::set(330, doRefresh);
  MsTimer2::start();
}


void loop() {
  sensorValue = analogRead(POTI);

  if (sensorValue < 50) {
    // ----------------------------------- Recorder

    if (digitalRead(BUTTON1) == LOW) {
      delay(200);
      recPlyPauSto = (recPlyPauSto+1)%4;
      clearText();
      txtid = 42;
    }

    if (digitalRead(BUTTON2) == LOW) {
      delay(200);
      if (recPlyPauSto == 0) { // --------------- del(?) and record
        Serial.println(F("AT+CREC=1,1,0"));
        delay(500);
        recPlyPauSto = 3; // stop
      } else if (recPlyPauSto == 1) { // -------- play (loop)
        Serial.println(F("AT+CREC=4,1,0,80,1"));
        delay(500);
        recPlyPauSto = 2; // pause
      } else if (recPlyPauSto == 2) { // -------- pause
        Serial.println(F("AT+CREC=5"));
        delay(500);
        recPlyPauSto = 1; // play
      } else if (recPlyPauSto == 3) { // -------- stop record
        Serial.println(F("AT+CREC=2"));
        delay(500);
        recPlyPauSto = 1; // play
      }
      clearText();
      txtid = 42;
    }
          
  } else if (sensorValue < 200) {

    clearText();
    text[0] = 'P'; text[1] = 'O'; text[2] = 'W'; text[3] = 'E'; text[4] = 'R';
    text[15] = 'S'; text[16] = 'M'; text[17] = 'S';
    txtid = 21;

    if (digitalRead(BUTTON1) == LOW) {
      delay(200);
      smsId = (smsId+1)%SMSPRESET;

    } else if (digitalRead(BUTTON2) == LOW) {
      delay(200);
      if (smsId == 0) smsId = SMSPRESET;     
      smsId = (smsId-1)%SMSPRESET;
    }

    if (smsId < 9) {
      text[20] = '1' + smsId;
    } else {
      text[19] = '1';
      text[20] = '0' + (smsId-9);      
    }
    
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    delay(5);
    ADCSRA |= _BV(ADSC); // Start conversion
    while (bit_is_set(ADCSRA,ADSC)); // measuring
    vcc = ADCL; 
    vcc |= ADCH<<8; 
    vcc = 1126400L / vcc;
    if (vcc<3700) vcc = 3700;    
  
    
  } else if (sensorValue < 400) {


    
    if (digitalRead(BUTTON1) == LOW) {
      txtid = 0;
      initGsm(); 
      
    } else if (digitalRead(BUTTON2) == LOW) {
      txtid = 0;
      delay(200);
      Serial.println(F("ati"));
    } else {
      delay(2000);
      Serial.println(F("at+csq"));
      clearText();
      text[0] = 'S'; text[1] = 'I'; text[2] = 'G'; text[3] = 'N'; text[4] = 'A'; text[5] = 'L';
      txtid = 42;
    }
  } else if (sensorValue < 600) {

    delay(2000);
    Serial.println(F("at+cclk?"));
    clearText();
    text[0] = 'D'; text[1] = 'A'; text[2] = 'T'; text[3] = 'E';
    txtid = 35;
    
  } else if (sensorValue < 800) {
    // --------------------------------------- read sms, accept calls and hang up 
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
    
  } else {
 
    clearText();

    if (sensorValue < 890) {
      eepromId = 0;
    } else if (sensorValue < 930) {
      eepromId = 1;
    } else if (sensorValue < 1015) {
      eepromId = 2;
    } else {
      eepromId = 3;
    }
    EEPROM.get(eepromId*20, numbers);

    if (pos == 14) {
      // ------------------------------------ dial & hangup
      if (digitalRead(BUTTON2) == LOW) {
        delay(200);
        if (hangedup == true) {
          
          Serial.print(F("atd"));
          for (int numberi=0; numberi<14; ++numberi) {
            if (numbers[numberi] != ' ') Serial.print(numbers[numberi]);
          }
          Serial.println(F(";"));
          delay(300);
          hangedup = false;
          Serial.println(F("atm"));
          delay(1000);
          
        } else {
          
          hangedup = true;
          Serial.println(F("ath"));
          delay(2000);
        }
      }
      
      if ((digitalRead(BUTTON1) == LOW) && (hangedup == true)) {
        delay(200);
        pos = 15;
      }
      
    } if (pos == 15) {
      
      if (digitalRead(BUTTON1) == LOW) {
        delay(200);
        pos = 0;
      }
      if (digitalRead(BUTTON2) == LOW) {
        // -------------------------------------- sms send
        pos = 0;        
        Serial.println(F("at+cmgf=1")); // sms text mode
        delay(500);        
        Serial.print(F("AT+CMGS=\""));
        for (int numberi=0; numberi<14; ++numberi) {
          if (numbers[numberi] != ' ') Serial.print(numbers[numberi]);
        }
        Serial.println(F("\""));
        delay(500);
        strcpy_P(
          &(text[63]),
          pgm_read_word( &(smsTable[smsId]) )
        );
        Serial.print(&(text[63]));
        delay(500);
        Serial.print((char)26); // ctrl+z
        delay(500);
      }
    } else {
      // -------------------------------------- number select
      if (digitalRead(BUTTON1) == LOW) {
        delay(300);
        pos = (pos+1)%16;
      } else if (digitalRead(BUTTON2) == LOW) {
        delay(200);
        if (numbers[pos] == ' ') {
          numbers[pos] = '+';
        } else if (numbers[pos] == '+') {
          numbers[pos] = '0';
        } else if (numbers[pos] == '9') {
          numbers[pos] = ' ';
        } else {
          numbers[pos]++;
        }
        EEPROM.write(eepromId*20 + pos, numbers[pos]);
      }       
    }
  }

  if (sensorValue < 800) {
    while ( Serial.available() ) {
      c = Serial.read();
      if (c == '\r') continue;
      if (c == '\0') c = ' ';
      if (c == '\n') c = ' ';
      text[txtid] = c;
      txtid = (txtid+1)%168;
    }
  }
}
