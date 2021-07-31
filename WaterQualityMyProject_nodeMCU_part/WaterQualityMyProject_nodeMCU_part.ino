///////////////ustudent___arduino
#include "FirebaseArduino.h"
#include <ESP8266WiFi.h>
#define WIFI_SSID "iPhone Amsa"
#define WIFI_PASSWORD "99999999"
#define FIREBASE_HOST "nodemcu-3bc23-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "jieZdV6BtI7te0DGBGfP7UYpwVQvu5lghccagSxu"
#include <LiquidCrystal.h> // includes the LiquidCrystal Library 
LiquidCrystal lcd(D6, D7, D2, D3, D4, D5); // Creates an LC object. Parameters: (rs, enable, d4, d5, d6, d7) 

void setup() {
  Serial.begin(9600);
  lcd.begin(16,2);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(500);
    }
    Serial.println();
    Serial.print("connected: ");
    Serial.println(WiFi.localIP());
    Firebase.begin(FIREBASE_HOST,FIREBASE_AUTH);
    
    
}

String gettingdata(){
  /////////getting data from nodeMCU
  char mystr[1050]={};
  char str[1050]={};
  bool assign=false;
  Serial.readBytes(mystr,100); //Read the serial data and store in var
////////////Cleaning data
    for(int i=0,j=0;i<100;i++){
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
    Serial.println(mystr);
//    Serial.println();
  //////////cleaning ended
  return str;
}


String a;
char s[500];
long count=1;
void loop() {
  
//    delay(1000);
//    lcd.clear();
//    lcd.print("Amsa");
//    lcd.setCursor(2,1);
//    lcd.print("400"); 
    
    String message="Data:";
    message=gettingdata();
    
    if(message[2]!='\0'){
    lcd.clear();
    lcd.print(message);
    
    Firebase.setString("message",message);
    Firebase.setString("Data1/data"+String(count),"data"+String(count)+": "+message);count=count+1;
    }
    
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());  
  }
  delay(1000);
  
}

void writeString(String stringData) { // Used to serially push out a String with Serial.write()

  for (int i = 0; i < stringData.length(); i++)
  {
    Serial.write(stringData[i]);   // Push each char 1 by 1 on each loop pass
  }

}// end writeString
