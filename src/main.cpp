#include "secrets.h"

#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_NeoPixel.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;


#define LED_PIN     4
#define SDA_PIN     8
#define SCL_PIN     9
#define NUM_LEDS    8
#define BUZZER_PIN  5

#define TEMP_ALARM  30.0

Adafruit_BME280 bme;
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
BlynkTimer timer;

// zapamiętanie poprzedniej temperatury
float lastTemp = -100.0;

void connectWiFi() {
  Serial.print("Łączenie z WiFi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(500);

  WiFi.begin(ssid, pass);

  unsigned long startAttempt = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi połączone");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n WiFi NIE połączone");
  }
}

void visualizeTemperature(float t) {
  t = constrain(t, 25, 30);

  int r = map(t, 25, 30, 0, 255);
  int g = map(t, 25, 30, 255, 0);

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, 0));
  }
  strip.show();
}

void readTemperature() {
  float temp = bme.readTemperature();
  if (isnan(temp)) return;

  Serial.print("Temperatura: ");
  Serial.println(temp);

  visualizeTemperature(temp);

  // Buzzer
  if (temp > TEMP_ALARM) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  if (lastTemp <= TEMP_ALARM && temp > TEMP_ALARM && Blynk.connected()) {
    Blynk.logEvent(
      "alarm_temperatury",
      String("Temperatura przekroczyła 30°C: ") + temp + "°C"
    );
  }

  lastTemp = temp;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  strip.begin();
  strip.setBrightness(50);
  strip.clear();
  strip.show();

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!bme.begin(0x76)) {
    Serial.println(" BME280 nie wykryty");
    while (1);
  }

  connectWiFi();

  Blynk.config(auth);
  Blynk.connect(5000);

  if (Blynk.connected())
    Serial.println("✅ Blynk połączony");
  else
    Serial.println(" Blynk NIE połączony");


  timer.setInterval(1000L, readTemperature);
}

void loop() {
  if (Blynk.connected()) {
    Blynk.run();
  }
  timer.run();
}
