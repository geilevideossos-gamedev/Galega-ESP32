#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "webseite.h"

const int LED_PIN = 4;
const int SCHUSS_PIN = 2;
const int LINKS_PIN = 7;
const int TEMP_PIN = 5;

const unsigned long ENTPRELLZEIT = 30;
const unsigned long MESSINTERVALL = 1000;

struct Taster {
  int pin;
  String name;
  bool gedrueckt;
  bool letzterWert;
  unsigned long letzteAenderung;
};

Taster linksTaster = {LINKS_PIN, "links", false, false, 0};
Taster schussTaster = {SCHUSS_PIN, "schuss", false, false, 0};

bool ledAn = false;
float temperatur = -127;
String gesendeteTemperatur = "";
unsigned long letzteMessung = 0;

WebServer server(80);
WebSocketsServer webSocket(81);
OneWire oneWire(TEMP_PIN);
DallasTemperature sensoren(&oneWire);

void anAlleSenden(String text) {
  webSocket.broadcastTXT(text);
}

void anEinenSenden(uint8_t nummer, String text) {
  webSocket.sendTXT(nummer, text);
}

String ledText() {
  if (ledAn) {
    return "led:1";
  }
  return "led:0";
}

String tasterText(Taster &taster) {
  if (taster.gedrueckt) {
    return taster.name + ":1";
  }
  return taster.name + ":0";
}

String temperaturText() {
  return "temp:" + String(temperatur, 1);
}

void ledSetzen(bool an) {
  if (an == ledAn) {
    return;
  }
  ledAn = an;
  digitalWrite(LED_PIN, ledAn ? HIGH : LOW);
  Serial.println(ledText());
  anAlleSenden(ledText());
}

void webSocketEreignis(uint8_t nummer, WStype_t typ, uint8_t *daten, size_t laenge) {
  if (typ == WStype_CONNECTED) {
    Serial.printf("Browser %d verbunden\n", nummer);
    anEinenSenden(nummer, ledText());
    anEinenSenden(nummer, temperaturText());
    anEinenSenden(nummer, tasterText(linksTaster));
    anEinenSenden(nummer, tasterText(schussTaster));
  } else if (typ == WStype_DISCONNECTED) {
    Serial.printf("Browser %d getrennt\n", nummer);
  } else if (typ == WStype_TEXT) {
    String text = String((char *)daten);
    if (text == "led:1") {
      ledSetzen(true);
    } else if (text == "led:0") {
      ledSetzen(false);
    }
  }
}

void tasterPruefen(Taster &taster) {
  bool wert = digitalRead(taster.pin) == LOW;
  if (wert != taster.letzterWert) {
    taster.letzterWert = wert;
    taster.letzteAenderung = millis();
  }
  if (wert != taster.gedrueckt && millis() - taster.letzteAenderung >= ENTPRELLZEIT) {
    taster.gedrueckt = wert;
    Serial.println(tasterText(taster));
    anAlleSenden(tasterText(taster));
  }
}

void temperaturPruefen() {
  if (millis() - letzteMessung < MESSINTERVALL) {
    return;
  }
  letzteMessung = millis();
  temperatur = sensoren.getTempCByIndex(0);
  sensoren.requestTemperatures();

  String text = temperaturText();
  if (text != gesendeteTemperatur) {
    gesendeteTemperatur = text;
    Serial.println(text);
    anAlleSenden(text);
  }
}

void startseite() {
  server.send(200, "text/html", WEBSEITE);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(LINKS_PIN, INPUT_PULLUP);
  pinMode(SCHUSS_PIN, INPUT_PULLUP);

  sensoren.begin();
  sensoren.setWaitForConversion(false);
  sensoren.requestTemperatures();
  letzteMessung = millis();

  WiFi.softAP("ESP32-LED", "12345678");
  Serial.print("Webseite: http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", startseite);
  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEreignis);
}

void loop() {
  server.handleClient();
  webSocket.loop();
  tasterPruefen(linksTaster);
  tasterPruefen(schussTaster);
  temperaturPruefen();
}
