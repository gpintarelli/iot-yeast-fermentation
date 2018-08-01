//-----------------------------------------------
// Author: Trieu Le
// Email: lethanhtrieuk36@gmail.com
// Publish date: 29-Oct-2017
// Description: This code for demonstration send data from ESP8266 into Google Spreadsheet
// https://www.youtube.com/watch?v=MCofPoglqKo
//-----------------------------------------------
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include<Wire.h>
const int MPU_addr=0x68;  // I2C address of the MPU-6050
const int sda_pin = D3; // definição do pino I2C SDA
const int scl_pin = D4; // definição do pino I2C SCL
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
int16_t out_data;
String readString;
const char* ssid = "IFO404B";
const char* password = "LinhaLIVRE208291";
const int buttonPin = 2;    			// the number of the pushbutton pin
int buttonState;             			// the current reading from the input pin
int lastButtonState = LOW;   			// the previous reading from the input pin
unsigned long lastDebounceTime = 0;  	// the last time the output pin was toggled
unsigned long debounceDelay = 50;    	// the debounce time; increase if the output flickers
const char* host = "script.google.com";
const int httpsPort = 443;
// Use WiFiClientSecure class to create TLS connection
WiFiClientSecure client;
// SHA1 fingerprint of the certificate, don't care with your GAS service
const char* fingerprint = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";
String GAS_ID = "AKfycbyBxMuWZkTrpTRksHOJ--on-Qde3NRqTRug_hJhEN2HP1KL4SrC"; 	// Replace by your GAS service id 
void setup() 
{
  Wire.begin(sda_pin, scl_pin);
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  
  pinMode(buttonPin, INPUT);
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  sendData(113,125);		// Send test data
}

void loop() 
{
//  int reading = digitalRead(buttonPin);
//
//  // check to see if you just pressed the button
//  // (i.e. the input went from LOW to HIGH), and you've waited long enough
//  // since the last press to ignore any noise:
//
//  // If the switch changed, due to noise or pressing:
//  if (reading != lastButtonState) {
//    // reset the debouncing timer
//    lastDebounceTime = millis();
//  }
//
//  if ((millis() - lastDebounceTime) > debounceDelay) {
//    // whatever the reading is at, it's been there for longer than the debounce
//    // delay, so take it as the actual current state:
//
//    // if the button state has changed:
//    if (reading != buttonState) {
//      buttonState = reading;
//
//      // Send test data when button is pressed
//      if (buttonState == HIGH) 
//	  {
//        sendData(113,114);
//      }
//    }
//  }
//
//  // save the reading. Next time through the loop, it'll be the lastButtonState:
//  lastButtonState = reading;


// cagadas aqui

  out_data = tempMPURead();

  sendData(10.5,out_data/340.00+36.53);
  Serial.print(" | Tmp = "); Serial.print(out_data/340.00+36.53);
  delay(5000);

//
  
  while (Serial.available()) {
    char c = Serial.read();  //gets one byte from serial buffer
    readString += c; //makes the string readString
    delay(2);  //slow looping to allow buffer to fill with next character
  }

  if (readString.length() >0) {
    Serial.println(readString);  //so you can see the captured string 
    int n = readString.toInt();  //convert readString into a number

    // auto select appropriate value, copied from someone elses code.
    sendData(analogRead(A0),n);
    readString=""; //empty for next input
  } 
  
}

// Function MPU6050
int tempMPURead()
{
  Wire.beginTransmission(MPU_addr); //https://www.filipeflop.com/blog/acelerometro-com-esp8266-nodemcu/
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  delay(100);
  //Tmp = Tmp/340.00+36.53;
  return Tmp;
}


// Function for Send data into Google Spreadsheet
void sendData(int tem, float hum)
{
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  if (client.verify(fingerprint, host)) {
  Serial.println("certificate matches");
  } else {
  Serial.println("certificate doesn't match");
  }
  String string_temperature =  String(hum, DEC); // eram "tem" onde tem "hum"
  String string_humidity =  String(hum, 3); 
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
  String line = client.readStringUntil('\n');
  if (line == "\r") {
    Serial.println("headers received");
    break;
  }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
  Serial.println("esp8266/Arduino CI successfull!");
  } else {
  Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
} 
