#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <SD.h>
#include <SPI.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include <NewPing.h>

#define PI 3.141592653589

const char* SSID = "P1M0";
const char* PASSWORD= "obeyyourmaster";

WebServer SERVER(80);
NewPing SONAR(2, 35, 200);

Servo RFOREARM;
Servo LFOREARM;
Servo RARM;
Servo LARM;
Servo EYELIP;
Servo HEAD;

int OBSTACLE_DISTANCE;

const float SPEED_MULTIPLIER=1.0;

String TOKENIZE(String DATA, char DELIMITER, int INDEX) {
  int FOUND = 0;
  int STR_INDEX[] = {0, -1};
  int MAX_INDEX = DATA.length()-1;

  for(int I=0; I<=MAX_INDEX && FOUND<=INDEX; I++) {
    if (DATA.charAt(I)==DELIMITER || I==MAX_INDEX) {
      FOUND++;
      STR_INDEX[0]=STR_INDEX[1]+1;
      STR_INDEX[1]=(I==MAX_INDEX) ? I+1 : I;
    }
  }
  return FOUND>INDEX ? DATA.substring(STR_INDEX[0], STR_INDEX[1]) : "";
}

void PLAYWAV(const char* FILEPATH) {
  File WAVFILE = SD.open(FILEPATH);
  ledcAttachPin(25, 12);
  while (WAVFILE.available()) { ledcWrite(12, WAVFILE.read()); }
  ledcDetachPin(25);
  WAVFILE.close();
}

void WHEELS(String DIR, int SPEED=255, bool TIMER=false, int TIME=1000) {
  ledcWrite(13, map(SPEED, 0, 255, 140, 255));
  if (DIR=="BR") { digitalWrite(14, LOW); digitalWrite(22, LOW); digitalWrite(33, HIGH); digitalWrite(32, LOW); }
  if (DIR=="F") { digitalWrite(14, HIGH); digitalWrite(22, HIGH); digitalWrite(33, LOW); digitalWrite(32, LOW); }
  if (DIR=="BL") { digitalWrite(14, LOW); digitalWrite(22, LOW); digitalWrite(33, LOW); digitalWrite(32, HIGH); }
  if (DIR=="L") { digitalWrite(14, LOW); digitalWrite(22, HIGH); digitalWrite(33, LOW); digitalWrite(32, HIGH); }
  if (DIR=="FL") { digitalWrite(14, LOW); digitalWrite(22, HIGH); digitalWrite(33, LOW); digitalWrite(32, LOW); }
  if (DIR=="B") { digitalWrite(14, LOW); digitalWrite(22, LOW); digitalWrite(33, HIGH); digitalWrite(32, HIGH); }
  if (DIR=="FR") { digitalWrite(14, HIGH); digitalWrite(22, LOW); digitalWrite(33, LOW); digitalWrite(32, LOW); }
  if (DIR=="R") { digitalWrite(14, HIGH); digitalWrite(22, LOW); digitalWrite(33, HIGH); digitalWrite(32, LOW); }
  if (TIMER) {
    delay(TIME);
    digitalWrite(14, LOW); digitalWrite(22, LOW);
    digitalWrite(33, LOW); digitalWrite(32, LOW);
  }
}

float SMOOTH(int T, int P) { return cos(T/(float(P)/(2*PI))+PI)*0.5+0.5; }

void SERVO_360_SMOOTH(Servo &SERVO, int SPEED, int DURATION) {
  for (int T=0; T<DURATION; T++) {
    SERVO.writeMicroseconds(1500+SPEED*float(SMOOTH(T, DURATION))); delay(1);
  }
}

void SERVO_180_SMOOTH(Servo &SERVO, int S_ANGLE, int E_ANGLE) {
  int I;
  int STEPS=abs(E_ANGLE-S_ANGLE);
 
  if (E_ANGLE>S_ANGLE) { I=1; }
  else { I=-1; }
  
  for (int T=0; T<=STEPS; T++) {
    SERVO.write(S_ANGLE+T*I);
    delay(25-24*SMOOTH(T, STEPS));
  }
}

void MAINPAGE() {
  if (WiFi.softAPgetStationNum()>=1) {
    File HTMLFILE = SD.open("/index.html");
    String HTML = "";
    while (HTMLFILE.available()) { HTML += char(HTMLFILE.read()); }
    HTMLFILE.close();
    SERVER.send(200, "text/html", HTML);
  }
}

void PROCESS_PULSE() {
  String PULSE = SERVER.arg("plain");

  SERVER.send(200, "text/html", PULSE);
  
  if (TOKENIZE(PULSE, '&', 0)=="FL") { WHEELS("FL", TOKENIZE(PULSE, '&', 1).toInt()*0.75); }
  if (TOKENIZE(PULSE, '&', 0)=="F") { WHEELS("F", TOKENIZE(PULSE, '&', 1).toInt()); }
  if (TOKENIZE(PULSE, '&', 0)=="FR") { WHEELS("FR", TOKENIZE(PULSE, '&', 1).toInt()*0.75); }
  if (TOKENIZE(PULSE, '&', 0)=="L") { WHEELS("L", TOKENIZE(PULSE, '&', 1).toInt()*0.5); }
  if (TOKENIZE(PULSE, '&', 0)=="BL") { WHEELS("BL", TOKENIZE(PULSE, '&', 1).toInt()*0.75); }
  if (TOKENIZE(PULSE, '&', 0)=="B") { WHEELS("B", TOKENIZE(PULSE, '&', 1).toInt()); }
  if (TOKENIZE(PULSE, '&', 0)=="BR") { WHEELS("BR", TOKENIZE(PULSE, '&', 1).toInt()*0.75); }
  if (TOKENIZE(PULSE, '&', 0)=="R") { WHEELS("R", TOKENIZE(PULSE, '&', 1).toInt()*0.5); }

  if (PULSE == "N") {
    digitalWrite(14, LOW); digitalWrite(22, LOW);
    digitalWrite(33, LOW); digitalWrite(32, LOW);
  }

  if (PULSE == "EYEUP") { EYELIP.writeMicroseconds(1600); }
  if (PULSE == "EYEDN") { EYELIP.writeMicroseconds(1400); }
  if (PULSE == "EYEN") { EYELIP.writeMicroseconds(1500); }

  if (PULSE == "HEADR") { HEAD.writeMicroseconds(1400); }
  if (PULSE == "HEADL") { HEAD.writeMicroseconds(1600); }
  if (PULSE == "HEADN") { HEAD.writeMicroseconds(1500); }

  if (PULSE == "V1") { PLAYWAV(String("/SOUNDS/"+String(random(1, 11))+".wav").c_str()); }
  if (PULSE == "V2") { PLAYWAV(String("/SOUNDS/"+String(random(12, 37))+".wav").c_str()); }
 
  if (PULSE == "E1") { EMOTE(0); }
  if (PULSE == "E2") { EMOTE(0); EMOTE(1); EMOTE(3); EMOTE(0); }
  if (PULSE == "E3") { EMOTE(0); EMOTE(5); EMOTE(0); }
  if (PULSE == "E4") { EMOTE(0); EMOTE(1); EMOTE(4); EMOTE(0); }
  if (PULSE == "E5") { EMOTE(0); EMOTE(1); EMOTE(2); EMOTE(0); }

  if (PULSE == "ATK1") { ATTACK(0); }
  if (PULSE == "ATK2") { ATTACK(1); }
  if (PULSE == "ATK3") { ATTACK(2); }
}

void EMOTE(int ID) {
  if (ID==0) { //IDLE
    LFOREARM.write(90); RFOREARM.write(90);
    LARM.write(15); RARM.write(165); 
  }
  if (ID==1) { //T-POSE
    LFOREARM.write(0); RFOREARM.write(180);
    LARM.write(90); RARM.write(90); 
  }
  if (ID==2) { //FLY
    LFOREARM.write(0); RFOREARM.write(180);
    LARM.write(90); RARM.write(90); 
    delay(200);
    for (int R=0; R<rand()%3+3; R++) {
      LARM.write(135); RARM.write(45);
      delay(250);
      LARM.write(45); RARM.write(135);
      delay(250);
    }
  }
  if (ID==3) { //RUN
    for (int R=0; R<rand()%3+3; R++) {
      LFOREARM.write(0); RFOREARM.write(45);
      LARM.write(135); RARM.write(135); 
      delay(200);
      LFOREARM.write(135); RFOREARM.write(180);
      LARM.write(45); RARM.write(45);
      delay(200); 
    }
  }
  if (ID==4) { //SNEER
    LFOREARM.write(0); RFOREARM.write(180);
    LARM.write(180); RARM.write(0); 
    delay(100);
    for (int R=0; R<rand()%3+3; R++) {
      LFOREARM.write(135); RFOREARM.write(180);
      delay(200);
      LFOREARM.write(0); RFOREARM.write(45);
      delay(200);
    }
  }
  if (ID==5) { //GREETINGS
    LFOREARM.write(90); RFOREARM.write(180);
    LARM.write(15); RARM.write(0); 
    delay(200);
    for (int R=0; R<rand()%3+3; R++) {
      RARM.write(45); delay(200);
      RARM.write(0); delay(200);
    }
  }
  if (ID==6) { //HEAD IDLE
    SERVO_360_SMOOTH(HEAD, -200, 1000); 
    SERVO_360_SMOOTH(HEAD, 200, 475);
  }
  if (ID==7) { //NO
    for (int R=0; R<rand()%3; R++) {
      SERVO_360_SMOOTH(HEAD, -250, 900); 
      SERVO_360_SMOOTH(HEAD, 250, 900);  
    }
  }
  if (ID==8) { //LOOK RIGHT
    SERVO_360_SMOOTH(HEAD, -100, 1000); 
    delay(3000);
    SERVO_360_SMOOTH(HEAD, 125, 700);
  }
  if (ID==9) { //LOOK LEFT
    SERVO_360_SMOOTH(HEAD, 125, 1000); 
    delay(3000);
    SERVO_360_SMOOTH(HEAD, -100, 850);
  }
  if (ID==10) { //CRAZY
    for (int R=0; R<6+rand()%6; R++) {
      SERVO_360_SMOOTH(HEAD, -500, 150); 
      SERVO_360_SMOOTH(HEAD, 500, 150);  
    }
  }
  if (ID==11) { //EYELIP IDLE
    SERVO_360_SMOOTH(EYELIP, 150, 1000); 
  }
  if (ID==12) { //HALF EYE
    SERVO_360_SMOOTH(EYELIP, -150, 300);
  }
  if (ID==13) { //BLINK
    SERVO_360_SMOOTH(EYELIP, 150, 500); 
    SERVO_360_SMOOTH(EYELIP, -200, 750); 
    SERVO_360_SMOOTH(EYELIP, 200, 900); 
  }
}

void ATTACK(int ID) {
  if (ID==0) { //LEFT
    LFOREARM.write(0); RFOREARM.write(90);
    LARM.write(90); RARM.write(165); 
    WHEELS("R", 155, true, 500);
  }
  if (ID==1) { //HELICOPTER
    LFOREARM.write(0); RFOREARM.write(180);
    LARM.write(90); RARM.write(90); 
    WHEELS("R", 155, true, 1000);
  }
  if (ID==2) { //RIGHT
    LFOREARM.write(90); RFOREARM.write(180);
    LARM.write(15); RARM.write(90); 
    WHEELS("L", 155, true, 500);
  }
}

bool IMPULSE(float PERCENTAGE) {
  if (random(1, 100/PERCENTAGE)==1) { return true; }
  else { return false; }
}

void setup() { 
  Serial.begin(115200);
  SD.begin(5);

  pinMode(14, OUTPUT); pinMode(22, OUTPUT);
  pinMode(33, OUTPUT); pinMode(32, OUTPUT);

  ledcSetup(12, 44100, 8);
  ledcSetup(13, 20000, 8);
  ledcAttachPin(21, 13);
  
  RARM.setPeriodHertz(50);
  LARM.setPeriodHertz(50);  
  RFOREARM.setPeriodHertz(50);
  LFOREARM.setPeriodHertz(50);
  EYELIP.setPeriodHertz(50);
  HEAD.setPeriodHertz(50);
  
  RARM.attach(15, 1000, 2500);
  LARM.attach(12, 1000, 2500);
  RFOREARM.attach(4, 1500, 2500);
  LFOREARM.attach(13, 500, 1500);
  HEAD.attach(27, 1000, 2000);
  EYELIP.attach(26, 1000, 2000);
  
  WiFi.softAP(SSID, PASSWORD); 
  SERVER.on("/", MAINPAGE);
  SERVER.on("/PULSE", HTTP_POST, PROCESS_PULSE);
  SERVER.serveStatic("/", SD, "/");
  SERVER.begin();
  
  PLAYWAV("/SOUNDS/4.wav");
  EMOTE(0);
  EMOTE(6);
  EMOTE(11);
  EMOTE(12);
}

void loop() {
  if (WiFi.softAPgetStationNum()>0) { SERVER.handleClient(); }
  else {
    if (IMPULSE(0.001)) { 
      int P=random(100, 500);
      for(int T=0; T<=P; T++) {
        OBSTACLE_DISTANCE=SONAR.ping_cm();
        if (OBSTACLE_DISTANCE==0) { OBSTACLE_DISTANCE=200; }
        if (OBSTACLE_DISTANCE>40) { WHEELS("F", random(155, 220)*SPEED_MULTIPLIER, true, 10); }
        else {
          if (IMPULSE(0.5)) { PLAYWAV(String("/SOUNDS/"+String(random(1, 25))+".wav").c_str()); }
          if (IMPULSE(0.5)) { WHEELS("BR", 155*SPEED_MULTIPLIER, true, 1500); }
          else if (IMPULSE(0.5)) { WHEELS("BL", 155*SPEED_MULTIPLIER, true, 1500);  }
          else { 
            WHEELS("B", 155*SPEED_MULTIPLIER, true, 700); 
            WHEELS("R", 155*SPEED_MULTIPLIER, true, 300);
            break; 
          }
        }
      }  
    }
    if (IMPULSE(0.001)) {
      int LEFT, FRONT, RIGHT;
      
      FRONT=SONAR.ping_cm();
      if (FRONT==0) { FRONT=200; }
      SERVO_360_SMOOTH(HEAD, 125, 1000);
      LEFT=SONAR.ping_cm();
      if (LEFT==0) { LEFT=200; }
      SERVO_360_SMOOTH(HEAD, -100, 2000);
      RIGHT=SONAR.ping_cm();
      if (RIGHT==0) { RIGHT=200; }
    
      SERVO_360_SMOOTH(HEAD, 200, 475);

      if (LEFT>FRONT && LEFT>RIGHT && LEFT>40) {
        WHEELS("B", 155*SPEED_MULTIPLIER, true, 200);
        SERVO_360_SMOOTH(HEAD, 125, 1000); 
        HEAD.writeMicroseconds(1400);
        WHEELS("L", 200, true, 300);
        delay(100);
        HEAD.writeMicroseconds(1500);

        int P=random(100, 500);
        for(int T=0; T<=P; T++) {
          OBSTACLE_DISTANCE=SONAR.ping_cm();
          if (OBSTACLE_DISTANCE==0) { OBSTACLE_DISTANCE=200; }
          if (OBSTACLE_DISTANCE>40) { WHEELS("F", random(155, 220)*SPEED_MULTIPLIER, true, 10); }
          else {
            if (IMPULSE(0.5)) { PLAYWAV(String("/SOUNDS/"+String(random(1, 25))+".wav").c_str()); }
            if (IMPULSE(0.5)) { WHEELS("BR", 155*SPEED_MULTIPLIER, true, 1500); }
            else if (IMPULSE(0.5)) { WHEELS("BL", 155*SPEED_MULTIPLIER, true, 1500);  }
            else { 
              WHEELS("B", 155*SPEED_MULTIPLIER, true, 700); 
              WHEELS("R", 155*SPEED_MULTIPLIER, true, 300);
              break; 
            }
          }
        }  
      }
      else if (RIGHT>FRONT && RIGHT>LEFT && RIGHT>40) {
        WHEELS("B", 155*SPEED_MULTIPLIER, true, 200);
        SERVO_360_SMOOTH(HEAD, -100, 1000); 
        HEAD.writeMicroseconds(1625);
        WHEELS("R", 155, true, 200);
        delay(100);
        HEAD.writeMicroseconds(1500);

        int P=random(100, 500);
        for(int T=0; T<=P; T++) {
          OBSTACLE_DISTANCE=SONAR.ping_cm();
          if (OBSTACLE_DISTANCE==0) { OBSTACLE_DISTANCE=200; }
          if (OBSTACLE_DISTANCE>40) { WHEELS("F", random(155, 220)*SPEED_MULTIPLIER, true, 10); }
          else {
            if (IMPULSE(0.5)) { PLAYWAV(String("/SOUNDS/"+String(random(1, 25))+".wav").c_str()); }
            if (IMPULSE(0.5)) { WHEELS("BR", 155*SPEED_MULTIPLIER, true, 1500); }
            else if (IMPULSE(0.5)) { WHEELS("BL", 155*SPEED_MULTIPLIER, true, 1500);  }
            else { 
              WHEELS("B", 155*SPEED_MULTIPLIER, true, 700); 
              WHEELS("R", 155*SPEED_MULTIPLIER, true, 300);
              break; 
            }
          }
        }  
      }
      else if (FRONT>LEFT && FRONT>RIGHT && FRONT>40) {
        WHEELS("B", 155*SPEED_MULTIPLIER, true, 200);
        int P=random(100, 500);
        for(int T=0; T<=P; T++) {
          OBSTACLE_DISTANCE=SONAR.ping_cm();
          if (OBSTACLE_DISTANCE==0) { OBSTACLE_DISTANCE=200; }
          if (OBSTACLE_DISTANCE>40) { WHEELS("F", random(155, 220)*SPEED_MULTIPLIER, true, 10); }
          else {
            if (IMPULSE(0.5)) { PLAYWAV(String("/SOUNDS/"+String(random(1, 25))+".wav").c_str()); }
            if (IMPULSE(0.5)) { WHEELS("BR", 155*SPEED_MULTIPLIER, true, 1500); }
            else if (IMPULSE(0.5)) { WHEELS("BL", 155*SPEED_MULTIPLIER, true, 1500);  }
            else { 
              WHEELS("B", 155*SPEED_MULTIPLIER, true, 700); 
              WHEELS("R", 155*SPEED_MULTIPLIER, true, 300);
              break; 
            }
          }
        }  
      } 
      else {
        WHEELS("B", 155*SPEED_MULTIPLIER, true, 700); 
        WHEELS("R", 155*SPEED_MULTIPLIER, true, 300);
      }
    }
   
    if (IMPULSE(0.00075)) { PLAYWAV(String("/SOUNDS/"+String(random(1, 37))+".wav").c_str()); }
    if (IMPULSE(0.00075)) {
      SERVO_360_SMOOTH(HEAD, -100, 1000); 
      OBSTACLE_DISTANCE=SONAR.ping_cm();
      if (OBSTACLE_DISTANCE==0) { OBSTACLE_DISTANCE=200; }
      if (OBSTACLE_DISTANCE>40) { 
        WHEELS("B", 155*SPEED_MULTIPLIER, true, 200);
        HEAD.writeMicroseconds(1625);
        WHEELS("R", 155, true, 200);
        delay(100);
        HEAD.writeMicroseconds(1500);
        int P=random(100, 500);
        for(int T=0; T<=P; T++) {
          OBSTACLE_DISTANCE=SONAR.ping_cm();
          if (OBSTACLE_DISTANCE==0) { OBSTACLE_DISTANCE=200; }
          if (OBSTACLE_DISTANCE>40) { WHEELS("F", random(155, 220)*SPEED_MULTIPLIER, true, 10); }
          else {
            if (IMPULSE(0.5)) { PLAYWAV(String("/SOUNDS/"+String(random(1, 25))+".wav").c_str()); }
            if (IMPULSE(0.5)) { WHEELS("BR", 155*SPEED_MULTIPLIER, true, 1500); }
            else if (IMPULSE(0.5)) { WHEELS("BL", 155*SPEED_MULTIPLIER, true, 1500);  }
            else { 
              WHEELS("B", 155*SPEED_MULTIPLIER, true, 700); 
              WHEELS("R", 155*SPEED_MULTIPLIER, true, 300);
              break; 
            }
          }
        }  
      }
      else { SERVO_360_SMOOTH(HEAD, 125, 700); }
    }
    if (IMPULSE(0.00075)) {
      SERVO_360_SMOOTH(HEAD, 125, 1000); 
      OBSTACLE_DISTANCE=SONAR.ping_cm();
      if (OBSTACLE_DISTANCE==0) { OBSTACLE_DISTANCE=200; }
      if (OBSTACLE_DISTANCE>40) { 
        WHEELS("B", 155*SPEED_MULTIPLIER, true, 200);
        HEAD.writeMicroseconds(1400);
        WHEELS("L", 200, true, 300);
        delay(100);
        HEAD.writeMicroseconds(1500);
        int P=random(100, 500);
        for(int T=0; T<=P; T++) {
          OBSTACLE_DISTANCE=SONAR.ping_cm();
          if (OBSTACLE_DISTANCE==0) { OBSTACLE_DISTANCE=200; }
          if (OBSTACLE_DISTANCE>40) { WHEELS("F", random(155, 220)*SPEED_MULTIPLIER, true, 10); }
          else {
            if (IMPULSE(0.5)) { PLAYWAV(String("/SOUNDS/"+String(random(1, 25))+".wav").c_str()); }
            if (IMPULSE(0.5)) { WHEELS("BR", 155*SPEED_MULTIPLIER, true, 1500); }
            else if (IMPULSE(0.5)) { WHEELS("BL", 155*SPEED_MULTIPLIER, true, 1500);  }
            else { 
              WHEELS("B", 155*SPEED_MULTIPLIER, true, 700); 
              WHEELS("R", 155*SPEED_MULTIPLIER, true, 300);
              break; 
            }
          }
        }  
      }
      else { SERVO_360_SMOOTH(HEAD, -100, 700); }
    }

    if (IMPULSE(0.0005)) { 
      int P=random(100, 200);
      for(int T=0; T<=P; T++) {
        OBSTACLE_DISTANCE=SONAR.ping_cm();
        if (OBSTACLE_DISTANCE==0) { OBSTACLE_DISTANCE=200; }
        if (OBSTACLE_DISTANCE>40) { WHEELS("FL", random(220, 255)*SPEED_MULTIPLIER, true, 10); }
        else {
          if (IMPULSE(0.5)) { PLAYWAV(String("/SOUNDS/"+String(random(1, 25))+".wav").c_str()); }
          if (IMPULSE(0.5)) { WHEELS("BR", 155*SPEED_MULTIPLIER, true, 1500);  }
          else if (IMPULSE(0.5)) { WHEELS("BL", 155*SPEED_MULTIPLIER, true, 1500);  }
          else { 
            WHEELS("B", 155*SPEED_MULTIPLIER, true, 700); 
            WHEELS("BR", 155*SPEED_MULTIPLIER, true, 300);
            break; 
          }
        }
      }  
    }
    if (IMPULSE(0.0005)) { 
      int P=random(100, 200);
      for(int T=0; T<=P; T++) {
        OBSTACLE_DISTANCE=SONAR.ping_cm();
        if (OBSTACLE_DISTANCE==0) { OBSTACLE_DISTANCE=200; }
        if (OBSTACLE_DISTANCE>40) { WHEELS("FR", random(220, 255)*SPEED_MULTIPLIER, true, 10); }
        else {
          if (IMPULSE(0.5)) { PLAYWAV(String("/SOUNDS/"+String(random(1, 25))+".wav").c_str()); }
          if (IMPULSE(0.5)) { WHEELS("BR", 155*SPEED_MULTIPLIER, true, 1500);  }
          else if (IMPULSE(0.5)) { WHEELS("BL", 155*SPEED_MULTIPLIER, true, 1500);  }
          else { 
            WHEELS("B", 155*SPEED_MULTIPLIER, true, 700); 
            WHEELS("L", 155*SPEED_MULTIPLIER, true, 250);
            break; 
          }
        }
      }  
    }
    if (IMPULSE(0.0005)) { 
      WHEELS("B", 155*SPEED_MULTIPLIER, true, 200);
      SERVO_360_SMOOTH(HEAD, 125, 1000); 
      HEAD.writeMicroseconds(1400);
      WHEELS("L", 200, true, 300);
      delay(100);
      HEAD.writeMicroseconds(1500);
    }
    if (IMPULSE(0.0005)) {  
      WHEELS("B", 155*SPEED_MULTIPLIER, true, 200);
      SERVO_360_SMOOTH(HEAD, -100, 1000); 
      HEAD.writeMicroseconds(1625);
      WHEELS("R", 155, true, 200);
      delay(100);
      HEAD.writeMicroseconds(1500);
    }
    if (IMPULSE(0.0005)) { EMOTE(13); }
    
    if (IMPULSE(0.0001)) { EMOTE(7); EMOTE(6); }
    if (IMPULSE(0.0001)) { EMOTE(10); EMOTE(6); }
    if (IMPULSE(0.0001)) { EMOTE(11); EMOTE(12); }
    if (IMPULSE(0.0001)) { EMOTE(8); }
    if (IMPULSE(0.0001)) { EMOTE(9); }
    if (IMPULSE(0.0001)) { EMOTE(11); }
    
    if (IMPULSE(0.0001)) { WHEELS("B", 155*SPEED_MULTIPLIER, true, 1000); }
    if (IMPULSE(0.0001)) { WHEELS("BL", 155*SPEED_MULTIPLIER, true, 1000); }
    if (IMPULSE(0.0001)) { WHEELS("BR", 155*SPEED_MULTIPLIER, true, 1000); }

    if (IMPULSE(0.00007)) { 
      digitalWrite(32, HIGH);
      PLAYWAV(String("/SOUNDS/"+String(random(1, 25))+".wav").c_str());
      digitalWrite(32, LOW);
    }
    if (IMPULSE(0.00007)) { 
      EMOTE(0); 
      PLAYWAV(String("/SOUNDS/"+String(random(1, 25))+".wav").c_str());
      EMOTE(5); 
      EMOTE(0); 
    }
    if (IMPULSE(0.00007)) { 
      EMOTE(0); 
      EMOTE(1);
      PLAYWAV(String("/SOUNDS/"+String(random(1, 25))+".wav").c_str());      EMOTE(3); 
      EMOTE(0); 
    }
    if (IMPULSE(0.00007)) { 
      EMOTE(0);
      EMOTE(1);
      WHEELS("R", 127, true, 1000);
      EMOTE(0);
    }
    if (IMPULSE(0.00007)) { 
      EMOTE(0); 
      EMOTE(1);
      PLAYWAV(String("/SOUNDS/"+String(random(1, 25))+".wav").c_str());
      EMOTE(4); 
      EMOTE(0); 
    }
    if (IMPULSE(0.00007)) { 
      EMOTE(0); 
      EMOTE(1);
      EMOTE(2); 
      EMOTE(0); 
    }
  }
}
