#include "WiFiS3.h"
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int keyIndex = 0;
int led =  LED_BUILTIN;
int status = WL_IDLE_STATUS;
WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
  server.begin();
  printWifiStatus();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("new client");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            client.println("<h1>Set Pill Dispenser Parameters</h1>");
            client.println("<form action='/' method='POST'>");
            client.println("Hour of pill drop: <input type='text' name='time'><br>");
            client.println("Amount of Pills: <input type='text' name='amount'><br>");
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
          if (currentLine.indexOf("time=") != -1) {
            int timeIndex = currentLine.indexOf("time=") + 5;
            int timeEndIndex = currentLine.indexOf("&", timeIndex);
            String timeValue = currentLine.substring(timeIndex, timeEndIndex);
            Serial.print("Time for Pill Drop: ");
            Serial.println(timeValue);
          }
          if (currentLine.indexOf("amount=") != -1) {
            int amountIndex = currentLine.indexOf("amount=") + 7;
            String amountValue = currentLine.substring(amountIndex);
            Serial.print("Amount of Pills: ");
            Serial.println(amountValue);
          }
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
