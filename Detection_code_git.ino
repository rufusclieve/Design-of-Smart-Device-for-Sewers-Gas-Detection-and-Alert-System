#define MQ7 32
#define MQ4 33
#define MQ9 25
#define GREEN 16
#define RED 17


#include <Arduino.h>
#include <WiFi.h>               

#include <Firebase_ESP_Client.h>


#include "addons/TokenHelper.h"

#include "addons/RTDBHelper.h"


#define WIFI_SSID "Redmi11T"
#define WIFI_PASSWORD "12345678"


#define API_KEY "AIzaSyBWcqX3NzrvrO7ubi0S928DyBv2QaYjX0g"


#define DATABASE_URL "https://real-time-gas-monitoring-default-rtdb.asia-southeast1.firebasedatabase.app/" 


FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;     

float analogToPPM(int analogValue, int sensorType) {
  
  
  float mq7PPM = analogValue * 0.2;  
  float mq4PPM = analogValue * 0.2;  
  float mq9PPM = analogValue * 0.2;  
  
  switch(sensorType) {
    case MQ7:
      return mq7PPM;
    case MQ4:
      return mq4PPM;
    case MQ9:
      return mq9PPM;
    default:
      return 0.0;  
  }
}

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(MQ7, INPUT);
  pinMode(MQ4, INPUT);
  pinMode(MQ9, INPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
}


void loop() {
  
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis(); 

    
    int mq7Value = analogRead(MQ7);
    int mq4Value = analogRead(MQ4);
    int mq9Value = analogRead(MQ9);

    
    float mq7PPM = analogToPPM(mq7Value, MQ7);
    float mq4PPM = analogToPPM(mq4Value, MQ4);
    float mq9PPM = analogToPPM(mq9Value, MQ9);


  if (mq7PPM < 400 || mq4PPM < 400 && mq9PPM == 0) {
      digitalWrite(GREEN, HIGH); 
      digitalWrite(RED, LOW);     
    } else {
      digitalWrite(GREEN, LOW);   
      digitalWrite(RED, HIGH);    
    }

    
    
    // String jsonData = "{\"MQ7\": " + String(mq7PPM) + ", \"MQ4\": " + String(mq4PPM) + ", \"MQ9\": " + String(mq9PPM) + "}";

    // Serial.println(jsonData);
    Serial.println(String(mq7PPM));
    Serial.println(String(mq4PPM));
    Serial.println(String(mq9PPM));
    if ((Firebase.RTDB.setString(&fbdo, "SensorData/mq7", String(mq7PPM)))&&(Firebase.RTDB.setString(&fbdo, "SensorData/mq4", String(mq4PPM)))&&(Firebase.RTDB.setString(&fbdo, "SensorData/mq9", String(mq9PPM)))) {
      Serial.println("Sensor data sent to Firebase successfully");
    } else {
      Serial.println("Failed to send sensor data to Firebase");
      Serial.println("Reason: " + fbdo.errorReason());
    }
  }
}