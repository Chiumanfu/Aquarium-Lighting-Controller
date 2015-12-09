// Aquarium Light Controller by Chiu Fang 2014

// Include Libraries
#include <Wire.h>              // For DS1307 Real Time Clock
#include <DS1307RTC.h>         // For DS1307 Real Time Clock
#include <Time.h>              // For Time functions
#include <LiquidCrystalFast.h> // For 16x1 LCD Display
#include <Keypad.h>            // For Keypad
#include <EEPROMex.h>          // For EEPROM
#include <TimerOne.h>          // For 16-bit PWM

// Set constants
const byte pwmPin = 10;        // PWM light control pin
const byte blPin = 9;          // Backlight pin
                               // Pin A4 → I2C SDA, Pin A5 → I2C SCL
LiquidCrystalFast lcd (2, 3, 4, 5, 6, 7, 8); // rs,rw,en1,d4,d5,d6,d7 Init LCD
const byte ROWS = 1; // Keypad one rows
const byte COLS = 4; // Keypad four columns
char keys[ROWS][COLS] = {
  {'1','2','3','4'}
};
byte rowPins[ROWS] = {12}; // Row pinouts of the keypad
byte colPins[COLS] = {15, 14, 17, 16}; // Column pinouts of the keypad (A0, A1, A2, A3)
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS ); // Initialize keypad

// Set Parameter Values
int maxPWM;              // Brightest light level
byte timeOnStart;         // Time that the lights start to ramp on
byte timeOffStart;        // Time that the lights start to ramp off
byte fadeTime;            // Time that the fade takes


// Declare Variables
int valuePWM = 0;
unsigned long blDelay = 0;
unsigned long pwmDelay = 0;
unsigned long timer = 0;
boolean onFlag = false;
boolean offFlag = false;
boolean onFinishFlag = false;
boolean offFinishFlag = false;
char key;

void setup () {
  maxPWM = EEPROM.readInt(0);
  timeOnStart = EEPROM.readByte(2);
  timeOffStart = EEPROM.readByte(3);
  fadeTime = EEPROM.readByte(4);
  
//  Serial.begin(115200);       // For debug
  lcd.begin(8, 2); //Config LCD
  Timer1.initialize(2000); //500Hz PWM
  Timer1.pwm(pwmPin, valuePWM);
  pinMode(blPin, OUTPUT);
}

void loop () {
  
/***** Real Time Clock *****/

  lcd.setCursor(0, 0);
  tmElements_t tm;           // Calls RTC lib
  if (RTC.read(tm)) {        // Read DS1307
    if (tm.Hour >= 0 && tm.Hour < 10) {
      lcd.print('0');
    }
    lcd.print(tm.Hour);
    lcd.print(':');
    if (tm.Minute >= 0 && tm.Minute < 10) {
      lcd.print('0');
    }
    lcd.print(tm.Minute);
    lcd.print(':');
    if (tm.Second >= 0 && tm.Second < 10) {
      lcd.print('0');
    }
    lcd.print(tm.Second);
    lcd.setCursor(0, 1);
    lcd.print(" ->     ");
    lcd.setCursor(4, 1);
    lcd.print(valuePWM);
  } else {
    if (RTC.chipPresent()) {
      lcd.print(F("RTC Stop"));
    } else {
      lcd.print(F("RTC Err "));
    }
  }


/***** Backlight Timer *****/

if ((millis() - blDelay) >= 60000){    // 1 minute
  digitalWrite(blPin, LOW);
}


/***** Lights ON if powered on after timeOnStart *****/

  if (tm.Hour > timeOnStart && tm.Hour < timeOffStart && onFinishFlag == false) {
    valuePWM = maxPWM;
    Timer1.setPwmDuty(pwmPin, valuePWM);
    onFlag = false;
    onFinishFlag = true;
  }


/***** Lights ON *****/
  
  if (tm.Hour == timeOnStart && onFlag == false && onFinishFlag == false) { //tm.Hour
    pwmDelay = (fadeTime * 60000) / maxPWM;    // calculate the correct delay for the ramp time
    onFlag = true;
    onFinishFlag = false;
    valuePWM ++;
    Timer1.setPwmDuty(pwmPin, valuePWM);
    timer = millis();
  }

  if (valuePWM == maxPWM && onFinishFlag == false) {
    onFlag = false;
    onFinishFlag = true;
    offFinishFlag = false;
  }

  if (onFlag == true) {
    if ((millis() - timer) >= pwmDelay){
      valuePWM ++;
      Timer1.setPwmDuty(pwmPin, valuePWM);
      timer = millis();
    }
  }


/***** Lights OFF *****/

  if (tm.Hour == timeOffStart && offFlag == false && offFinishFlag == false && onFinishFlag == true) {
    pwmDelay = (fadeTime * 60000) / maxPWM;    // calculate the correct delay for the ramp time
    offFlag = true;
    offFinishFlag = false;
    valuePWM --;
    Timer1.setPwmDuty(pwmPin, valuePWM);
    timer = millis();
  }

  if (valuePWM == 0 && offFinishFlag == false){
    offFlag = false;
    offFinishFlag = true;
    onFinishFlag = false;
  }
  
  if (offFlag == true) {
    if ((millis() - timer) >= pwmDelay){
      valuePWM --;
      Timer1.setPwmDuty(pwmPin, valuePWM);
      timer = millis();
    }
  }


/***** Menu System *****/

  key = keypad.getKey();
  if(key) {
    digitalWrite(blPin, HIGH);
    switch(key) {
    
    case '1':
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Max PWM "));
      lcd.setCursor(0, 1);
      lcd.print(F("=       "));
      lcd.setCursor(2, 1);
      lcd.print(maxPWM);
      do {
        key = keypad.waitForKey();
        if(key == '1' && maxPWM < 1023) {
          maxPWM++;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("Max PWM "));
          lcd.setCursor(0, 1);
          lcd.print(F("=       "));
          lcd.setCursor(2, 1);
          lcd.print(maxPWM);
        } else if(key == '1' && maxPWM >= 1023) {
          maxPWM = 0;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("Max PWM "));
          lcd.setCursor(0, 1);
          lcd.print(F("=       "));
          lcd.setCursor(2, 1);
          lcd.print(maxPWM);
        } else if(key == '2' && maxPWM > 0) {
          maxPWM--;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("Max PWM "));
          lcd.setCursor(0, 1);
          lcd.print(F("=       "));
          lcd.setCursor(2, 1);
          lcd.print(maxPWM);
        } else if(key == '2' && maxPWM == 0) {
          maxPWM = 1023;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("Max PWM "));
          lcd.setCursor(0, 1);
          lcd.print(F("=       "));
          lcd.setCursor(2, 1);
          lcd.print(maxPWM);
        }
      } while(key != '4');
      EEPROM.updateInt(0,maxPWM);
      valuePWM = maxPWM;
      Timer1.setPwmDuty(pwmPin, valuePWM);
      blDelay = millis();
      break;

    case '2':
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("On Time "));
      lcd.setCursor(0, 1);
      lcd.print(F("=       "));
      lcd.setCursor(2, 1);
      lcd.print(timeOnStart);
      do {
        key = keypad.waitForKey();
        if(key == '1' && timeOnStart < 23) {
          timeOnStart++;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("On Time "));
          lcd.setCursor(0, 1);
          lcd.print(F("=       "));
          lcd.setCursor(2, 1);
          lcd.print(timeOnStart);
        } else if(key == '1' && timeOnStart == 23) {
          timeOnStart = 0;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("On Time "));
          lcd.setCursor(0, 1);
          lcd.print(F("=       "));
          lcd.setCursor(2, 1);
          lcd.print(timeOnStart);
        } else if(key == '2' && timeOnStart > 0) {
          timeOnStart--;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("On Time "));
          lcd.setCursor(0, 1);
          lcd.print(F("=       "));
          lcd.setCursor(2, 1);
          lcd.print(timeOnStart);
        } else if(key == '2' && timeOnStart == 0) {
          timeOnStart = 23;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("On Time "));
          lcd.setCursor(0, 1);
          lcd.print(F("=       "));
          lcd.setCursor(2, 1);
          lcd.print(timeOnStart);
        }
      } while(key != '4');
      EEPROM.updateByte(2,timeOnStart);
      blDelay = millis();
      break;

    case '3':
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Off Time"));
      lcd.setCursor(0, 1);
      lcd.print(F(" =      "));
      lcd.setCursor(3, 1);
      lcd.print(timeOffStart);
      do {
        key = keypad.waitForKey();
        if(key == '1' && timeOffStart < 23) {
          timeOffStart++;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("Off Time"));
          lcd.setCursor(0, 1);
          lcd.print(F(" =      "));
          lcd.setCursor(3, 1);
          lcd.print(timeOffStart);
        }else if(key == '1' && timeOffStart == 23) {
          timeOffStart = 0;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("Off Time"));
          lcd.setCursor(0, 1);
          lcd.print(F(" =      "));
          lcd.setCursor(3, 1);
          lcd.print(timeOffStart);
        } else if(key == '2' && timeOffStart > 0) {
          timeOffStart--;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("Off Time"));
          lcd.setCursor(0, 1);
          lcd.print(F(" =      "));
          lcd.setCursor(3, 1);
          lcd.print(timeOffStart);
        } else if(key == '2' && timeOffStart == 0) {
          timeOffStart = 23;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("Off Time"));
          lcd.setCursor(0, 1);
          lcd.print(F(" =      "));
          lcd.setCursor(3, 1);
          lcd.print(timeOffStart);
        }
      } while(key != '4');
      EEPROM.updateByte(3,timeOffStart);
      blDelay = millis();
      break;
    
    case '4':
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Fade Tim"));
      lcd.setCursor(0, 1);
      lcd.print(F("e =     "));
      lcd.setCursor(4, 1);
      lcd.print(fadeTime);
      do {
        key = keypad.waitForKey();
        if(key == '1') {
          fadeTime++;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("Fade Tim"));
          lcd.setCursor(0, 1);
          lcd.print(F("e =     "));
          lcd.setCursor(4, 1);
          lcd.print(fadeTime);
        } else if(key == '2') {
          fadeTime--;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("Fade Tim"));
          lcd.setCursor(0, 1);
          lcd.print(F("e =     "));
          lcd.setCursor(4, 1);
          lcd.print(fadeTime);
        }
      } while(key != '4');
      EEPROM.updateByte(4,fadeTime);
      blDelay = millis();
      break;
    }
  }
}
