/*
 * EcoJelantah RFID Data Sender
 * Sends RFID data to web interface when requested
 */

#define SS_PIN D4  //D2
#define RST_PIN D3 //D1

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

// WiFi credentials
const char* ssid = "ecojelantah";     
const char* password = "12345678";  

// Web endpoint
const char* webEndpoint = "https://smk1kawunganten.github.io/ecojelantah/login.html";

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

String lastCardUID = ""; // Store last scanned card UID

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("Silakan tempelkan kartu RFID...");
}

void loop() {
  // Check if new card present
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    
    // Build UID string
    String cardUID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      cardUID += (mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      cardUID += String(mfrc522.uid.uidByte[i], HEX);
    }
    cardUID.trim();
    cardUID.toUpperCase();
    
    lastCardUID = cardUID; // Store latest UID
    
    // Print card details to Serial Monitor
    Serial.println("\n===================");
    Serial.println("Kartu RFID Terdeteksi!");
    Serial.print("UID Kartu: ");
    Serial.println(cardUID);
    Serial.println("===================\n");
    
    // Send UID to web interface
    if(WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      HTTPClient http;
      
      // Format data to send
      String postData = "uid=" + lastCardUID;
      
      // Use the new API format with WiFiClient
      http.begin(client, webEndpoint);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      
      int httpCode = http.POST(postData);
      
      if(httpCode > 0) {
        String response = http.getString();
        Serial.println("HTTP Response: " + response);
      } else {
        Serial.println("HTTP Error: " + http.errorToString(httpCode));
      }
      
      http.end();
    }
    
    delay(1000); // Delay to prevent multiple reads
  }
}
