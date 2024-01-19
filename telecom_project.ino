#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>   // Include the Wi-Fi-Multi library
#include <ESP8266WebServer.h>   // Include the WebServer library
#include <ESP8266mDNS.h>        // Include the mDNS library
#include <PubSubClient.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>

/*
const char* ssid = "Telenor9969lam";
const char* pass = "s85kOCERJ";
WiFiClient client;
*/
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);
time_t prevDisplay  = 0;
time_t t  = 0;


//Wifi
ESP8266WiFiMulti wifiMulti;
const char* ssid = "Jonathans iPhone";
const char* password = "jbs12345";

// MQTT Broker
const char *mqtt_broker = "maqiatto.com";
const char* topic1 = "victor.hald97@gmail.com/temperatur";
const char* topic2 = "victor.hald97@gmail.com/AlarmClock";
const char* topic3 = "victor.hald97@gmail.com/seng";
const char* topic4 = "victor.hald97@gmail.com/lampe";
const char *mqtt_username = "victor.hald97@gmail.com";
const char *mqtt_password = "mqtt";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

String payload;
void callback(char* topic, byte* byteArrayPayload, unsigned int length);
int Pval = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);   //start serial interface
  Serial.setTimeout(50);  //set serial timeout to reduce time for reading
  lcd.init();             //start lcd display
  lcd.backlight();
  pinMode(D5, INPUT_PULLUP);
  //WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid, password);
  
  Serial.println("Connecting Wifi...");
  if(wifiMulti.run() == WL_CONNECTED) {     //connect to WiFi
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);   //sync time lib to ntp time
  }
  //connecting to a mqtt broker
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback);
 while (!client.connected()) {
     String client_id = "esp32-client-";
     client_id += String(WiFi.macAddress());
     Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
     if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
         Serial.println("Public maqiatto mqtt broker connected");
     } else {
         Serial.print("failed with state ");
         Serial.print(client.state());
         delay(2000);
     }
 }
 // publish and subscribe
 client.publish(topic1, "hey"); 
 client.publish(topic2, "hey"); 
 client.publish(topic3, "hey");
 client.publish(topic4, "hey");
 //client.subscribe(topic1); //temp
 client.subscribe(topic2); //alarm
 client.subscribe(topic3); //seng



}


time_t c_hr = 0;
time_t c_min = 0;
time_t c_s = 0;
int alarm = 0;
int button_reset = 0;
float temp = 0;
time_t current;
char str[10]= {0};



void loop() {
  client.loop();            //loop client function for mqtt sub/pub
  int receivedData = 0;
  while (Serial.available()) {
    receivedData = Serial.read();
    if (receivedData == '2') {
      Serial.readBytesUntil('x', str, 10);
    }
    int data_0 = 0;
    data_0 = atoi(str);
    float temperatur = data_0*((5.0/1023.0)/(0.01));  //decode temp. sensor data into celcius
    char strt[5];
    sprintf(strt, "%.2f", temperatur);
    client.publish(topic1, strt);                     //publish temperature data to home assistant
    temp = temperatur;
  for ( int i = 0 ; i < 10; i++){
    str[i] = 0;
  }
  }
  
  AlarmClock();     //call alarm function to update display
  
  if(digitalRead(D5) == 0){   //read pin D5 (button) for reset of alarm
    button_reset = 1;
    delay(100);
    //if(digitalRead(D5) == '0')
    
  }
  
  
}




void callback(char *topic, byte *byteArrayPayload, unsigned int length) { //callback function that recieves a topic payload
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    
   
    
    
    
    //Serial.print("Message:");
    payload = ""; // reset payload string for new data
    for (int i = 0; i < length; i++) {
        payload += (char)byteArrayPayload[i];   //load payload data into string
    }
    //Serial.println(payload);
    //Serial.println();
    //Serial.println("-----------------------");
    for(int i = 0; i <= 255; i++){
        String j = String(i);
        if(payload.indexOf(j) != -1){
            Pval = i;     //convert string to int val range 0:255
        }
        }
    //decode string topic and carry out actions
    if(strncmp(topic, topic3, strlen(topic3)) == 0){  //topic3 = bed control
      Serial.write('0');  //start data packet with '0'-ID
      Serial.print(Pval); //send Pval to UNO that controls bed incline
      Serial.write('x');  //end data packet with 'x'
    }
    else if(strncmp(topic, topic2, strlen(topic2)) == 0){ //topic2 = user alarm input
      Serial.println(payload[3]);
      Serial.println(payload[4]);
      
      char t_hr[2] = {payload[0], payload[1]};  //set alarm compare: hr, min, sec
      c_hr = (time_t)atoi(t_hr);
      char t_min[3] = {payload[3], payload[4]};//{0};
      c_min = (time_t)atoi(t_min);
      char t_s[3] = {payload[6], payload[7]};
      c_s = (time_t)atoi(t_s);
      Serial.print("hr =");
      Serial.println(c_hr);
      Serial.print("min =");
      Serial.println(c_min);
      Serial.print("sec =");
      Serial.println(c_s);
      Serial.print("payload =");
      Serial.println(payload);
    }  
    }




void SyncTime(){
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println("Telenor9969lam");
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password); 
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected."); 
  }
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
}

void AlarmClock(){
  updateClock();  //sync time library with ntp server
    if(hour() == c_hr && minute() == c_min && second() == c_s){ //user alarm value is true
      client.publish(topic4, "on");
      Serial.write('1');    //start buzzer
      Serial.write('1');
      Serial.write('x');
      delay(1000);
    }
    else if(button_reset == 1){
      Serial.write('1');    //user presses button to reset buzzer/alarm
      Serial.write('0');
      Serial.write('x');
      button_reset = 0;
    }
    updateLCD();
  }




void updateClock(){
  if(timeStatus() == timeNotSet){
    time_t noww =time(nullptr);
    setTime(noww);
    adjustTime(3600); //sync time with NTP and adjust 1hr for timezone 

    
  }
}



void updateLCD(){ //update LCD function
  
    if(now() != t){
      t = now();  //compare if current time is equal to previous time
      LCDTimeDisplay();
    }
  }



void LCDTimeDisplay(){  //update LCD display function
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(hour());  //set hr
  lcd.print(":");
  if(minute() < 10){  
    lcd.print("0");
  }
  lcd.print(minute());  //set min
  lcd.print(":");
  if(second() < 10){
    lcd.print("0");
  }
  lcd.print(second());  //set sec
  lcd.setCursor(0,1);
  lcd.print(temp);    //update lcd with temperature sensor val
  lcd.print((char)223);
  lcd.print("C");
}











