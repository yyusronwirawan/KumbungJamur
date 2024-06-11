#include <ESP8266WiFi.h>
#include <ThingerESP8266.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define USERNAME "yyusron"
#define DEVICE_ID "ESP8266"
#define DEVICE_CREDENTIAL "!_8XeWagRWr5d#co"

#define SSID "SUBIANTO"
#define SSID_PASSWORD "americano"

#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define LDRPIN A0

#define RELAY_EXHAUST_FAN D5
#define RELAY_WATER_PUMP D6
#define RELAY_GROW_LED D7

ThingerESP8266 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

void setup() {
    Serial.begin(115200);
    dht.begin();
    lcd.begin(); // Initialize the LCD with 16 columns and 2 rows
    lcd.backlight();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Welcome to IoT");
    lcd.setCursor(0, 1);
    lcd.print("Sistem Otomasi");
    delay(2000);
    
    pinMode(RELAY_EXHAUST_FAN, OUTPUT);
    pinMode(RELAY_WATER_PUMP, OUTPUT);
    pinMode(RELAY_GROW_LED, OUTPUT);

    WiFi.begin(SSID, SSID_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi Connected!");

    thing.add_wifi(SSID, SSID_PASSWORD);

    thing["dht22"] >> [](pson &out) {
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        if (isnan(h) || isnan(t)) {
            Serial.println("Sensor DHT22 Gagal Membaca!");
            return;
        }
        out["temperature"] = t;
        out["humidity"] = h;

        if (t > 28) {
            digitalWrite(RELAY_EXHAUST_FAN, HIGH);
            out["exhaust_fan"] = 1;
        } else if (t < 22) {
            digitalWrite(RELAY_GROW_LED, HIGH);
            out["grow_led"] = 1;
        } else {
            digitalWrite(RELAY_EXHAUST_FAN, LOW);
            digitalWrite(RELAY_GROW_LED, LOW);
            out["exhaust_fan"] = 0;
            out["grow_led"] = 0;
        }

        if (h < 80) {
            digitalWrite(RELAY_WATER_PUMP, HIGH);
            out["water_pump"] = 1;
        } else {
            digitalWrite(RELAY_WATER_PUMP, LOW);
            out["water_pump"] = 0;
        }
    };

    thing["ldr"] >> [](pson &out) {
        int ldrValue = analogRead(LDRPIN);
        float lux = map(ldrValue, 0, 1024, 1000, 0); // Ubah mapping nilai lux
        out["light"] = lux;

        if (lux > 300) {
            digitalWrite(RELAY_GROW_LED, LOW);
            out["grow_led"] = 0;
        } else if (lux <= 300) {
            digitalWrite(RELAY_GROW_LED, HIGH);
            out["grow_led"] = 1;
        }
    };

    thing["exhaust_fan"] << digitalPin(RELAY_EXHAUST_FAN);
    thing["water_pump"] << digitalPin(RELAY_WATER_PUMP);
    thing["grow_led"] << digitalPin(RELAY_GROW_LED);

    // Define control resources for Thinger.io
    thing["control"] << [](pson &in) {
        if (in.is_empty()) return;

        if (in["exhaust_fan"]) {
            digitalWrite(RELAY_EXHAUST_FAN, in["exhaust_fan"]);
        }

        if (in["water_pump"]) {
            digitalWrite(RELAY_WATER_PUMP, in["water_pump"]);
        }

        if (in["grow_led"]) {
            digitalWrite(RELAY_GROW_LED, in["grow_led"]);
        }
    };
}

void loop() {
    thing.handle();

    int ldrValue = analogRead(LDRPIN);
    float lux = map(ldrValue, 0, 1024, 1000, 0); // Ubah mapping nilai lux

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    String suhuStatus;
    if (t < 22) {
        suhuStatus = "Air Temp Dingin";
    } else if (t > 28) {
        suhuStatus = "Air Temp Panas";
    } else {
        suhuStatus = "Air Temp Normal";
    }

    String kelembabanStatus;
    if (h < 80) {
        kelembabanStatus = "Air RH Rendah";
    } else {
        kelembabanStatus = "Air RH Normal";
    }

    String cahayaStatus;
    if (lux <= 300) {
        cahayaStatus = "Cahaya Kurang";
    } else {
        cahayaStatus = "Cahaya Cukup";
    }

    Serial.println("-----------------------------------------------------------");
    Serial.print("Suhu Udara : ");
    Serial.print(t);
    Serial.print(" °C                  | ");
    Serial.println(suhuStatus);
    Serial.println("-----------------------------------------------------------");
    Serial.print("Exhaust Fan: ");
    Serial.println(digitalRead(RELAY_EXHAUST_FAN) ? "ON" : "OFF");
    Serial.println("-----------------------------------------------------------");
    Serial.print("Kelembaban Udara : ");
    Serial.print(h);
    Serial.print(" %             | ");
    Serial.println(kelembabanStatus);
    Serial.print("Water Pump: ");
    Serial.println(digitalRead(RELAY_WATER_PUMP) ? "ON" : "OFF");
    Serial.println("-----------------------------------------------------------");
    Serial.print("Intensitas Cahaya : ");
    Serial.print(lux);
    Serial.print(" Lux   | ");
    Serial.println(cahayaStatus);
    Serial.print("Grow Lamp: ");
    Serial.println(digitalRead(RELAY_GROW_LED) ? "ON" : "OFF");
    Serial.println("-----------------------------------------------------------");
    Serial.println("Data Dikirim Ke Thinger.io!");
    Serial.println("-----------------------------------------------------------");

    // Display data on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Suhu: ");
    lcd.print(t);
    lcd.print("C ");
    lcd.setCursor(0, 1);
    lcd.print(suhuStatus);
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Exhaust Fan: ");
    lcd.print(digitalRead(RELAY_EXHAUST_FAN) ? "ON" : "OFF");
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Air RH: ");
    lcd.print(h);
    lcd.print("% ");
    lcd.setCursor(0, 1);
    lcd.print(kelembabanStatus);
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Water Pump: ");
    lcd.print(digitalRead(RELAY_WATER_PUMP) ? "ON" : "OFF");
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cahaya: ");
    lcd.print(lux);
    lcd.print(" Lux ");
    lcd.setCursor(0, 1);
    lcd.print(cahayaStatus);
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Grow Lamp: ");
    lcd.print(digitalRead(RELAY_GROW_LED) ? "ON" : "OFF");
    delay(2000);
}
