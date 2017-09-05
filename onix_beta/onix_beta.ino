#include "U8glib.h"
#include "onixhex.c"
#define line_L_pin
#define line_R_pin
#define dist_L_pin
#define dist_R_pin
#define pwm_L_pin
#define pwm_R_pin
#define IN1
#define IN2
#define IN3
#define IN4
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


int strategy = 0;

U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NO_ACK);
int number = 0;
enum {BufSize=9};
char buf[BufSize];

int mode = 0;

void readSensors(){
//   if (analogRead(line_L_pin) > L_lin_threshold){
//     L_line = 1;
//   }else{
//     l_line = 0;
//   }
//   if (analogRead(line_R_pin) > R_lin_threshold){
//     R_line = 1;
//   }else{
//     R_line = 0;
//   }

//   R_dist = analogRead(dist_R_pin);
  
//   L_dist = analogRead(dist_L_pin);
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
  if (number == 2 || number == 6){
    u8g.drawCircle(20, 30, 14, U8G_DRAW_UPPER_LEFT);
  }
  if (number ==3 || number == 6){
    u8g.drawCircle(105, 30, 14, U8G_DRAW_UPPER_RIGHT);
  }
  if (number ==4 || number == 8){
    u8g.drawBox(38,1,20,10);
  }
  if (number ==5 || number == 8  ){
    u8g.drawBox(68,1,20,10);
  }
}


void wait(int duration){
  long aux_time = millis() + duration;
  while (millis() < aux_time){
  }  
}
  
void setMotors(int L_speed, int R_speed){
//   if (L_speed > 0){
//     digitalWrite(IN3, HIGH);
//     digitalWrite(IN4, LOW);
//   }else if ( L_speed < 0){
//     digitalWrite(IN3, LOW);
//     digitalWrite(IN4, HIGH);
//   }else{
//     digitalWrite(IN3, LOW);
//     digitalWrite(IN4, LOW);
//   }
//   analogWrite(pwm_L_pin, abs(L_speed));

//   if (R_speed > 0){
//     digitalWrite(IN1, HIGH);
//     digitalWrite(IN2, LOW);
//   }else if ( R_speed < 0){
//     digitalWrite(IN1, LOW);
//     digitalWrite(IN2, HIGH);
//   }else{
//     digitalWrite(IN1, LOW);
//     digitalWrite(IN2, LOW);
//   }
//   analogWrite(pwm_R_pin, abs(R_speed));
}

void setup(void){
  // pinMode(line_L_pin, INPUT);
  // pinMode(line_R_pin, INPUT);
  // pinMode(dist_L_pin, INPUT);
  // pinMode(dist_R_pin, INPUT);
  // pinMode(pwm_L_pin, OUTPUT);
  // pinMode(pwm_R_pin, OUTPUT);
  // pinMode(IN1, OUTPUT);
  // pinMode(IN2, OUTPUT);
  // pinMode(IN3, OUTPUT);
  // pinMode(IN4, OUTPUT);
  pinMode(button, INPUT);
  u8g.firstPage();  
  do 
  {
    drawLogo();
  } while( u8g.nextPage() );
  wait(2000);
}

// modos: menu / waiting / playing

void loop(void){
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
        start_time = millis() + 4800;
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
  readSensors;
  if(mode == playing){
    switch (strategy){
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

