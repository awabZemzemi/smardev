#include <Wire.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // Include ArduinoJson library

LiquidCrystal_I2C lcd(0x27, 20, 4);
const char* ssid = "realme 11";
const char* password = "p6j3vts3";
String serverUrl = "http://192.168.82.79:5000/";

#define SS_PIN 5
#define RST_PIN 4
MFRC522 rfid(SS_PIN, RST_PIN);
int rel = 13;
int led = 12;
String code;

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  SPI.begin();
  rfid.PCD_Init();
  pinMode(rel, OUTPUT);
  pinMode(led, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Insert card");
  lcd.setCursor(6, 1);
  lcd.print("Please");
  Serial.println("Ready for exit detection. Type card ID in the Serial Monitor.");
}

void handleRFIDEntry() {
  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (rfid.PICC_ReadCardSerial()) {
    code = "";
    for (byte i = 0; i < 4; i++) {
      code += String(rfid.uid.uidByte[i]);
    }
    Serial.println("Card ID (Entry): " + code);

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      http.begin(serverUrl+"entry");

      http.addHeader("Content-Type", "application/json");

      // Create JSON payload
      StaticJsonDocument<200> jsonDoc;
      jsonDoc["tag"] = code;
      String payload;
      serializeJson(jsonDoc, payload);

      int httpResponseCode = http.POST(payload);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response: " + response);

        // Parse the JSON response
        StaticJsonDocument<300> responseDoc;
        DeserializationError error = deserializeJson(responseDoc, response);

        if (error) {
          Serial.print("JSON parse error: ");
          Serial.println(error.c_str());
        } else {
          String status = responseDoc["status"];
          if (status == "success") {
            String userName = responseDoc["user"];

            lcd.clear();
            lcd.setCursor(3, 0);
            lcd.print("Welcome:");
            lcd.setCursor(3, 1);
            lcd.print(userName);

            Serial.println("Entry detected for: " + userName);

            digitalWrite(rel, HIGH);
            delay(500);
            digitalWrite(rel, LOW);
             delay(500); 
               digitalWrite(rel, HIGH);
            delay(500);
            digitalWrite(rel, LOW);
             delay(500);
                digitalWrite(rel, HIGH);
            delay(500);
            digitalWrite(rel, LOW);
           
          
          } else {
            lcd.clear();
            lcd.setCursor(3, 0);
            lcd.print("Access Denied!");
                  digitalWrite(led, HIGH);
                  delay(500);
                  digitalWrite(led, LOW);
                  delay(500);
                   digitalWrite(led, HIGH);
                  delay(500);
                  digitalWrite(led, LOW);
                  delay(500);
                   digitalWrite(led, HIGH);
                  delay(500);
                  digitalWrite(led, LOW);
                
            Serial.println("Entry denied for card ID: " + code);
         
          }
        }
      } else {
        Serial.printf("HTTP POST failed, error: %d\n", httpResponseCode);
      }
      http.end();
    } else {
      Serial.println("WiFi not connected!");
    }

    // Reset the RFID reader
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Insert card");
    lcd.setCursor(6, 1);
    lcd.print("Please");
  }
}

void handleSerialExit() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // Remove any extra whitespace or newlines

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverUrl+"exit");
      http.addHeader("Content-Type", "application/json");

      // Create JSON payload
      StaticJsonDocument<200> jsonDoc;
      jsonDoc["tag"] = input; // Use the serial input as the tag
      String payload;
      serializeJson(jsonDoc, payload);

      int httpResponseCode = http.POST(payload);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response: " + response);

        // Parse the JSON response
        StaticJsonDocument<300> responseDoc;
        DeserializationError error = deserializeJson(responseDoc, response);

        if (error) {
          Serial.print("JSON parse error: ");
          Serial.println(error.c_str());
        } else {
          String status = responseDoc["status"];
          if (status == "success") {
            String userName = responseDoc["user"];

            lcd.clear();
            lcd.setCursor(3, 0);
            lcd.print("Goodbye:");
            lcd.setCursor(3, 1);
            lcd.print(userName);
              digitalWrite(rel, HIGH);
            delay(500);
            digitalWrite(rel, LOW);
             delay(500); 
               digitalWrite(rel, HIGH);
            delay(500);
            digitalWrite(rel, LOW);
             delay(500);
                digitalWrite(rel, HIGH);
            delay(500);
            digitalWrite(rel, LOW);

            Serial.println("Exit detected for: " + userName);

          } else {
            lcd.clear();
            lcd.setCursor(3, 0);
            lcd.print("Exit Denied!");
                  digitalWrite(led, HIGH);
                  delay(500);
                  digitalWrite(led, LOW);
                  delay(500);
                   digitalWrite(led, HIGH);
                  delay(500);
                  digitalWrite(led, LOW);
                  delay(500);
                   digitalWrite(led, HIGH);
                  delay(500);
                  digitalWrite(led, LOW);
            Serial.println("Exit denied for card ID: " + input);

          }
        }
      } else {
        Serial.printf("HTTP POST failed, error: %d\n", httpResponseCode);
      }
      http.end();
    } else {
      Serial.println("WiFi not connected!");
    }
     rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Insert card");
    lcd.setCursor(6, 1);
    lcd.print("Please");
  }
}
void checkRemoteCommand() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl + "check-door-command"); // Endpoint for checking the door command

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();

      StaticJsonDocument<200> jsonDoc;
      DeserializationError error = deserializeJson(jsonDoc, response);

      if (!error) {
        bool open = jsonDoc["open"];
        if (open) {
          // Execute the door open sequence
         digitalWrite(rel, HIGH);
            delay(500);
            digitalWrite(rel, LOW);
             delay(500); 
               digitalWrite(rel, HIGH);
            delay(500);
            digitalWrite(rel, LOW);
             delay(500);
                digitalWrite(rel, HIGH);
            delay(500);
            digitalWrite(rel, LOW);
          Serial.println("Door opened remotely.");
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Door Opened!");
          delay(2000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Insert card");
          lcd.setCursor(6, 1);
          lcd.print("Please");
        }
      } else {
        Serial.println("Error parsing JSON response.");
      }
    } else {
      Serial.printf("Failed to fetch remote command, error: %d\n", httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
}


void loop() {
  handleRFIDEntry();  // Handle RFID-based entry
  handleSerialExit(); // Handle serial-based exit
  checkRemoteCommand();
  delay(500);
}
