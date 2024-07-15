/* -- Core Libs -- */
#include <Arduino.h>            // Base Arduino library
#include <ESP8266WiFi.h>        // ESP8266 core for Arduino
#include <Wire.h>               // I2C library

/* -- External Libs -- */
#include <PubSubClient.h>       // MQTT library
#include <sfm.hpp>              // SFM V1.7 fingerprint library
#include <Adafruit_GFX.h>       // Adafruit graphics library
#include <Adafruit_SSD1306.h>   // Adafruit OLED display library

/* -- Local Libs -- */
#include <credentials.h>        // WiFi and MQTT credentials
#include <screen.h>             // Bitmaps for the OLED display

#define SFM_RX 14               /**< RX pin for SFM fingerprint sensor (Black Wire) */
#define SFM_TX 12               /**< TX pin for SFM fingerprint sensor (Yellow Wire) */
#define SFM_IRQ 0               /**< IRQ pin for SFM fingerprint sensor (Blue Wire) */
#define SFM_VCC 2               /**< VCC pin for SFM fingerprint sensor (Not V_TOUCH!) (Green Wire) */

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

/* ---- Initialize and define the OLED Screen ---- */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/* ---- Initialize and define the fingerprint sensor ---- */
SFM_Module SFM(SFM_VCC, SFM_IRQ, SFM_TX, SFM_RX);
bool lastTouchState = 0;

/* ---- Setup the WiFi and MQTT connection ----
 * For security, the information is stored on a local file within
 * "lib/credentials/credentials.h" and is not pushed to the repository
 * 
 * To include this file, please follow the instructions within "credentials.example.h"
 * All information is added to this file by including "credentials.h"
 */
IPAddress staticIP(192, 168, 2, 6);   // Your desired static IP address
IPAddress gateway(192, 168, 2, 1);    // Your gateway IP address
IPAddress subnet(255, 255, 255, 0);   // Your subnet mask

WiFiClient espClient;
PubSubClient mqttClient(espClient);

uint8_t temp = 0; // used to get recognition return
uint16_t tempUid = 0; // used to get recognized uid

void setup() {
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  SFM.enable();
  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Print the IP address
  Serial.println(WiFi.localIP());

  delay(1000); // Wait for a second before attempting MQTT connection

  mqttClient.setServer(MQTT_SERVER, 1883);
  if (mqttClient.connect("Esp_finger", MQTT_USER, MQTT_PASS)) {
    Serial.println("Connected to MQTT Broker");
  } else {
    Serial.print("MQTT Broker connection failed, rc=");
    Serial.println(mqttClient.state());
  }
  display.clearDisplay();
  display.drawBitmap(0, 0, epd_bitmap_carinha_1, 128, 64, 1);
  display.display();
  
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  display.drawBitmap(0, 0, epd_bitmap_carinha_1, 128, 64, 1);
  display.display();
  ReadFinger();
  mqttClient.loop();
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("Esp_finger", MQTT_USER, MQTT_PASS)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void openDoor() {
  mqttClient.publish("NeRDDOOR/nerd/fingerprint", "open");
  Serial.println("Sent 'open' signal to Node-RED");
  carinha_feliz();
  
}

void ReadFinger(){
  delay(200);
  SFM.setRingColor(SFM_RING_BLUE, SFM_RING_OFF, 1000); // Ring fade from red to black at default period (500ms), creating a breathe effect
  // Here we are going to start recognition, if unlocked, we will change the ring color to green and send a message
  temp = SFM.recognition_1vN(tempUid);
  if(tempUid != 0) {
    Serial.println(tempUid);
    SFM.setRingColor(SFM_RING_GREEN);
    openDoor();
    
  }
}

void carinha_feliz(){
  display.drawBitmap(0, 0, epd_bitmap_carinha_1, 128, 64, 1);
  display.display();
  delay(1000);
  display.clearDisplay();
  display.drawBitmap(0, 0, epd_bitmap_carinha_2, 128, 64, 1);
  display.display();
  delay(150);
  display.clearDisplay();
  display.drawBitmap(0, 0, epd_bitmap_carinha_3, 128, 64, 1);
  display.display();
  delay(150);
  display.clearDisplay();
  display.drawBitmap(0, 0, epd_bitmap_carinha_4, 128, 64, 1);
  display.display();
  delay(6000);
  display.clearDisplay();
  display.drawBitmap(0, 0, epd_bitmap_carinha_5, 128, 64, 1);
  display.display();
  delay(150);
  display.clearDisplay();
  display.drawBitmap(0, 0, epd_bitmap_carinha_6, 128, 64, 1);
  display.display();
  delay(150);
  display.clearDisplay();
}
