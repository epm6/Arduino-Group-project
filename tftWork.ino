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
//screen libraries
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
//RFID library
#include <MFRC522.h>
//Servo library
#include "Servo.h"
//Wifi
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
//wifi password
const char *ssid = "NOWEQGUZ";
const char *password = "22m5v1dYbRCy";
// NTP settings
const long gmtOffset_sec = 0;         // Your timezone offset in seconds
const int daylightOffset_sec = 3600;  // Daylight saving time offset in seconds
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
int buzzerPin = A3;
int button1 = A1;
int button2 = A2;
int buttonState = 0;
int menuSelect = 0;
//Servo attach
Servo myservo1;
Servo myservo2;
Servo myservo3;
Servo myservo4;
//Master RFID key
byte readCard[4];
String MasterTag = "BDEA5359";
String tagID = "";
MFRC522 mfrc522(SS_PIN, RST_PIN);
//Screen setup
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

int day[7];
int pillHour = 21;
int pillNumber = 0
int currentHour;

void setup() {
  //wifi setup
  WiFi.begin(ssid, password);
  //button and buzzer pins
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  //servo pins
  myservo1.attach(A4);
  myservo2.attach(A5);
  myservo3.attach(D0);
  myservo4.attach(D1);
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
  tft.fillScreen(ILI9341_DARKCYAN);
  timeClient.begin();
  scanToStart();
}
void loop(void) {

  while (getID()) {
    if (tagID == MasterTag) {
      tone(buzzerPin, 1000);
      delay(500);
      noTone(buzzerPin);
      tft.begin();
      mainMenuButton();
      }
    else {
      scanDenied();
      return;
      delay(1000);
      scanToStart();
    }
    while (true) {
      wifiTime();
      buttonState = digitalRead(button1);
      if (buttonState == LOW){
        checkTime();
      }
    }
  }
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
//main menu screen
void mainMenuButton() {
  myservo1.write(0);
  myservo2.write(0);
  myservo3.write(0);
  myservo4.write(0);
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
  tft.setCursor(218, 120);
  tft.setTextColor(ILI9341_DARKCYAN);
  tft.setTextSize(2);
  tft.print("Open");
}
//scan to start screen
void scanToStart() {
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
  tft.setTextColor(ILI9341_WHITE);
  tft.print("Access Denied");
}

void checkTime(){
  timeClient.update();
  currentHour = timeClient.getHours();
  if(currentHour <= pillHour)
  {
    dispense();
    Serial.print(currentHour);
  }
  else
  {
    denied();
    Serial.print(currentHour);
  }
  timeClient.update();
}

int denied() {
  tft.fillScreen(ILI9341_DARKCYAN);
  tft.setCursor(80, 100);
  delay(1000);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.print("Denied");
  myservo1.write(90);
  mainMenuButton();
}

int dispense() {
  tft.fillScreen(ILI9341_DARKCYAN);
  tft.setCursor(80, 100);
  delay(1000);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.print("Dispensing...");
  myservo1.write(90);
  delay(1000);
  mainMenuButton();
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