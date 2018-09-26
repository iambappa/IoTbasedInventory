/*
Weight, Counter, and Level in 1 code
By- Bappaditya
simplified version- weight+count+level test
*/
//#define D0 16
//#define D1 5

#include <ESP8266WiFi.h>
#include "HX711.h" // Load Cell Amplifier
HX711 cell(D0, D1); // (data, CLK) Amplifier is connected to these pins on the NodeMCU ESP8266 Board

/// for weight
float scale = (8439000-8334400)/500.0f;   // for 10kg. scale
float weight = 0;

/// for counter
int eggnos = 4;                       // max nos of eggs
int eggarray = eggnos-1;              // to use in array
int sensorPin = A0;                   // select the input pin for the potentiometer
int enable[] = {14, 12, 13, 15};      // enable reading sensor A,B,C,D // D5,D6,D7,D8

int sensorValue[] = {0, 0, 0, 0};  // variable to store the value coming from sensor A,B,C,D

int nos[] = {0, 0, 0, 0}; // change sensorValue to 0 or 1
int count = 0;  // sum of 0 or 1's

int countEggs(){
  count = 0;
  for (int j=0; j<=eggarray; j++){
    digitalWrite(enable[j], HIGH); 
    sensorValue[j] = analogRead(sensorPin);
    Serial.println(sensorValue[j]);
    digitalWrite(enable[j], LOW);
    if(sensorValue[j]>785)
      {nos[j]=0;}
      else
      {nos[j]=1;}
    delay(100);
  }
  for (int k=0; k<=eggarray; k++){
  count += nos[k];
  }
  Serial.println(count);
  return count;
}

/// for level and buzzer
// defines pins numbers
const int trigPin = 2;  //D4
const int echoPin = 0;  //D3
const int buzzer = 4;   //D2

// defines variables
long duration;
int distance;

void playBuzzer(){
  for (int i=0; i<500; i++){
  digitalWrite(buzzer, HIGH);
  delayMicroseconds(500);
  digitalWrite(buzzer, LOW);
  delayMicroseconds(500);
  }
//delay(100);
}

/// for wi-fi
const char* ssid     = "xxxxxx";      // your SSID here
const char* password = "xxxxxxxxxx";  // your password here
const int httpPort = 80;
WiFiClient client;

/// for dweet
const char* host = "www.dweet.io";
const char* thing  = "test_bappa1";
const char* thing_content1 = "Rice-gms.";  
const char* thing_content2 = "Eggs-count";
const char* thing_content3 = "water-level-cm.";

/// for IFTTT
const char* key = "oVIZNFWGHtSDqz1xwQUYjujclbgWs0SSFU2729b0Jc8";
const char* host1 = "maker.ifttt.com";
const long sendPeriod2 = 60000L; // 1 minute for IFTTT

void sendToIFTTT(float count)
{
  Serial.print("connecting to ");
  Serial.println(host1);

  if (!client.connect(host1, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  String url = "/trigger/low-egg/with/key/";
  url += key;
  url += "?value1=" + String(count);

  Serial.print("Requesting URL: ");
  Serial.println(url);

  String req = String("GET ") + url + " HTTP/1.1\r\n" +  
               "Host: " + host1 + "\r\n" +
               "Connection: close\r\n\r\n";
  Serial.println(req);
  client.print(req);
    if (client.available())
  {
    Serial.write(client.read());
  }
}

void setup() {
  
  Serial.begin(115200);
  //delay(1000);    // not required
  // declare the enable and ledPin as an OUTPUT:
  for (int i=0; i<=eggarray; i++){
  pinMode(enable[i], OUTPUT);
  }

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
  pinMode(buzzer, OUTPUT);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  delay(3000);
  /// get weight
  weight=(cell.read()-8334400)/scale;   

  /// get counter
  // read the value from sensor A, B, C, D:  
  countEggs();

  /// get level
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH, 50000);

  // Calculating the distance
  distance= duration*0.034/2;
  // Prints the distance on the Serial Monitor
  Serial.println(duration);
  Serial.print("Distance: ");
  Serial.println(distance);
  int level = 10 - distance;
  if (level <=3){
  playBuzzer();
  }
  //delay(500);

  Serial.println("----------------------------------------");
  
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  //WiFiClient client;
  
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/dweet/for/";
  url += thing;
  url += "?";
  url += thing_content1;
  url += "=";
  url += weight;   
  url += "&";
  url += thing_content2;
  url += "=";
  url += count;
  url += "&";
  url += thing_content3;
  url += "=";
  url += level;
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  Serial.println(cell.read());
  Serial.println(weight);
  
  // This will send the request to the server
  String req = String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n";
  Serial.println(req);
  client.print(req);
  if (client.available())
  {
    Serial.write(client.read());
  }

  /// sending to IFTTT
  static long lastReadingTime2 = 0;
  long now2 = millis();
  if (now2 > lastReadingTime2 + sendPeriod2)
  {
    if (count<2){
    //Serial.println(cell.read());    // it was there for weight
    Serial.println(count);
    sendToIFTTT(count);}
    lastReadingTime2 = now2;
  }            
}

