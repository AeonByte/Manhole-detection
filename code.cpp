#include <SoftwareSerial.h>
#include <LiquidCrystal.h>


#define wlevel 3
#define buzz   4
#define lm35   A0
#define gas    A5
#define D0     A1


#define config_sms  "This number is configured."
#define user_cut_call  "USER cut the call"
#define USER_not_Answering  "USER not Answering call..."
#define call_establish  "call establish.."
#define disconnecting_call  "disconnecting call"
#define cant_make_CALL  "can't make CALL"
#define  ERROR_str "ERROR"


#define RECEIVINGCALL 1
#define IDLE 2
#define BUSY 3
#define NO_ANSWER 4
#define TALKING 5
#define ERROR 6


#define Pin "pin"
#define ok "OK"
#define ERROR_str "ERROR"
#define Connected "Connected"
#define Connecting "connecting"
#define CLCC "+CLCC:"
#define atCLCC "AT+CLCC"
#define CMGF "AT+CMGF=1"
#define CMGL "+CMGL:"
#define CMGS "AT+CMGS=\""
#define CMGD "AT+CMGD="
#define UNREAD "AT+CMGL=\"REC UNREAD\""
#define CMGDA "AT+CMGD=0,4"
#define CNMI "AT+CNMI=0,0,0,0"
#define CLIP "+CLIP"


String send_data_string;
float temp = 0.0;
boolean gsm_connect = false;
String numtel;
int no_configured = 0;
String indata = "";
String newdata = "";
String num = "";
unsigned long current_time ;
String msg = "";
String inputString = "";
String msg_index;
int index1;
int index2;
String call_number;
String sms_num, sms; // = get_sender_number();


unsigned long previous_Time = 0;
unsigned long current_Time ;
bool tilt;
bool level;
float gas_value;
int temp_limit = 35;
float gas_limit = 60.0;
int a;
int b;


LiquidCrystal lcd(8, 9, 10, 11, 12, 13);


SoftwareSerial esp8266(A3, A4);
SoftwareSerial gsm(7, 6);


void setup() {
  gsm.begin(9600);
  Serial.begin(9600);
  pinMode(wlevel, INPUT);
  pinMode(gas, INPUT);
  pinMode(temp, INPUT);
  pinMode(buzz, OUTPUT);


  lcd.begin(20, 4);
  lcd.clear();
  lcd.print(F(" Manhole Detection"));
  lcd.setCursor(0, 1);
  lcd.print(F("    Using IOT"));
  delay(10000);
  lcd.clear();
  lcd.print(F("Connecting to "));
  lcd.setCursor(0, 1);
  lcd.print(F("GSM"));
    if (connect_gsm())
    {
      lcd.clear();
      lcd.print(F("GSM Connected.."));
      Serial.println(F("GSM Connected.."));
      delay(1000);
    }
    else
    { lcd.clear();
      lcd.print(F("GSM not Connected.."));
      Serial.println(F("not connected"));
      delay(1000);
      //    while (1);
    }
    while (!get_network_status())
    {
      Serial.println("Waiting");
      Serial.println("for Network ");
      delay(100);
    }
    lcd.clear();
    lcd.print(F("network register"));
    lcd.clear();
    lcd.print(F("Kindly call"));
    Serial.println(F("network register"));
    int no_configured = 0;
    while (!no_configured)
    {
      switch (call_status())
      {
        case IDLE: // Nothing is happening
          break;
  
        case RECEIVINGCALL: // Yes! Someone is calling us
          delay(500);
          hangcall();
          delay(10000);
          numtel = get_call_number();
          delay(2000);
          lcd.clear();
          lcd.print(F("Receiving call"));
          lcd.setCursor(0, 1);
          lcd.print(numtel);
          delay(500);
          numtel = numtel;
          sendsms(F("This number configured as USER"), numtel);
          lcd.clear();
          lcd.print(F("SMS sent.."));
          no_configured = 1;
      }
    }
    delay(3000);
  gsm.end();
  delay(1000);
  esp8266.begin(9600);
  lcd.clear();
}


void loop() {
  sensors();
  IOT();
  alert();
  display();
  delay(500);
}


void sensors() {
  for ( int i = 0; i < 100; i++ )
  {
    temp = temp + (5.0 * analogRead(lm35) * 100.0) / 1023;
  }
  temp = temp / 100.0;


  gas_value = ((analogRead(gas) * 100.0) / 1023);
  gas_value = map(gas_value, 0, 75, 0, 100);
  tilt = digitalRead(D0);
  level = digitalRead(wlevel);


  Serial.println("temp:" + String(temp));
  Serial.println("tilt:" + String(tilt));
  Serial.println("flow:" + String(level));
  Serial.println("gas:" + String(gas_value));
  IOT();
}


void display() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gas :");


  lcd.print(gas_value);
  lcd.print("%");


  lcd.setCursor(0, 1);
  lcd.print("Temp :");
  lcd.print(temp);
  lcd.print((char)223);
  lcd.print("C");


  lcd.setCursor(0, 2);
  lcd.print("Tilt :");
  if (tilt == 0) {
    lcd.print("Not Detected");
  }
  lcd.setCursor(0, 3);
  lcd.print("Level=");
  lcd.print("Normal");


}


void alert() {
  Serial.println("in alert");
  if ( temp > temp_limit) {
     sendsms("temperature Alert!!", numtel);
    lcd.clear();
    lcd.print("Temperature Alert");
    Serial.println("Temperature Alert");
    digitalWrite(buzz, HIGH);
    while ( temp > temp_limit)
    {
      sensors();
      delay(2000);
    }
  }


  if ( gas > gas_limit) {
    sendsms("gas Alert", numtel);
    lcd.clear();
    Serial.println("gas Alert");
    lcd.print("Gas Alert");
    //    lcd.print("Temperature Alert");
    while ( gas > gas_limit)
    {
      sensors();
      
      delay(2000);
    }
  }
  if (level == 1) {
    sendsms("Level Alert!!", numtel);
    lcd.clear();
    lcd.print("Level Alert");
    Serial.println("Level Alert");
    while ( level == 1 )
    {
      sensors();
      if (level == 0) {
        break;
      }
      delay(2000);
    }
  }


  if (tilt == 1) {
    sendsms("Tilt Detected!!", numtel);
    lcd.clear();
    //    lcd.print("Temperature Alert");
    lcd.print("Tilt Detected");
    Serial.println("Tilt Detected");
    while ( 1 )
    {
      sensors();
      if (tilt == 0) {
        break;
      }
      delay(2000);
    }
  }
}


void IOT() {
  if (level == 1)
  {
    a = 3;
  }
  else
  {
    a= 0;
  }


  if(tilt == 1)
  {
    b = 3;
  }
  else{
    b = 1;
  }
//  if ( (millis() - prev_millis) >= 10000) {
//    send_data_string = String(temp) + "*" + String(a) + "*" + String(gas) + "*" + String(tilt);
//    send_parameters();
//    Serial.println("uploaded");
//    prev_millis = millis();
//  }
//  current_Time = millis();
  if ((millis()- previous_Time) >= 10000)
  {
    send_data_string = String(temp) + "*" + String(a) + "*" + String(gas_value) + "*" + String(b);
    send_parameters();
    Serial.println("uploaded");
    previous_Time = millis();
  }


}
