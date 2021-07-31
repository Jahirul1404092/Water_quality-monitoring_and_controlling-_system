float turlevelmin=1.0;
float turlevelmax=2.3;

int NTUoffset=0;
bool startclean=false;
bool pumpstatus=false;
int count=0;

////////////////////ph
#define SensorPin A3            //pH meter Analog output to Arduino Analog Input 0
#define Offset -2.51            //deviation compensate
#define LED 13
#define buzzer 6
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    //times of collection
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex=0;  


/////////////////////Gsm
//Create software serial object to communicate with SIM900
#include <SoftwareSerial.h>
SoftwareSerial mySerial(7, 8); //SIM900 Tx & Rx is connected to Arduino #7 & #8

/////////////////////servo
#include <Servo.h> 
int servoPin = A2; 
Servo Servo1; 
#define spin 9
////////////////////cleaning motor
int cleaning_motor = 10;
////////////////////pump motor
#define pump 5

////////////////////temperature
int ThermistorPin = A1;
///////////////////turbidity
#define turbidity A0
int y;
float a;
float b;
float NTU;

/////////////////////////////GPS
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 4, TXPin = 3; //////////////// RX arduino er 3 te ar TX 4 e lagate hobe
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

/////////////////flow
byte statusLed    = 13;

byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;
///////////////////////


char mystr[1050]={};
///////////////////////////////////sonar
int trigPin = 11;    // Trigger
int echoPin = 12;    // Echo  kaje lagbe
long duration, cm, inches;
long volume;
long Time;
int SerFlowRate;
int SerTotalmm;
int used_water_by_sonar=0;
int previous_volume;
int check=0;















String message="";




void setup() {
  Serial.begin (9600);
  ////////////////////////////servo
  Servo1.attach(servoPin);
  pinMode(spin, OUTPUT);
  //////////////////////////pump
  pinMode(pump,OUTPUT);
  /////////////////////////////////Gsm
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  
  //Begin serial communication with Arduino and SIM900
  mySerial.begin(19200);

  Serial.println("Initializing..."); 
  delay(1000);

  mySerial.println("AT"); //Handshaking with SIM900
//  updateSerial();
  
  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
//  updateSerial();
  mySerial.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
//  sendsms();
  updateSerial();

  ////////////flow
  // Set up the status LED line as an output
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  //////////////////////
  
  
  ss.begin(GPSBaud);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(buzzer,INPUT);

}
int c=1;









String gettingdata(){
  /////////getting data from nodeMCU
  char mystr[1050]={};
  char str[1050]={};
  bool assign=false;
  Serial.readBytes(mystr,200); //Read the serial data and store in var
////////////Cleaning data
    for(int i=0,j=0;i<50;i++){
      if(mystr[i]=='y' && mystr[i+1]=='y'){
          assign=true;
          i+=2;
        }
      if(mystr[i]=='z'&& mystr[i+1]=='z'){
          assign=false;
          break;
        }
      if(assign==true){
        str[j]=mystr[i];
          j++;
       }
    }
    Serial.print("data: ");
//    Serial.println(str);
//    Serial.println();
  //////////cleaning ended
  return str;
}

void clean_flush(){
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////cleaning motor
  digitalWrite(cleaning_motor,HIGH);
  delay(10000);
  digitalWrite(cleaning_motor,LOW);
  delay(2000);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////servo
//  int maxdeg=120;
  digitalWrite(spin,HIGH);
  delay(200);
  digitalWrite(spin,LOW);
}















void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
double avergearray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5){   //less than 5, calculated directly statistics
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max;    //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
}

//////////writing to NodeMcu
void writeString(String stringData) { // Used to serially push out a String with Serial.write()

  for (int i = 0; i < stringData.length(); i++)
  {
    Serial.write(stringData[i]);   // Push each char 1 by 1 on each loop pass
  }

}// end writeString




/////////////////////////////gsm
void sendsms(){
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////GPS
  
  int T=0;
  bool flag=false;
  String str="Tank gets dirt";
  while(T<400){
    while (ss.available() > 0){
    gps.encode(ss.read());
    if (gps.location.isUpdated()){
//      Serial.print("Latitude= ");
      float latt= gps.location.lat();
      float longg=gps.location.lng();
//      Serial.print(latt);
//      Serial.print(" Longitude= "); 
      
//      Serial.println(gps.location.lng(),6);
      str=str+" at latt: "+latt+" long: "+longg;
//      Serial.print(str);
//      Serial.println(str);
      delay(200);
//      mySerial.flush();
      mySerial.println("AT+CMGS=\"+8801521579898\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
      updateSerial();
      mySerial.print(str); //text content
      updateSerial();
      mySerial.write(26);
      flag=true;
break;
    }
    }
    T++;
  }
  delay(100);
  if(flag==false){
//  Serial.println(str);
  mySerial.println("AT+CMGS=\"+8801521579898\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();
  mySerial.print(str); //text content
  updateSerial();
  mySerial.write(26);
  }
}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}




void loop() {
// while (ss.available() > 0){
//    gps.encode(ss.read());
//    if (gps.location.isUpdated()){
//      Serial.print("Latitude= "); 
//      Serial.print(gps.location.lat(), 6);
//      Serial.print(" Longitude= "); 
//      Serial.println(gps.location.lng(), 6);
////      gas_value=analogRead(sensor);
////      Serial.println(gas_value);
//    }
//  }

  
//  sendsms();
  message="";
  
  
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////ph
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltageph;
  if(millis()-samplingTime > samplingInterval)
  {
      pHArray[pHArrayIndex++]=analogRead(SensorPin);
      if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
      voltageph = avergearray(pHArray, ArrayLenth)*5.0/1024;
      pHValue = 3.5*voltageph+Offset;
      samplingTime=millis();
  }
  if(millis() - printTime > printInterval)   //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
  {
//  Serial.print("Voltage:");
//        Serial.print(voltage,2);
//        Serial.print("  pH:  ");//
//        Serial.println(pHValue,2);//
//        digitalWrite(LED,digitalRead(LED)^1);
        printTime=millis();
        message=message+"pH: "+pHValue+",";
        if(pHValue>=8 && pHValue<=6){
          startclean=true;
          }
  }
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////temprature
  int Vo;
  float R1 = 10000;
  float logR2, R2, T,C;
  float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T = T - 273.15;
  T = (T * 9.0)/ 5.0 + 32.0; 
  T=T-150;
  C=(T-32)/1.8;

//  Serial.print("Temperature: "); //
//  Serial.print(C);//
//  Serial.println(" C")//; 
  message=message+"Temperature: "+C+"C"+",";
  
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////turbidity
    int sensorValue = analogRead(turbidity);// read the input on analog pin 0:
    float voltage = sensorValue * (5.0 / 1024.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
//    Serial.println(voltage); // print out the value you read:
    //////finding NTU from NTU-voltage curve eqn: NTU= -1120.4v2+5742.3v-4352.9
    b=5742.3*voltage;
    voltage=voltage*voltage;
    a=1120.4*voltage;
    b=b-4352.9;
    NTU=b-a;
    NTU=NTU-NTUoffset;
//    NTU=NTU/1000;
//    Serial.print("tur :");//
//    Serial.println(NTU);//
//    delay(500);

//    Serial.println(NTU);
    message=message+"Turbidity: "+NTU+",";
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////flow
if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
      
    unsigned int frac;
    
    // Print the flow rate for this second in litres / minute
//    Serial.print("Flow rate: ");//
//    Serial.print(int(flowRate));  // Print the integer part of the variable     ////eta kaje lagbe//
//    Serial.print("L/min");//
//    Serial.print("\t");       // Print tab space//
    message=message+"Flow: "+int(flowRate)+"L/min"+",";
    
    // Print the cumulative total of litres flowed since starting
//    Serial.print("Output Liquid Quantity: ");        //
//    Serial.print(totalMilliLitres);
//    Serial.print("mL"); 
//    Serial.print("\t");       // Print tab space
//  Serial.print(totalMilliLitres/1000);//
//  Serial.println("L");//
    message=message+"Uesd: "+totalMilliLitres/1000+"L"+",";

    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////GPS
while (ss.available() > 0){
    gps.encode(ss.read());
    if (gps.location.isUpdated()){
//      Serial.print("Latitude= "); //
//      Serial.print(gps.location.lat(), 6);//
//      Serial.print(" Longitude= "); //
//      Serial.println(gps.location.lng(), 6);//
      message=message+"Location: "+gps.location.lat()+" "+gps.location.lng()+",";
    }
  }
  
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////sonar
   // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
 
  // Convert the time into a distance
  cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  inches = (duration/2) / 70;   // Divide by 74 or multiply by 0.0135
  volume=3.5-(0.17*cm);   //volume=1500-(3.1416*(0.07798*0.07798))*(cm/100)*1000);
  if(volume<0){volume=0;}
//  if(volume<=1){count=0;}

  
//Serial.print("aaaaaaaaaaaaaaaa");Serial.println(cm);
  if(volume<1 && pumpstatus==false){
    digitalWrite(pump,HIGH);
    pumpstatus=true;
    
    }
    else if(volume>=3/*(cm<=5 && cm>=2)*/&& (pumpstatus==true)){
//      Serial.println("Low");
    digitalWrite(pump,LOW);
    pumpstatus=false;
    }
  
//  Serial.print("in ");
//  Serial.println(inches);
//  Serial.print(volume);//
//  Serial.print("volume");//
//  Serial.println();//
//  delay(250);
  
//  Serial.print("vvvvvvvvvvvvv");
//        Serial.print(volume);
  message=message+"volume: "+volume+"L";
  /*
  if(message[52]!='\0'){
  Serial.write("yy");
  writeString(message);
  Serial.write("zz");
  Serial.println();
  }*/
  if(digitalRead(buzzer)==HIGH){
  Serial.print(c);
  Serial.print(" : ");
  int ntu=NTU;
  Serial.print(ntu);
  Serial.print(",");
  Serial.print(pHValue);
  Serial.print(",");
  Serial.println(C);
  c++;
  delay(1000);
  }
  
  /*
  ///////cleaning and controlling
  if(NTU<=turlevelmin ){
    startclean=true;
    count=count+1;
    Serial.println(count);
    }else{
      count=0;
    }
    */
    
    if(startclean==true){
//      if(volume>=2){
      if(count>=1000){
        Serial.println("Dirt!!!!!!!!");
//        Serial.print(volume);
//        Serial.print(count);
//        Serial.println();
    digitalWrite(buzzer,HIGH);
    delay(2000);
    digitalWrite(buzzer,LOW);
    delay(100);
    clean_flush();
    sendsms();
//    digitalWrite(pump,HIGH);
//    delay(100);
//    if(cm<=5){
//    analogWrite(pump,LOW);
//    delay(1000);
//    }
    startclean=false;
    count=0;
//    delay(10000);
    }
//   }
}
    
//    clean_flush();
//    sendsms();
//    digitalWrite(pump,HIGH);
//    delay(1000);
//    digitalWrite(pump,LOW);
    
  
//   String string=gettingdata();
//   Serial.println(string);

}
