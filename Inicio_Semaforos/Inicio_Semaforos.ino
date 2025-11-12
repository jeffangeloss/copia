#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <ESP32Servo.h>

// ========= PROTOTIPOS =========
void applyTraffic();
void applyPauseTraffic();
String hhmmss();
void serialTick();
void updateServoTargetsByState();
static inline String stText();
static inline uint32_t dur();
void servoTick();
static inline int physAngle(int logical, bool reversed, int trim);

// ====== RTC ======
#define USE_DS3231 1  // 1=DS3231, 0=DS1307
#if USE_DS3231
  RTC_DS3231 rtc;
#else
  RTC_DS1307 rtc;
#endif
bool rtc_ok=false;

// ====== Semáforo (tiempos/estado) ======
const int LED_ROJO=16, LED_AMARILLO=17, LED_VERDE=18;
uint32_t T_ROJO=8000, T_VERDE=9000, T_AMARILLO=2000;
enum TL{ROJO,VERDE,AMARILLO}; TL st=ROJO; uint32_t t0=0;

// ====== LCD ======
LiquidCrystal_I2C lcd(0x27,16,2);
String prev0="", prev1="";
static inline void lcdLine(uint8_t r, String s){
  if(s.length()>16)s=s.substring(0,16);
  while(s.length()<16)s+=' ';
  String& p=(r==0?prev0:prev1);
  if(s==p) return;
  lcd.setCursor(0,r); lcd.print(s); p=s;
}

// ====== Servos (19 derecha, 23 izquierda) ======
const int SERVO1_PIN=19, SERVO2_PIN=23;
const bool SERVO1_REVERSED = false;
const bool SERVO2_REVERSED = true;
const int  SERVO1_TRIM = 0;
const int  SERVO2_TRIM = 0;
const int SERVO_NEUTRO=90;
const int SERVO_OPEN = 90;
const int SERVO_CLOSE= 0;

Servo s1, s2;
int pos1=SERVO_NEUTRO, pos2=SERVO_NEUTRO;
int tgt1=SERVO_CLOSE,  tgt2=SERVO_CLOSE;
uint32_t lastServoTick=0;
const uint16_t SERVO_PERIOD_MS=20;
const uint8_t  SERVO_STEP_DEG=2;

// ====== Control de ciclo ======
volatile bool running=true;      // ARRANCA CORRIENDO
volatile bool force_reset=true;  // al iniciar, reinicia ciclo

// ====== Utiles ======
static inline String two(int v){ char b[3]; snprintf(b,sizeof(b),"%02d",v); return String(b); }
String hhmmss(){
  if(!rtc_ok) return "--:--:--";
  DateTime n=rtc.now();
  char b[9]; snprintf(b,9,"%02d:%02d:%02d",n.hour(),n.minute(),n.second());
  return b;
}
static inline String stText(){ return st==ROJO?"RED":(st==VERDE?"GREEN":"YELLOW"); }
static inline uint32_t dur(){ return st==ROJO?T_ROJO:(st==VERDE?T_VERDE:T_AMARILLO); }

// ====== Servo helpers ======
static inline int clamp180(int a){ if(a<0) return 0; if(a>180) return 180; return a; }
static inline int physAngle(int logical, bool reversed, int trim){
  int a = reversed ? (180 - logical) : logical;
  return clamp180(a + trim);
}
void updateServoTargetsByState(){
  if(st==ROJO){       tgt1=SERVO_CLOSE; tgt2=SERVO_CLOSE; }
  else /*VERDE/AMARILLO*/ { tgt1=SERVO_OPEN;  tgt2=SERVO_OPEN;  }
}
void servoTick(){
  if(millis()-lastServoTick < SERVO_PERIOD_MS) return;
  lastServoTick = millis();
  auto stepTo=[&](int& cur, int tgt){
    if(cur < tgt){ cur += SERVO_STEP_DEG; if(cur>tgt) cur=tgt; }
    else if(cur > tgt){ cur -= SERVO_STEP_DEG; if(cur<tgt) cur=tgt; }
    if(cur<0) cur=0; if(cur>180) cur=180;
  };
  stepTo(pos1, tgt1);
  stepTo(pos2, tgt2);
  s1.write( physAngle(pos1, SERVO1_REVERSED, SERVO1_TRIM) );
  s2.write( physAngle(pos2, SERVO2_REVERSED, SERVO2_TRIM) );
}

// ====== Serial: S=YYYY-MM-DDTHH:MM:SS y comandos ======
bool parseISO(const String& iso, DateTime& out){
  if(iso.length()<19) return false;
  int Y=iso.substring(0,4).toInt();
  int M=iso.substring(5,7).toInt();
  int D=iso.substring(8,10).toInt();
  int h=iso.substring(11,13).toInt();
  int m=iso.substring(14,16).toInt();
  int s=iso.substring(17,19).toInt();
  if(Y<2000||M<1||M>12||D<1||D>31||h<0||h>23||m<0||m>59||s<0||s>59) return false;
  out = DateTime(Y,M,D,h,m,s); return true;
}

void serialTick(){
  static String buf="";
  while(Serial.available()){
    char c=Serial.read();
    if(c=='\r'||c=='\n'){
      if(buf.length()){
        String up=buf; up.toUpperCase();

        if(up=="STOP"){ running=false; applyPauseTraffic(); Serial.println(F("[SYS] Pausado")); }
        else if(up=="START" || up=="RUN"){ running=true; force_reset=true; Serial.println(F("[SYS] Iniciado")); }
        else if(up.startsWith("S=") && rtc_ok){
          DateTime dt;
          if(parseISO(buf.substring(2), dt)){ rtc.adjust(dt); Serial.println(F("[RTC] Ajustado OK")); }
          else { Serial.println(F("[RTC] Formato invalido. Ej: S=2025-11-06T21:30:00")); }
        }
        else if(up.startsWith("CFG")){
          // Formato: CFG R=8000,G=9000,Y=2000
          uint32_t r=T_ROJO,g=T_VERDE,y=T_AMARILLO;
          int iR=up.indexOf("R="); if(iR>=0) r=up.substring(iR+2).toInt();
          int iG=up.indexOf("G="); if(iG>=0) g=up.substring(iG+2).toInt();
          int iY=up.indexOf("Y="); if(iY>=0) y=up.substring(iY+2).toInt();
          if(r>0) T_ROJO=r; if(g>0) T_VERDE=g; if(y>0) T_AMARILLO=y;
          force_reset=true;
          Serial.printf("[CFG] R=%u, G=%u, Y=%u (ms)\n",T_ROJO,T_VERDE,T_AMARILLO);
        }
        else {
          Serial.println(F("[CMD] Comando no reconocido"));
        }
      }
      buf="";
    }else{
      if(buf.length()<64) buf+=c;
    }
  }
}

// ======================= Semáforo =======================
void applyTraffic(){
  digitalWrite(LED_ROJO,     st==ROJO && running);
  digitalWrite(LED_AMARILLO, st==AMARILLO && running);
  digitalWrite(LED_VERDE,    st==VERDE && running);
  updateServoTargetsByState();
}
void applyPauseTraffic(){
  digitalWrite(LED_ROJO,LOW);
  digitalWrite(LED_AMARILLO,LOW);
  digitalWrite(LED_VERDE,LOW);
  tgt1 = SERVO_CLOSE; tgt2 = SERVO_CLOSE;
}

// ======================= SETUP/LOOP =====================
void setup(){
  Serial.begin(115200);

  pinMode(LED_ROJO,OUTPUT); pinMode(LED_AMARILLO,OUTPUT); pinMode(LED_VERDE,OUTPUT);

  Wire.begin(21,22);                 // I2C en ESP32
  lcd.init(); lcd.backlight();
  lcdLine(0,"ESP32 Semaforos");
  lcdLine(1,"RTC iniciando...");

  // RTC
  rtc_ok = rtc.begin();
#if USE_DS3231
  if(rtc_ok && rtc.lostPower()) rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
#else
  if(rtc_ok && !rtc.isrunning()) rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
#endif
  lcdLine(1, rtc_ok? "RTC OK":"RTC FAIL");

  // Servos
  delay(200);
  s1.setPeriodHertz(50); s2.setPeriodHertz(50);
  s1.attach(SERVO1_PIN, 500, 2400);
  s2.attach(SERVO2_PIN, 500, 2400);
  pos1 = pos2 = SERVO_NEUTRO;
  s1.write( physAngle(pos1, SERVO1_REVERSED, SERVO1_TRIM) );
  s2.write( physAngle(pos2, SERVO2_REVERSED, SERVO2_TRIM) );

  // Estado inicial -> CORRIENDO
  running=true; force_reset=true; st=ROJO;
  applyTraffic();

  Serial.println(F("Comandos: STOP | START | RUN | S=YYYY-MM-DDTHH:MM:SS | CFG R=ms,G=ms,Y=ms"));
}

void loop(){
  serialTick();
  servoTick();

  if(running){
    if(force_reset){ st = ROJO; t0 = millis(); force_reset=false; }
    if(millis()-t0 >= dur()){
      st = (st==ROJO)?VERDE:(st==VERDE?AMARILLO:ROJO);
      t0 = millis();
    }
    applyTraffic();
  }

  // LCD: muestra estado + hora
  static uint32_t last=0;
  if(millis()-last>=250){
    last=millis();
    if(running){
      lcdLine(0,"Semaf: "+stText());
      lcdLine(1,"Hora:  "+hhmmss());
    }else{
      lcdLine(0,"PAUSADO");
      lcdLine(1, rtc_ok? ("Hora:  "+hhmmss()) : "RTC FAIL");
    }
  }
}
