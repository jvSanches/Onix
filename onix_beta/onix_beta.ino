#include "U8glib.h" 
#include "onixhex.c"
#include "IRremote.h"

#define irRec_PIN  16
IRrecv irrecv(irRec_PIN);
decode_results results;

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
#define IR_arm 3
#define start_delay 4700
#define read_IR_timeout 1000

#define strategies 10
// times:
unsigned long ir_recv_time;
long remaining_time = 5000;
unsigned long depressing_time;
unsigned long start_time;
unsigned long time_pressed = 0;
#define ff_period 100
#define ramp_time 200
#define ramp_start 30

//sensors:
#define L_lin_threshold 550
#define R_lin_threshold 550
#define L_dis_threshold 350
#define R_dis_threshold 350
#define close_R_threshold 740
#define close_L_threshold 740

#define samples 15

#define LEFT 1
#define RIGHT 2

////////// IR Control _ LG
#define control_power 875853959
#define control_menu 875845034
#define control_vol_inc 875882519
#define control_vol_dec 875849879
#define control_src 875836364
#define control_number 875852939
#define control_0 875876909
#define control_1 875856509
#define control_2 875840189
#define control_3 875872829
#define control_4 875832029
#define control_5 875864669
#define control_6 875848349
#define control_7 875880989
#define control_8 875827949
#define control_9 875860589
#define control_band 875862629
#define control_disp 875873594

int control_numbers[10] = {control_0, control_1, control_2, control_3, control_4, control_5, control_6, control_7, control_8,control_9};


unsigned long losing_time = 0;
int last_seen = 0;
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

int received_number;
int turning = 0;
unsigned long turn_start;
int pushing = 0;
int ff_state =0;
int ff_speed = 0;
unsigned long ff_time;
unsigned long push_start;

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
      setMotors(100,100);
    }
    setMotors(0,0);
  }
}

// void read_IR_strategy(){
//   irrecv.resume();
//   received_number = -1;
//   while (1){
//     u8g.firstPage();  
//     do 
//     {
//       drawStrategy();
//     } while( u8g.nextPage() );
  
//     if (irrecv.decode(&results)) {
      
//       if (results.value == control_number){
//         if (received_number != -1){
//           strategy = received_number;
//         }
//         break;
//       }else if (getDigit(results.value) != -1){

//         if (received_number == -1){
//           received_number = 0;
//         }
//         received_number = 10 * received_number;
//         received_number = received_number + getDigit(results.value);  
//       }
      
//       wait(250);
//       irrecv.resume(); // Receive the next value
//     }
//   }
// }
void read_IR_strategy(){
  ir_recv_time = millis();
  received_number = getDigit(results.value);
  irrecv.resume();
  while(millis() - ir_recv_time < 700 ){
    u8g.firstPage();  
      do 
      {
        drawStrategy();
      } while( u8g.nextPage() );
    if (irrecv.decode(&results)) {
      ir_recv_time = millis();
      if (getDigit(results.value)!= -1){
        received_number = received_number * 10;
        received_number = received_number + getDigit(results.value);
      }
      irrecv.resume(); // Receive the next value
    }
  }
  strategy = received_number;
}

int getDigit(int data){
  for(int i =0; i < 10; i++){
    if(data == control_numbers[i]){
      return i;
    }
  }
  return -1;
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

void drawStrategy(){
  snprintf (buf, BufSize, "%d", received_number);  
  u8g.setFont(u8g_font_profont22r);
  u8g.drawFrame(48,33,30,30);
  u8g.drawFrame(49,34,28,28);
  u8g.drawRBox(40,36,7,25, 2);
  u8g.drawRBox(79,36,7,25, 2);
  u8g.drawBox(40,21,46,12);
  u8g.drawStr( 10, 15, "Strategy:" );
  if (received_number == -1){

  }else if (received_number < 10){
    u8g.drawStr( 58, 54, buf );
  }else{
    u8g.drawStr( 52, 54, buf );
    
  }

}

void drawLogo(){
  u8g.drawBitmapP(0, 0 , 16, 64, Logoonix);
}
void drawReady(){
  u8g.setFont(u8g_font_profont22r);
  u8g.drawFrame(48,33,30,30);
  u8g.drawFrame(49,34,28,28);
  u8g.drawRBox(40,36,7,25, 2);
  u8g.drawRBox(79,36,7,25, 2);
  u8g.drawBox(40,21,46,12);
  u8g.drawStr( 30, 15, "READY!" );
}
void drawSeconds(int seconds){
  snprintf (buf, BufSize, "%d", seconds);  
  u8g.setFont(u8g_font_profont22r);
  u8g.drawStr( 58, 30, buf );

}
void drawMenu() {
  snprintf (buf, BufSize, "%d", strategy);  
  u8g.setFont(u8g_font_profont22r);
  u8g.drawFrame(48,33,30,30);
  u8g.drawFrame(49,34,28,28);
  u8g.drawRBox(40,36,7,25, 2);
  u8g.drawRBox(79,36,7,25, 2);
  u8g.drawBox(40,21,46,12);
  if (strategy < 10){
    u8g.drawStr( 58, 54, buf );
  }else{
    u8g.drawStr( 52, 54, buf );
    
  }

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
  u8g.setFont(u8g_font_profont22r);
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
  
void crawl(int L_speed, int R_speed, int crawl_period){
  
  if ((millis() / crawl_period) % 2 != 0){
    setMotors(L_speed, -L_speed/5);
  }else{
    setMotors(-R_speed/5, R_speed);
  }
}

void setMotors(int L_speed, int R_speed){
  L_speed = map(L_speed, -100, 100, -255, 255);
  R_speed = map(R_speed, -100, 100, -255, 255);

  if (L_speed > 0){
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }else if ( L_speed < 0){
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }else{
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, HIGH);
  }
  analogWrite(pwm_L_pin, abs(L_speed));

  if (R_speed > 0){
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  }else if ( R_speed < 0){
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }else{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, HIGH);
  }
  analogWrite(pwm_R_pin, abs(R_speed));
}

void setup(void){
  irrecv.enableIRIn();
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
  // while (!Serial){}
  
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
  //// IR Receive
  if (irrecv.decode(&results)) {
    Serial.println(results.value);
    if (mode == menu){
      if (results.value == control_menu){
        mode = IR_arm;
        
        wait(100);
      }else if(getDigit(results.value) != -1){
        read_IR_strategy(); 
        irrecv.resume();

      }else if( results.value == control_vol_inc){
        strategy++;
        if (strategy >strategies){
          strategy =strategies;
        }
      }
      else if (results.value == control_vol_dec){
        strategy--;
        if(strategy<0){
          strategy = 0;
        }
      }

    }else if(mode == IR_arm){

      if (results.value == control_power){
        mode = waiting;
        
        start_time = millis() + start_delay;
      }else if (results.value == control_menu){
        mode = menu;
      } 
    }else if (results.value != control_power){
      mode = menu;
      setMotors(0,0);
      wait(100);
    }
    irrecv.resume(); 
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
        start_time = millis() + start_delay;
      }else{
        strategy ++;
        if (strategy > strategies){
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
        Serial.println("Strat 0");
        break;
      case 9: /// old 0
        /////////////Strategy 0 - Turn to object///////////    
        if(R_dist){
          if(L_dist){
            setMotors(0,0);
          }else{
            setMotors(40,-40);
            last_seen = RIGHT;
            losing_time = millis();
          }
        }else if (L_dist){
          setMotors(-40,40);
          last_seen = LEFT;
          losing_time = millis();
          
          
        }else if (millis() - losing_time < 100){
          if (last_seen == RIGHT){
            setMotors(60,-60);
          }else if (last_seen == LEFT){
            setMotors(-60,60);
          }
        }else{
          setMotors(0,0);
        }  

        break;
        
      case 1:
      if(R_dist){
        if(L_dist){
          if (L_dist > close_L_threshold || R_dist > close_R_threshold){
            setMotors(75,75);
            pushing = 1;
          }else{
          if (pushing){
            setMotors(75,75);
            }else{
              setMotors(0,0);
            }
          }
        }else{
          setMotors(40,-40);
          last_seen = RIGHT;
          losing_time = millis();
          pushing = 0;
        }
      }else if (L_dist){
        setMotors(-40,40);
        last_seen = LEFT;
        losing_time = millis();
        pushing = 0;
        
        
      }else if (millis() - losing_time < 1000){
        pushing = 0;
        if (last_seen == RIGHT){
          setMotors(60,0);
        }else if (last_seen == LEFT){
          setMotors(0,60);
        }
      }else{
        pushing = 0;
        setMotors(0,0);
      }  

      break;
      
      case 2:
        if (turning){
          if (millis()- turn_start > 130){
            setMotors(0,0);
            turning = 0;
          }
        }else{
          if (L_line){
            setMotors(-100,-100);
            wait(100);
            setMotors(80,-80);
            turn_start = millis();
            turning = 1;
          }else if (R_line){
            setMotors(-100,-100);
            wait(100);            
            setMotors(-80,80);
            turn_start = millis();
            turning = 1;
          }else{
            if(R_dist){
              if(L_dist){
                if (pushing) {
                  if (millis() - push_start > 500){
                    crawl(100,100,70);
                  }
                }else{
                  setMotors(40,40);
                  pushing = 1;
                  push_start = millis();
                }
              }else{
                pushing =0;
                setMotors(50,-20);
                last_seen = RIGHT;
                losing_time = millis();
              }
            }else if (L_dist){
              pushing =0;
              setMotors(-20,50);
              last_seen = LEFT;
              losing_time = millis();
              
              
            }else if (millis() - losing_time < 500){
              // pushing =0;
              if (last_seen == RIGHT){
                setMotors(60,-60);
              }else if (last_seen == LEFT){
                setMotors(-60,60);
              }
            }else{
              // pushing =0;
              crawl(50,50, 150);
            }
          }
        }
        break;
     

        case 3:
        
        if(R_dist){
          if(L_dist){
            setMotors(40,40);
          }else{
            setMotors(40,-40);
            last_seen = RIGHT;
            losing_time = millis();
          }
        }else if (L_dist){
          setMotors(-40,40);
          last_seen = LEFT;
          losing_time = millis();
          
          
        }else if (millis() - losing_time < 500){
          if (last_seen == RIGHT){
            setMotors(60,-60);
          }else if (last_seen == LEFT){
            setMotors(-60,60);
          }
        }else{
          setMotors(0,0);
        }  

        break;
      
      case 4:
        if (turning){
          if (millis()- turn_start > 130){
            setMotors(0,0);
            turning = 0;
          }
        }else{
          if (L_line){
            setMotors(-100,-100);
            wait(100);
            setMotors(80,-80);
            turn_start = millis();
            turning = 1;
          }else if (R_line){
            setMotors(-100,-100);
            wait(100);            
            setMotors(-80,80);
            turn_start = millis();
            turning = 1;
          }else{
            setMotors(40,40);
            // crawl( 50, 50, 150);
          }
        }
        break;
      case 5:
        setMotors(-80,0);
        wait(100);
        setMotors(-80,-80):
        wait(100);
        setMotors(40,40);
        wait(50);
        strategy = 0;
        break;
      
      case 6:
        if (L_dist || R_dist){
          strategy = 2;
        }else{
          if (rotating){
            if (rotating == 1){// esquerda
              setMotors(-30,30);
            }else{
              setMotors(30,-30);
            }
            if (millis()- rotating_tick > rotantin_period){
              
            }
          }else{
            setMotors(50,50);
            front_tick


      default :
        setMotors(0,0);
        mode = menu;
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
  }else if (mode == IR_arm){
    u8g.firstPage();  
    do 
    {
      drawReady();
    } while( u8g.nextPage() );
  }else{
    u8g.firstPage();  
    do 
    {
      drawMenu();
    } while( u8g.nextPage() );
  }
}

