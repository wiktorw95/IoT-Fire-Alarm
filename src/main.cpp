#define BLYNK_TEMPLATE_ID "TMPL4g3TI7-4v"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "TraOIL7mPhCm33YalyL85dcZHH9KQiqT"

#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_NeoPixel.h>
#include <BlynkSimpleEsp32.h>

// -------- BLYNK / WIFI --------
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Phone_1_7635";
char pass[] = "12345687";

// -------- PINY --------
#define LED_PIN     4
#define SDA_PIN     8
#define SCL_PIN     9
#define NUM_LEDS    8
#define BUZZER_PIN  5

// -------- PROGI --------
#define TEMP_ALARM  30.0

Adafruit_BME280 bme;
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
BlynkTimer timer;

// zapamiętanie poprzedniej temperatury
float lastTemp = -100.0;

// ---------------- WIFI ----------------
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
    Serial.println("\n❌ WiFi NIE połączone");
  }
}

// ---------------- LED ----------------
void visualizeTemperature(float t) {
  t = constrain(t, 25, 30);

  int r = map(t, 25, 30, 0, 255);
  int g = map(t, 25, 30, 255, 0);

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, 0));
  }
  strip.show();
}

// ---------------- TEMPERATURA ----------------
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

  // 🔔 POWIADOMIENIE – TYLKO przy PRZEKROCZENIU progu
  if (lastTemp <= TEMP_ALARM && temp > TEMP_ALARM && Blynk.connected()) {
    Blynk.logEvent(
      "alarm_temperatury",
      String("Temperatura przekroczyła 30°C: ") + temp + "°C"
    );
  }

  lastTemp = temp;
}

// ---------------- SETUP ----------------
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
    Serial.println("❌ BME280 nie wykryty");
    while (1);
  }

  connectWiFi();

  // Blynk bez blokowania
  Blynk.config(auth);
  Blynk.connect(5000);

  if (Blynk.connected())
    Serial.println("✅ Blynk połączony");
  else
    Serial.println("❌ Blynk NIE połączony");

  // odczyt co 1 sekundę
  timer.setInterval(1000L, readTemperature);
}

// ---------------- LOOP ----------------
void loop() {
  if (Blynk.connected()) {
    Blynk.run();
  }
  timer.run();
}
