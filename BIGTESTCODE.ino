//screen colours
#define ILI9341_BLACK 0x0000        ///<   0,   0,   0
#define ILI9341_NAVY 0x000F         ///<   0,   0, 123
#define ILI9341_DARKGREEN 0x03E0    ///<   0, 125,   0
#define ILI9341_DARKCYAN 0x03EF     ///<   0, 125, 123
#define ILI9341_MAROON 0x7800       ///< 123,   0,   0
#define ILI9341_PURPLE 0x780F       ///< 123,   0, 123
#define ILI9341_OLIVE 0x7BE0        ///< 123, 125,   0
#define ILI9341_LIGHTGREY 0xC618    ///< 198, 195, 198
#define ILI9341_DARKGREY 0x7BEF     ///< 123, 125, 123
#define ILI9341_BLUE 0x001F         ///<   0,   0, 255
#define ILI9341_GREEN 0x07E0        ///<   0, 255,   0
#define ILI9341_CYAN 0x07FF         ///<   0, 255, 255
#define ILI9341_RED 0xF800          ///< 255,   0,   0
#define ILI9341_MAGENTA 0xF81F      ///< 255,   0, 255
#define ILI9341_YELLOW 0xFFE0       ///< 255, 255,   0
#define ILI9341_WHITE 0xFFFF        ///< 255, 255, 255
#define ILI9341_ORANGE 0xFD20       ///< 255, 165,   0
#define ILI9341_GREENYELLOW 0xAFE5  ///< 173, 255,  41
#define ILI9341_PINK 0xFC18         ///< 255, 130, 198
#define ILI9341_ARDUINO 0x306D6F    ///< 48, 107, 111
int help = 0;
//screen libraries
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
//web server library
#include "WiFiS3.h"
#include "arduino_secrets.h"
int keyIndex = 0;
int led = LED_BUILTIN;
int status = WL_IDLE_STATUS;
WiFiServer server(80);
//RFID library
#include <MFRC522.h>
//Servo library
#include "Servo.h"

//Alarm timers
unsigned long lastDispenseTime1 = 0;
unsigned long lastDispenseTime2 = 0;
unsigned long lastDispenseTime3 = 0;

//check if pill dropped
int check;
//Servervariables
int time1 = 3;
int amount1 = 10;
int time2 = 3;
int amount2 = 10;
int time3 = 3;
int amount3 = 10;
int dropped;
//Wifi
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
//wifi password
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
// NTP settings
const long gmtOffset_sec = 3600;   // Your timezone offset in seconds
const int daylightOffset_sec = 0;  // Daylight saving time offset in seconds
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", gmtOffset_sec, daylightOffset_sec);
//Screen Pins
#define TFT_CLK 8
#define TFT_MISO -1
#define TFT_MOSI 9
#define TFT_DC 4
#define TFT_CS 7
#define TFT_RST 3
//RFID Pins
#define SS_PIN 10
#define RST_PIN 2
//Pin attach
int timerCounter = 0;
int lidCounter = 0;
int echoPin = 6;
int trigPin = 5;
int forceSensor = A0;
int buzzerPin = A3;
int button1 = A1;
int button2 = A2;
int buttonState1 = 0;
int buttonState2 = 0;
int menuSelect = 0;
int flag = 0;
long duration;
int distance;
//Servo attach
Servo myservo1;
Servo myservo2;
Servo myservo3;
Servo myservo4;
//Master RFID key
byte readCard[4];
String UserTag = "D938C899";
String MasterTag = "BDEA5359";
String tagID = "";
MFRC522 mfrc522(SS_PIN, RST_PIN);
//Screen setup
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

int pillHour = 23;
int pillNumber = 0;
int currentHour;

void setup() {
  //wifi setup
  WiFi.begin(ssid, pass);
  //button and buzzer pins
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(forceSensor, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  myservo4.attach(D0);
  myservo4.write(18);
  //serial start
  Serial.begin(9600);
  //screen start
  SPI.begin();
  //RFID start
  mfrc522.PCD_Init();
  delay(4);
  //screen setup
  tft.begin();
  tft.begin(0x9341);
  tft.setRotation(3);
  timeClient.begin();
  scanToStart();
}
void loop(void) {
  noTone(buzzerPin);
  while (getID()) {
    noTone(buzzerPin);
    if (tagID == MasterTag) {
      flag = 1;
      tone(buzzerPin, 1000);
      delay(500);
      noTone(buzzerPin);
      tft.begin();
      adminMenu();
    } else if (tagID == UserTag) {
      flag = 0;
      tone(buzzerPin, 2000);
      delay(500);
      noTone(buzzerPin);
      tft.begin();
      mainMenuButton();
    } else {
      scanDenied();
      return;
      delay(1000);
      scanToStart();
    }
    while (true) {
      if (flag == 1) {
        wifiTime();
        buttonState2 = digitalRead(button2);
        buttonState1 = digitalRead(button1);
        if (buttonState2 == LOW) {
          serverSetup();
          while(true){
          buttonState1 = digitalRead(button1);
          serverHost();
          if (buttonState1 == LOW){
            break;
          }
          }
        }
        if (buttonState1 == LOW) {
          WiFiClient client = server.available();
          client.stop();
          if (lidCounter == 0) {
            openLid();
          } else {
            closeLid();
            scanToStart();
            delay(1000);
            break;
          }
        }
      } 
      else if (flag == 0) {
        wifiTime();
        buttonState2 = digitalRead(button2);
        buttonState1 = digitalRead(button1);
        if (buttonState2 == LOW) {
          checkTime();
          break;
        }
        if (buttonState1 == LOW) {
          scanToStart();
          delay(1000);
          break;
        }
      } 
      else if (flag == 4) {
        scanToStart();
        delay(1000);
        break;
      } 
    }
  }
  timeClient.update();
  if (time1 || time2 || time3 == timeClient.getHours()){
    if (millis() - lastDispenseTime1 >= 43200000 ) {
    tone(buzzerPin, 2000); // Set the buzzer frequency
    delay(500); // Keep the buzzer on for 500 milliseconds
    noTone(buzzerPin); // Turn off the buzzer
    delay(500); // Wait for 500 milliseconds
  }
}
}

void lowPills(){
  //Email Carer Low Pills
}

//WIFI time
void wifiTime() {
  tft.setCursor(150, 10);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_DARKCYAN);
  timeClient.update();
  tft.print(timeClient.getFormattedTime());
  delay(1000);
  tft.setCursor(150, 10);
  tft.fillRect(150, 10, 95, 15, ILI9341_WHITE);
}

int openLid() {
  myservo4.write(180);
  lidCounter = 1;
  delay(1000);
}

int closeLid() {
  myservo4.write(20);
  lidCounter = 0;
  delay(1000);
}

void adminMenu() {
  flag = 1;
  tft.setRotation(3);
  tft.fillScreen(ILI9341_DARKCYAN);
  tft.setCursor(90, 10);
  tft.setTextColor(ILI9341_DARKCYAN);
  tft.setTextSize(2);
  tft.fillRoundRect(88, 8, 158, 20, 3, ILI9341_WHITE);
  tft.print("TIME:");
  tft.fillRoundRect(30, 80, 100, 100, 10, ILI9341_WHITE);
  tft.fillRoundRect(190, 80, 100, 100, 10, ILI9341_WHITE);
  tft.setCursor(55, 120);
  tft.setTextColor(ILI9341_DARKCYAN);
  tft.setTextSize(2);
  tft.print("Edit");
  tft.setCursor(218, 120);
  tft.setTextColor(ILI9341_DARKCYAN);
  tft.setTextSize(2);
  tft.print("Open");
}

void checkDropped() {
  flag = 3;
  tft.setRotation(3);
  tft.fillScreen(ILI9341_DARKCYAN);
  tft.setCursor(90, 10);
  tft.setTextColor(ILI9341_DARKCYAN);
  tft.setTextSize(2);
  tft.fillRoundRect(88, 8, 158, 20, 3, ILI9341_WHITE);
  tft.print("TIME:");
  tft.setCursor(60, 50);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("Has Pill Dropped?");
  tft.fillRoundRect(30, 80, 100, 100, 10, ILI9341_WHITE);
  tft.fillRoundRect(190, 80, 100, 100, 10, ILI9341_WHITE);
  tft.setCursor(65, 120);
  tft.setTextColor(ILI9341_DARKCYAN);
  tft.setTextSize(2);
  tft.print("No");
  tft.setCursor(218, 120);
  tft.setTextColor(ILI9341_DARKCYAN);
  tft.setTextSize(2);
  tft.print("Yes");
  while (true) {
    help = 10;
    wifiTime();
    buttonState2 = digitalRead(button2);
    buttonState1 = digitalRead(button1);
    if (buttonState1 == LOW) {
      break;
    }
    if (buttonState2 == LOW) {
      if (check == 1){
        checkPill1Dropped();
        check = 0;
        dropped = 1;
        amount1 = amount1 + 1;
      }
      if (check == 2){
        checkPill2Dropped();
        check = 0;
        dropped = 1;
        amount2 = amount2 + 1;
      }
      if (check == 3){
        checkPill3Dropped();
        check = 0;
        dropped = 1;
        amount3 = amount3 + 1;
      }
      break;
    }
  }
}

void checkPill1Dropped(){
  dispensePill1();
  check = 1;
  checkDropped();
}

void checkPill2Dropped(){
  dispensePill2();
  check = 2;
  checkDropped();
}

void checkPill3Dropped(){
  dispensePill3();
  check = 3;
  checkDropped();
}

void checkTaken() {
  tft.fillScreen(ILI9341_DARKCYAN);
  tft.setCursor(60, 50);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("Checking if Taken");
  flag = 2;
  while (true) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;
    if (distance > 3) {
      scanToStart();
      break;
    }
    if (distance < 3) {
      ++timerCounter;
    }
    if (timerCounter >= 5) {
      tone(buzzerPin, 1000);
      delay(250);
      tone(buzzerPin, 2000);
      delay(250);
      tone(buzzerPin, 1000);
      delay(250);
      tone(buzzerPin, 2000);
      delay(250);
      tone(buzzerPin, 1000);
      delay(250);
      tone(buzzerPin, 2000);
      delay(250);
      tone(buzzerPin, 1000);
      delay(250);
      tone(buzzerPin, 2000);
      delay(250);
      if (distance > 5) {
        noTone(buzzerPin);
        delay(1000);
        scanToStart();
        delay(1000);
        break;
      }
    }
    if (timerCounter >= 10) {
      //Email carer
    }
  }
}

//main menu screen
void mainMenuButton() {
  flag = 0;
  tft.setRotation(3);
  tft.fillScreen(ILI9341_DARKCYAN);
  tft.setCursor(90, 10);
  tft.setTextColor(ILI9341_DARKCYAN);
  tft.setTextSize(2);
  tft.fillRoundRect(88, 8, 158, 20, 3, ILI9341_WHITE);
  tft.print("TIME:");
  tft.fillRoundRect(30, 80, 100, 100, 10, ILI9341_WHITE);
  tft.fillRoundRect(190, 80, 100, 100, 10, ILI9341_WHITE);
  tft.setCursor(55, 120);
  tft.setTextColor(ILI9341_DARKCYAN);
  tft.setTextSize(2);
  tft.print("Pill");
  tft.setCursor(210, 120);
  tft.setTextColor(ILI9341_DARKCYAN);
  tft.setTextSize(2);
  tft.print("Cancel");
}
//scan to start screen
void scanToStart() {
  noTone(buzzerPin);
  tft.fillScreen(ILI9341_DARKCYAN);
  tft.setCursor(80, 80);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.print("Scan card");
}
//scan denied screen
void scanDenied() {
  tft.setCursor(80, 100);
  tft.setTextColor(ILI9341_DARKCYAN);
  tft.setTextSize(3);
  tft.print("Scan card");
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.print("Access Denied");
}

void checkTime() {
  if (amount1 <= 5 || amount2 <= 5 || amount3 <= 5){
    lowPills();
  }
  Serial.print("Check Time");
  timeClient.update();
  currentHour = timeClient.getHours();
  if (currentHour >= time1 || currentHour >= time2 || currentHour >= time3){
    if (currentHour >= time1) {
      delay(2000);
      dispensePill1();
      check = 1;
      checkDropped();
      dropped = 1;
      amount1 = amount1 - 1;
      myservo1.detach();
    }
    if (currentHour >= time2) {
      delay(2000);
      dispensePill2();
      check = 2;
      checkDropped();
      dropped = 1;
      amount2 = amount2 - 1;
      myservo2.detach();
    }
    if (currentHour >= time3) {
      delay(2000);
      dispensePill3();
      check = 3;
      checkDropped();
      dropped = 1;
      amount3 = amount3 - 1;
      myservo3.detach();
    }
    if (dropped == 1) {
      checkTaken();
    }
  dropped = 0;
  }
  else{
    Serial.print("Denied");
    denied();
  }
  timeClient.update();
}

int denied() {
  tft.fillScreen(ILI9341_DARKCYAN);
  tft.setCursor(80, 100);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.print("Denied");
  scanToStart();
}

int dispensePill1() {
  tft.fillScreen(ILI9341_DARKCYAN);
  tft.setCursor(80, 100);
  delay(1000);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.print("Dispensing...");
  myservo1.attach(A4);
  myservo1.write(0);
  delay(2000);
  myservo1.write(180);
  lastDispenseTime1 = millis();
}

int dispensePill2() {
  tft.fillScreen(ILI9341_DARKCYAN);
  tft.setCursor(80, 100);
  delay(1000);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.print("Dispensing...");
  myservo2.attach(A5);
  myservo2.write(0);
  delay(2000);
  myservo2.write(180);
  lastDispenseTime2 = millis();
}

int dispensePill3() {
  tft.fillScreen(ILI9341_DARKCYAN);
  tft.setCursor(80, 100);
  delay(1000);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.print("Dispensing...");
  myservo3.attach(D1);
  myservo3.write(0);
  delay(2000);
  myservo3.write(180);
  lastDispenseTime3 = millis();
}

//RFID scan
boolean getID() {
  // Getting ready for Reading PICCs
  //If a new PICC placed to RFID reader continue
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }
  //Since a PICC placed get Serial and continue
  if (!mfrc522.PICC_ReadCardSerial()) {
    return false;
  }
  tagID = "";
  // The MIFARE PICCs that we use have 4 byte UID
  for (uint8_t i = 0; i < 4; i++) {
    //readCard[i] = mfrc522.uid.uidByte[i];
    // Adds the 4 bytes in a single String variable
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA();  // Stop reading
  return true;
}

void serverHost() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("new client");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            client.println("<h1>Set Pill Dispenser 1 Parameters</h1>");
            client.println("<form action='/' method='POST'>");
            client.println("Hour of pill drop: <input type='text' name='time1'><br>");
            client.println("Amount of Pills: <input type='text' name='amount1'><br>");
            client.println("<h1>Set Pill Dispenser 2 Parameters</h1>");
            client.println("Hour of pill drop: <input type='text' name='time2'><br>");
            client.println("Amount of Pills: <input type='text' name='amount2'><br>");
            client.println("<h1>Set Pill Dispenser 3 Parameters</h1>");
            client.println("Hour of pill drop: <input type='text' name='time3'><br>");
            client.println("Amount of Pills: <input type='text' name='amount3'><br>");
            client.println("<input type='submit' value='Submit'>");
            client.println("</form>");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }

        if (currentLine.endsWith("GET")) {
          int time1Index = currentLine.indexOf("time1=") + 6;
          int amount1Index = currentLine.indexOf("amount1=") + 8;
          int time2Index = currentLine.indexOf("time2=") + 6;
          int amount2Index = currentLine.indexOf("amount2=") + 8;
          int time3Index = currentLine.indexOf("time3=") + 6;
          int amount3Index = currentLine.indexOf("amount3=") + 8;

          time1 = getValue(currentLine, time1Index);
          amount1 = getValue(currentLine, amount1Index);
          time2 = getValue(currentLine, time2Index);
          amount2 = getValue(currentLine, amount2Index);
          time3 = getValue(currentLine, time3Index);
          amount3 = getValue(currentLine, amount3Index);

          Serial.print("Time for Pill Drop 1: ");
          Serial.println(time1);
          Serial.print("Amount of Pills 1: ");
          Serial.println(amount1);
          Serial.print("Time for Pill Drop 2: ");
          Serial.println(time2);
          Serial.print("Amount of Pills 2: ");
          Serial.println(amount2);
          Serial.print("Time for Pill Drop 3: ");
          Serial.println(time3);
          Serial.print("Amount of Pills 3: ");
          Serial.println(amount3);
        }
      }
    }
    client.stop();
    Serial.println("client disconnected");
  }
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("Signal Strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

int getValue(String data, int index) {
  String value = "";
  while (isDigit(data[index])) {
    value += data[index];
    index++;
  }
  return value.toInt();
}

void serverSetup() {

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  if (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(1000);
  }
  server.begin();
  printWifiStatus();
}