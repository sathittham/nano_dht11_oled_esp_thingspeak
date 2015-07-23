#include "DHT.h"
#include <Wire.h>
#include "U8glib.h"
#include <SoftwareSerial.h>

int ch_pd = 9; 

//IoT
#define SSID "Jannii_HotSpot";
#define PASS "jannii1234";

#define IP "api.thingspeak.com";
//#define API_KEY = "1TZDS75MNL72VOUH";
#define fieldId1 = "1";
#define fieldId2 = "2";

String GET = "GET /update?key=1TZDS75MNL72VOUH&";    // put here your thingspeak key instead of [THINGSPEAK_KEY]
String GET1 = "field1=";
String GET2 = "field2=";


//OLED
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);  //OLED

//DHT
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 11 

DHT dht(DHTPIN, DHTTYPE);

#define DEBUG FALSE //comment out to remove debug msgs

//*-- Hardware Serial
#define _baudrate 115200

//*-- Software Serial
//
#define _rxpin      10
#define _txpin      11
SoftwareSerial esp8266( _rxpin, _txpin ); // RX, TX

boolean connectWiFi(){
  esp8266.println("AT+CWMODE=1");
  delay(2000);
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  esp8266.println(cmd);
  delay(5000);
  if(esp8266.find("OK")){
    Serial.println("OK");
    return true;
  }else{
    Serial.println("KO");
    return false;
  }
}


void setup() {
  pinMode(ch_pd, OUTPUT); 
  digitalWrite(ch_pd, LOW); 
  delay(500); 
  digitalWrite(ch_pd, HIGH);
  
  Serial.begin( _baudrate );
  esp8266.begin( _baudrate );
  delay(10);
  
  // dht begin
  dht.begin();
  Serial.println("DHT11 test!");
  
  
  //WiFi
  esp8266.println("AT");
  delay(5000);
  if(esp8266.find("OK")){
    connectWiFi();
  }
  

// flip screen, if required
  // u8g.setRot180();
  
  // set SPI backup if required
  //u8g.setHardwareBackup(u8g_backup_avr_spi);

  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
  
}

void loop() {
  delay(1000);
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  
  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT");
  } else {
    Serial.print("Humidity: "); 
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: "); 
    Serial.print(t);
    Serial.println(" *C");
  }
  //Send Data to Thingspeak
  String _temp = String(t);
  String _hum = String(h);
  updateTS(_temp,_hum);
  
  // picture loop
  u8g.firstPage();  
  do {
    // show temp
  showTemp(t,h);
  } while( u8g.nextPage() );
  
  // rebuild the picture after some delay
  delay(3000);
  
}


void showTemp(float temp,float hum) {
  // graphic commands to redraw the complete screen should be placed here  
  u8g.setFont(u8g_font_unifont);
  //u8g.setFont(u8g_font_osb21);
  u8g.drawStr( 0, 22, "My Temp Station!");
  u8g.drawStr( 0, 42, "Temp = ");
  u8g.setPrintPos(60,42);
  u8g.print(String(temp) +" c");

  u8g.drawStr( 0, 62, "Hum = ");
  u8g.setPrintPos(60,62);
  u8g.print(String(hum) +" %");
}

//----- update the  Thingspeak string with 2 values
void updateTS( String T, String H)
{
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  esp8266.println(cmd);
  delay(2000);
  if(esp8266.find("Error")){
    Serial.print("Error1");
    return;
  }
  cmd = GET + GET1;
  cmd += T;
  cmd += GET2;
  cmd += H;
  cmd += "\r\n";
  Serial.print(cmd);
  esp8266.print("AT+CIPSEND=");
  esp8266.println(cmd.length());
  if(esp8266.find(">")){
    esp8266.print(cmd);
  }else{
    esp8266.println("AT+CIPCLOSE");
  }
}




