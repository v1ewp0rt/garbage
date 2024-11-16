#include <Wire.h>
#include <U8g2lib.h>
#include <RTClib.h>
#include <ESP32Servo.h>

#define LEFT_BUTTON 32
#define RIGHT_BUTTON 33
#define WHEEL 39
#define LASER 18
#define FLASH 23

const unsigned char sign [] PROGMEM = {
	0xfe, 0xff, 0x7f, 0xfc, 0xff, 0x7f, 0xf8, 0xff, 0x7f, 0xf0, 0x01, 0x00, 0xe0, 0x03, 0x00, 0xc0, 
	0x07, 0x00, 0x80, 0x0f, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x3e, 0x00, 0xf0, 0xff, 0x7f, 0xf8, 0xff, 
	0x7f, 0xfc, 0xff, 0x7f, 0x3e, 0x38, 0x70, 0x1f, 0x38, 0x70, 0x0f, 0x38, 0x70, 0x1f, 0x38, 0x70, 
	0x3e, 0x38, 0x70, 0xfc, 0x3f, 0x7e, 0xf8, 0x3f, 0x7e, 0xf0, 0x3f, 0x7e
}; const unsigned char flash_icon [] PROGMEM = {
	0x20, 0x00, 0x22, 0x02, 0x04, 0x01, 0x70, 0x00, 0x88, 0x00, 0x8b, 0x06, 0x88, 0x00, 0x70, 0x00, 
	0x04, 0x01, 0x22, 0x02, 0x20, 0x00
}; const unsigned char laser_icon [] PROGMEM = {
	0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0xff, 0x07, 0x20, 0x00, 0x20, 0x00, 
	0x20, 0x00, 0x20, 0x00, 0x20, 0x00
};

U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, 25, 13);
DS1302 rtc(26, 14, 27);
DateTime now;
Servo cannon;

bool laserState=0;
bool flashState=0;
bool leftState=0;
bool rightState=0;
bool lastLeftState=0;
bool lastRightState=0;

String nowStr;

void setup() {
    Wire.begin(25, 13); 
    cannon.attach(5);
    rtc.begin();
    display.begin();
    pinMode(LASER, OUTPUT);
    pinMode(FLASH, OUTPUT);
    pinMode(LEFT_BUTTON, INPUT_PULLDOWN);
    pinMode(RIGHT_BUTTON, INPUT_PULLDOWN);
} void loop() {
    nowStr = String(now.hour())+":"+String(now.minute());
    leftState = digitalRead(LEFT_BUTTON);
    rightState = digitalRead(RIGHT_BUTTON);
    if (lastLeftState && !leftState) { 
        laserState=!laserState; 
        digitalWrite(LASER, laserState);
    } if (lastRightState && !rightState) { 
        flashState=!flashState; 
        digitalWrite(FLASH, flashState);
    } now = rtc.now();
    display.clearBuffer();
    display.setFont(u8g2_font_timR14_tr);
    display.drawStr(64-(display.getUTF8Width(nowStr.c_str())/2), 39, nowStr.c_str());
    display.drawCircle(64, 32, 31);
    if (laserState) { 
        display.drawXBMP(117, 0, 11, 11, laser_icon);
        cannon.write(float(analogRead(WHEEL))*float(180.f/4095.f)); 
        display.setFont(u8g2_font_profont12_tr);
        display.drawStr(0, 64, String(int(analogRead(WHEEL)*float(180.f/4095.f))-21).c_str());
    } if (flashState) { display.drawXBMP(100, 0, 11, 11, flash_icon); } 
    display.drawXBMP(0, 22, 23, 20, sign);
    display.sendBuffer();
    delay(100);
    lastLeftState = leftState;
    lastRightState = rightState;
}