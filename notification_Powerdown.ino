void Line_Notify(String message) ;
void sentMSG();
#include <time.h>
#include <Wire.h>
#include <ESP8266WiFi.h>    //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>     //https://github.com/tzapu/WiFiManager
// set pin numbers:
#define D1 5
#define D2 4
#define D4 2
#define ConfigWiFi_Pin D1  //User Pin ตั้งค่าโหมด config wifi
#define SW D2 //User Pin แจ้งเตือน 
#define LED D4
int state = 0;            //กำหนดค่าไฟดับเป็น 0
#define ESP_AP_NAME "Alarm Config AP"  //ตัวแปรเก็บค่าชื่อ wifi

// Line config
#define LINE_TOKEN "RApG19nhulgdMqg2vANFDZmc6nua2S4qykan41hjWeE"
String message = "%E0%B8%A3%E0%B8%B0%E0%B8%9A%E0%B8%9A%E0%B8%AB%E0%B8%A2%E0%B8%B8%E0%B8%94!!!%20%E0%B9%80%E0%B8%82%E0%B9%89%E0%B8%B2%E0%B8%95%E0%B8%A3%E0%B8%A7%E0%B8%88%E0%B8%AA%E0%B8%AD%E0%B8%9A%E0%B8%94%E0%B9%89%E0%B8%A7%E0%B8%A2%E0%B8%88%E0%B9%89%E0%B8%B2";
String message2 = "%E0%B8%A3%E0%B8%B0%E0%B8%9A%E0%B8%9A%E0%B9%80%E0%B8%94%E0%B8%B4%E0%B8%99%E0%B8%9B%E0%B8%81%E0%B8%95%E0%B8%B4%20%E0%B8%82%E0%B8%AD%E0%B8%9A%E0%B8%84%E0%B8%B8%E0%B8%93%E0%B8%88%E0%B9%89%E0%B8%B2";
//int timezone = 7 * 3600;   //ตั้งค่า timezone ตามเวลาประเทศไทย

int checkLow = 0;
int xCountTime = 0 ;


void setup()
{

  // initialize pin D0 as an output.
  pinMode(ConfigWiFi_Pin, INPUT_PULLUP);
  pinMode(SW, INPUT); //สวิสตรวจเช็คไฟฟ้า
  pinMode(LED, OUTPUT); //LED STATUS
  
//--------------------fix IPAddress -----------------------
  IPAddress local_ip = {10,2,2,118};   
  IPAddress gateway={10,2,1,176};
  IPAddress subnet={255,255,0,0};
  //attachInterrupt(digitalPinToInterrupt(SW), Line, CHANGE);

  //initialize LCD

  Serial.begin(115200);
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  if (digitalRead(ConfigWiFi_Pin) == LOW) // Press button
  {
    //reset saved settings
    wifiManager.resetSettings(); // go to ip 192.168.4.1 to config
    //lcd.setCursor(1, 0); // ไปที่ตัวอักษรที่ 4 บรรทัดที่ 0
    //lcd.print("ConfigWiFi");
  }
  //fetches ssid and password from EEPROM and tries to connect
  //if it does not connect, it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect(ESP_AP_NAME);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    Serial.print(".");
  }
  WiFi.config(local_ip,gateway,subnet);
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED, HIGH);

}

void loop()

{

  state = digitalRead(SW);
  if (state == LOW) {   //ถ้าสถานะไฟดับ
    if (checkLow == 0) {  //ถ้ากำหนดตัวแปร CheckLow มีค่าเป็น 0 

      Serial.println("Print Low");
      //lcd.setCursor(1, 1); // ไปที่ตัวอักษรที่ 4 บรรทัดที่ 0
      //lcd.print("Power Down");
      Line_Notify(message);
      
      checkLow = 1 ;
      
    }
      digitalWrite(LED, LOW);
      delay(500);
      digitalWrite(LED, HIGH);
      delay(500);
    

    xCountTime = xCountTime + 1 ; //ตัวแปร xCountTime เริ่มนับค่าเพิ่มที่ละ 1

    if (xCountTime == 300) {  //ถ้าตัวแปร xCountTime เริ่มนับค่าเป็น 2 นาที

      Serial.println("Print Low 2 Minite");
      Line_Notify(message);
      xCountTime = 0 ;  //reset ค่าเป็น 0
    } 

  } else {

    if (checkLow == 1) {
      Serial.println("Print Hight");
      //lcd.setCursor(1, 1); // ไปที่ตัวอักษรที่ 4 บรรทัดที่ 0
     // lcd.print("Power Normal");
      Line_Notify(message2);
      checkLow = 0 ;
      xCountTime = 0 ;
    }
  }

}
void Line_Notify(String message) {
  WiFiClientSecure client;

  if (!client.connect("notify-api.line.me", 443)) {
    Serial.println("connection failed");
    return;
  }

  String req = "";
  req += "POST /api/notify HTTP/1.1\r\n";
  req += "Host: notify-api.line.me\r\n";
  req += "Authorization: Bearer " + String(LINE_TOKEN) + "\r\n";
  req += "Cache-Control: no-cache\r\n";
  req += "User-Agent: ESP8266\r\n";
  req += "Connection: close\r\n";
  req += "Content-Type: application/x-www-form-urlencoded\r\n";
  req += "Content-Length: " + String(String("message=" + message).length()) + "\r\n";
  req += "\r\n";
  req += "message=" + message;
  // Serial.println(req);
  client.print(req);

  delay(20);

  // Serial.println("-------------");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
    //Serial.println(line);
  }
  // Serial.println("-------------");
}
