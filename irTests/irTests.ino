#include "IRremote.h"
#define RECV_PIN 16
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
int strategy =0 ;
int  received_number =0;
unsigned long ir_recv_time;
int control_numbers[10] = {control_0, control_1, control_2, control_3, control_4, control_5, control_6, control_7, control_8,control_9};


IRrecv irrecv(16);

decode_results results;

int getDigit(int data){
  for(int i =0; i < 10; i++){
    if(data == control_numbers[i]){
      Serial.print("get digit ");
      Serial.println(i);
      
      return i;
    }
  }
  return -1;
}

void setup()
{
  Serial.begin(9600);
 
while(!Serial){}  // In case the interrupt driver crashes on setup, give a clue
  // to the user what's going on.
  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");
}


void read_IR_strategy(){
  ir_recv_time = millis();
  received_number = getDigit(results.value);
  irrecv.resume();
  while(millis() - ir_recv_time < 700 ){
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

void loop() {

  if (irrecv.decode(&results)) {
    if (getDigit(results.value)!= -1){
      
      read_IR_strategy();
      irrecv.resume(); // Receive the next value
    }
  }
  delay(100);
  Serial.println(strategy);
}