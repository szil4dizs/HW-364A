#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>

// ePaper kijelző csatlakozások (ESP8266)
#define CS  15
#define DC  4
#define RES 16
#define BUSY 5

// 2.13" fekete-fehér kijelző
GxEPD2_3C<GxEPD2_213_Z98c, GxEPD2_213_Z98c::HEIGHT> display(GxEPD2_213_Z98c(/*CS=5*/ 15, /*DC=*/ 4, /*RES=*/ 5, /*BUSY=*/ 16));

// NTP idő lekérése
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000); // 1 órás időeltolás (3600 másodperc)

// Napok nevei magyarul
const char* napok[] = {"vas", "hét", "kedd", "sze", "csü", "pén", "szo"};
const char* honapok[] = {"jan.", "febr.", "márc.", "ápr.", "máj.", "jún.", "júl.", "aug.", "szept.", "okt.", "nov.", "dec."};

void setup() {
  Serial.begin(115200);
  Serial.println("\nESP8266 ePaper óra");

  String ssid, password;
  getWiFiCredentials(ssid, password);
  
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Csatlakozás a WiFi-hez");

  int timeout = 10;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(1000);
    Serial.print(".");
    timeout--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi csatlakozva!");
    Serial.print("IP cím: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi csatlakozás sikertelen! Újrapróbálkozás...");
    setup();  // Újraindítja a WiFi beállításokat
  }

  timeClient.begin();
  display.init(115200,true,50,false);
  display.setRotation(1);

  frissitOra();
}

void loop() {
  delay(60000);
  frissitOra();
}

void frissitOra() {
  timeClient.update();
  time_t rawtime = timeClient.getEpochTime();
  struct tm* timeinfo = localtime(&rawtime);

  int ora = timeinfo->tm_hour;
  int perc = timeinfo->tm_min;
  int ev = timeinfo->tm_year + 1900;
  int honap = timeinfo->tm_mon;
  int nap = timeinfo->tm_mday;
  int nap_index = timeinfo->tm_wday;

  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

    // Óra középre
    display.setFont(&FreeMonoBold24pt7b);
    char idoStr[6];
    sprintf(idoStr, "%02d:%02d", ora, perc);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds(idoStr, 0, 0, &tbx, &tby, &tbw, &tbh);
    int x = (display.width() - tbw) / 2;
    int y = (display.height() / 3) + tbh;
    display.setCursor(x, y);
    display.print(idoStr);

    // Dátum alulra
    display.setFont(&FreeMonoBold9pt7b);
    char datumStr[32];
    sprintf(datumStr, "%d.%s%d.,%s", ev, honapok[honap], nap, napok[nap_index]);
    display.setTextColor(nap_index == 0 ? GxEPD_RED : GxEPD_BLACK);
    display.getTextBounds(datumStr, 0, 0, &tbx, &tby, &tbw, &tbh);
    x = (display.width() - tbw) / 2;
    y = (display.height() * 2 / 3) + tbh;
    display.setCursor(x, y);
    display.print(datumStr);

  } while (display.nextPage());

  display.hibernate();
}

void getWiFiCredentials(String &ssid, String &password) {
  Serial.println("Adja meg a WiFi SSID-t:");
  ssid = readSerialInput();
  
  Serial.println("Adja meg a WiFi jelszót:");
  password = readSerialInput();
}

String readSerialInput() {
  String input = "";
  while (true) {
    while (Serial.available() > 0) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') return input;
      input += c;
    }
  }
}
