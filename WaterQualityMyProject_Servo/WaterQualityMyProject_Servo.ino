

/////////////////////servo
#include <Servo.h> 
int servoPin = A2; 
int flushingtime=25000;
Servo Servo1; 
#define pin 9
void setup() {
  // put your setup code here, to run once:
Serial.begin (9600);
  Servo1.attach(servoPin); 
}

void loop() { 
  // put your main code here, to run repeatedly:
  if(digitalRead(pin)==HIGH){
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////servo
  int maxdeg=160;
  
   for(int i=90;i<=maxdeg;i+=3){
   Servo1.write(i); 
   delay(100);
    }
    delay(flushingtime);
//   for(int i=maxdeg;i>=90;i-=3){
   Servo1.write(80); 
   delay(100);
//    }
    } 
}
