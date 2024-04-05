#include <WiFi.h>
#include <ESP_Mail_Client.h>

#define WIFI_SSID "NOWEQGUZ"
#define WIFI_PASSWORD "22m5v1dYbRCy"

#define SMTP_HOST "smtp.office365.com"
#define SMTP_PORT esp_mail_smtp_port_587

#define AUTHOR_EMAIL "pilldispenser100@outlook.com"
#define AUTHOR_PASSWORD "XingGang$$$777"
#define RECIPIENT_EMAIL "epm6@kent.ac.uk"

SMTPSession smtp;

void smtpCallback(SMTP_Status status);

void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
  }

  smtp.debug(1);

  Session_Config config;
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = F("127.0.0.1");

  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;

  SMTP_Message message;
  message.sender.name = F("Pill Dispenser");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = F("Test");
  message.addRecipient(F("Pill Drop"), RECIPIENT_EMAIL);
  message.text.content = F("Your pill has dropped.\n");
  message.text.transfer_encoding = "base64";
  message.text.charSet = F("utf-8");
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_high;
  message.addHeader(F("Message-ID: <abcde.fghij@gmail.com>"));

  if (!smtp.connect(&config))
  {
    Serial.print("Connection error, Status Code: ");
    Serial.print(smtp.statusCode());
    Serial.print(", Error Code: ");
    Serial.print(smtp.errorCode());
    Serial.print(", Reason: ");
    Serial.println(smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn())
  {
    Serial.println("Not yet logged in.");
  }
  else
  {
    if (smtp.isAuthenticated())
      Serial.println("Successfully logged in.");
    else
      Serial.println("Connected with no Auth.");
  }

  if (!MailClient.sendMail(&smtp, &message))
  {
    Serial.print("Error, Status Code: ");
    Serial.print(smtp.statusCode());
    Serial.print(", Error Code: ");
    Serial.print(smtp.errorCode());
    Serial.print(", Reason: ");
    Serial.println(smtp.errorReason().c_str());
  }
}

void loop() {}
