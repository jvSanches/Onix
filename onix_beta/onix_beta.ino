#include "U8glib.h"
#include "onixhex.c"
#define line_L_pin A2
#define line_R_pin A1
#define dist_L_pin A3
#define dist_R_pin A0
#define pwm_L_pin 9
#define pwm_R_pin 5
#define IN1 4
#define IN2 6
#define IN3 7 
#define IN4 8
#define button 10
// modes:
#define menu 0
#define waiting 1
#define playing 2

// times:
long remaining_time = 5000;
unsigned long depressing_time;
unsigned long start_time;
unsigned long time_pressed = 0;

//sensors:
#define L_lin_threshold 450
#define R_lin_threshold 450
#define L_dis_threshold 270
#define R_dis_threshold 200
#define samples 15


int reading = 0;
int L_D_R[samples] = {}; // [200,200,200,200,200,200,200,200,200,200];
int LDR_C = 0;
int RDR_C = 0;
int R_D_R[samples] = {}; // [200,200,200,200,200,200,200,200,200,200];
int R_dist;
int L_dist;
long R_dist_ = 0;
long L_dist_ = 0;
int L_line;
int R_line;

int first_Run = 1;
int strategy = 0;
int mode = 0;


int turning = 0;
unsigned long turn_start;

U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NO_ACK);
enum {BufSize=9};
char buf[BufSize];

void debug(){
    while (1){
      L_dist = analogRead(dist_L_pin);
      R_dist = analogRead(dist_R_pin);
      L_line = analogRead(line_L_pin);
      R_line = analogRead(line_R_pin);
      Serial.print("Left distance: ");
      Serial.print(L_dist);
      Serial.print("    Right distance: ");
      Serial.print(R_dist);
      Serial.print("    Left line: ");
      Serial.print(L_line);
      Serial.print("    Right line: ");
      Serial.println(R_line);
      u8g.firstPage();  
      do 
      {
        drawDebug();
      } while( u8g.nextPage() );
      
      while (digitalRead(button) == 0){
        setMotors(50,50);
      }
        setMotors(0,0);
      
    }
}
void readSensors(){
  L_line = analogRead(line_L_pin);
  R_line = analogRead(line_R_pin);
  
  reading = analogRead(dist_L_pin); //// Left
  for (int j =0; j< 1+ abs(reading - L_dist_)/50 ; j++){
  L_D_R[LDR_C] = reading;
  LDR_C++;
  if (LDR_C == samples){
      LDR_C = 0;
    }
  }
  L_dist_ = 0;
  for (int k = 0; k< samples; k++){
    L_dist_ = L_dist_ + L_D_R[k];
    
  }
L_dist_ = L_dist_ / samples;

reading = analogRead(dist_R_pin); /// Right 
for (int j =0; j< 1+ abs(reading - R_dist_)/100 ; j++){
  R_D_R[RDR_C] = reading;
  RDR_C++;
  if (RDR_C == samples){
      RDR_C = 0;
    }
  }
  R_dist_ = 0;
  for (int k = 0; k< samples; k++){
    R_dist_ = R_dist_ + R_D_R[k];
  }
R_dist_ = R_dist_ / samples;
  
  if (L_line < L_lin_threshold){
    L_line = 1;
  }else{
    L_line = 0;
  }
  if (R_line < R_lin_threshold){
    R_line = 1;
  }else{
    R_line = 0;
  }

  if(R_dist_ < R_dis_threshold){
    R_dist = 0;
  }else{
    R_dist = R_dist_;
    
  }
  if(L_dist_< L_dis_threshold){
    L_dist = 0;
  }else{
    L_dist = L_dist_;
  }
}

void drawLogo(){
  u8g.drawBitmapP(0, 0 , 16, 64, Logoonix);
}
void drawReady(){
  u8g.setFont(u8g_font_profont22);
  u8g.drawFrame(48,33,30,30);
  u8g.drawFrame(49,34,28,28);
  u8g.drawRBox(40,36,7,25, 2);
  u8g.drawRBox(79,36,7,25, 2);
  u8g.drawBox(40,21,46,12);
  u8g.drawStr( 30, 15, "READY!" );
}
void drawSeconds(int seconds){
  snprintf (buf, BufSize, "%d", seconds);  
  u8g.setFont(u8g_font_profont22);
  u8g.drawStr( 58, 30, buf );

}
void drawMenu() {
  snprintf (buf, BufSize, "%d", strategy);  
  u8g.setFont(u8g_font_profont22);
  u8g.drawFrame(48,33,30,30);
  u8g.drawFrame(49,34,28,28);
  u8g.drawRBox(40,36,7,25, 2);
  u8g.drawRBox(79,36,7,25, 2);
  u8g.drawBox(40,21,46,12);
  u8g.drawStr( 58, 54, buf );
  if (L_line){
    u8g.drawCircle(20, 30, 14, U8G_DRAW_UPPER_LEFT);
  }
  if (R_line){
    u8g.drawCircle(105, 30, 14, U8G_DRAW_UPPER_RIGHT);
  }
  if (L_dist){
    u8g.drawBox(38,1,20,10);
  }
  if (R_dist){
    u8g.drawBox(68,1,20,10);
  }
}
void drawDebug(){
  u8g.setFont(u8g_font_profont22);
  u8g.drawStr( 0, 15, "Lline");
  snprintf (buf, BufSize, "%d", L_line);  
  u8g.drawStr( 80, 15, buf );

  u8g.drawStr( 0, 30, "Rline");
  snprintf (buf, BufSize, "%d", R_line);  
  u8g.drawStr( 80, 30, buf );

  u8g.drawStr( 0, 45, "Ldist");
  snprintf (buf, BufSize, "%d", L_dist);  
  u8g.drawStr( 80, 45, buf );
  
  u8g.drawStr( 0, 60, "Rdist");  
  snprintf (buf, BufSize, "%d", R_dist);  
  u8g.drawStr( 80, 60, buf );
}

void wait(int duration){
  long aux_time = millis() + duration;
  while (millis() < aux_time){
  }  
}
  
void setMotors(int L_speed, int R_speed){
  L_speed = map(L_speed, -100, 100, -255, 255);
  R_speed = map(R_speed, -100, 100, -255, 255);
  if (L_speed > 0){
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }else if ( L_speed < 0){
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }else{
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }
  analogWrite(pwm_L_pin, abs(L_speed));

  if (R_speed > 0){
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  }else if ( R_speed < 0){
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }else{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }
  analogWrite(pwm_R_pin, abs(R_speed));
}

void setup(void){
  pinMode(line_L_pin, INPUT);
  pinMode(line_R_pin, INPUT);
  pinMode(dist_L_pin, INPUT);
  pinMode(dist_R_pin, INPUT);
  pinMode(pwm_L_pin, OUTPUT);
  pinMode(pwm_R_pin, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(button, INPUT);
  Serial.begin(9600);
  
  u8g.firstPage();  
  do 
  {
    drawLogo();
  } while( u8g.nextPage() );
  wait(2000);
}

// modos: menu / waiting / playing

void loop(void){
  if (first_Run){
    first_Run = 0;
    setMotors(0,0);
    if (digitalRead(button) == 0){
      while (digitalRead(button) == 0){}
      wait(100);
      debug();
    }
  }
  if (digitalRead(button) == 0){
    wait(10);
    if (mode == menu){
      depressing_time = millis();
      while (digitalRead(button) == 0){
        time_pressed = millis() - depressing_time;
        if (time_pressed > 1200){
          u8g.firstPage();  
          do 
          {
            drawReady();
          } while( u8g.nextPage() );
        }
      }
      if (millis() - depressing_time > 1200){
        mode = waiting;
        start_time = millis() + 1000; //4800;
      }else{
        strategy ++;
        if (strategy > 9){
          strategy = 0;
        }
      }
    }else{
      mode = menu;
      setMotors(0,0);
      while (digitalRead(button) == 0){
        wait(10);
      }
    }
  }
  readSensors();
  if(mode == playing){
    switch (strategy){
      case 0:
        /////////////Strategy 0 - Turn to object///////////    
        if(R_dist){
          if(L_dist){
            Serial.println("Frente");
            setMotors(60,60);
          }else{
            setMotors(50,0);
          }
        }else if (L_dist){
          setMotors(0,50);
        }else{
          setMotors(0,0);
        }  

        break;
        
      case 1:
      if (turning){
        if (millis()- turn_start > 500){
          setMotors(0,0);
          turning = 0;
        }
      }else{
        if (L_line){
          setMotors(0,-50);
          turn_start = millis();
          turning = 1;
        }else if (R_line){
          setMotors(-50,0);
          turn_start = millis();
          turning = 1;
        }else{
          setMotors( 50, 50);
        }
      }



        break;
      case 2:
        Serial.println("Estrategia 2");
        break;
      default :
        break;

    }
  }else if (mode == waiting){
    remaining_time = start_time - millis();
    if (remaining_time <= 0){
      mode = playing;
      u8g.firstPage();  
      do 
      {
        drawLogo();
      } while( u8g.nextPage() );
    }else{
      u8g.firstPage();  
      do 
      {
        drawSeconds((remaining_time / 1000) +1) ;
      } while( u8g.nextPage() );
    }
  }else{
    u8g.firstPage();  
    do 
    {
      drawMenu();
    } while( u8g.nextPage() );
  }
}

