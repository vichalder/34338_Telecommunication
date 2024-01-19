#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>
// #include <Stepper.h>
#include <AccelStepper.h>
#include <ctype.h>
#include <HomeSpan.h>
#include <IRremote.h>
// #include <remote.h>

#define endStop_pin 18

#define IR_RECEIVE_PIN A0

#define IR_BUTTON_ON_OFF 69
#define IR_BUTTON_VOL_UP 70
#define IR_BUTTON_FUNC_STOP 71
#define IR_BUTTON_PLAYBACK 68
#define IR_BUTTON_FOWARD 67
#define IR_BUTTON_ARROW_DOWN 7
#define IR_BUTTON_VOL_DOWN 21
#define IR_BUTTON_ARROW_UP 9
#define IR_BUTTON_EQ 25
#define IR_BUTTON_ST_REPT 13
#define IR_BUTTON_0 22
#define IR_BUTTON_1 12
#define IR_BUTTON_2 24
#define IR_BUTTON_3 94
#define IR_BUTTON_4 8
#define IR_BUTTON_5 28
#define IR_BUTTON_6 90
#define IR_BUTTON_7 66
#define IR_BUTTON_8 82
#define IR_BUTTON_9 74
#define IR_BUTTON_PLAY_PAUSE 64

bool ENDSTOP = LOW;

bool switch1;

// Define pin connections
const int dirPin = 27;
const int stepPin = 25;

// Define motor interface type
#define motorInterfaceType 1

// Creates an instance
AccelStepper stepper(motorInterfaceType, stepPin, dirPin);

int Pval = 0;
String sliderVal = "";
int speed = 200;

int curtainLength = 0;

// WiFi
WiFiMulti wifiMulti;
// WiFi connect timeout per AP. Increase when connecting takes longer.
const uint32_t connectTimeoutMs = 10000;

uint8_t connectStatus;

unsigned long startMillis; // some global variables available anywhere in the program
unsigned long startMillis2;
unsigned long currentMillis;
const unsigned long period = 5000; // the value is a number of milliseconds

// MQTT Broker
const char *mqtt_broker = "maqiatto.com";
const char *topic1 = "victor.hald97@gmail.com/rullegardin";
const char *topic2 = "victor.hald97@gmail.com/switch";
const char *mqtt_username = "victor.hald97@gmail.com";
const char *mqtt_password = "mqtt";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

String payload; // Definerer variablen 'payload' i det globale scope (payload er navnet på besked-variablen)

// Callback function header
void callback(char *topic, byte *byteArrayPayload, unsigned int length);
void endStop();
void connectWiFi();

void setup()
{
  stepper.setMaxSpeed(300);
  startMillis = millis(); // initial start time
  // stepper.setAcceleration(1000);
  pinMode(endStop_pin, INPUT);
  // Set software serial baud to 115200;
  Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN);
  // connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("FastSpeed2", "30566395");
  wifiMulti.addAP("WiFimodem-9CC1", "tz4ydmjhzy");
  wifiMulti.addAP("Pixel 6", "kombareind");
  // WiFi.scanNetworks will return the number of networks found

  // connectWiFi();

  //endStop();
}

void connectWiFi()
{

  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
  {
    Serial.println("no networks found");
  }
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);
    }
  }
  // Connect to Wi-Fi using wifiMulti (connects to the SSID with strongest connection)
  Serial.println("Connecting Wifi...");
  if (wifiMulti.run() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    // connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected())
    {
      String client_id = "esp32-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password))
      {
        Serial.println("Public maqiatto mqtt broker connected");
      }
      else
      {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
      }
    }
  }
  // publish and subscribe
  client.publish(topic1, "Wassup?");

  client.subscribe(topic1);
  client.subscribe(topic2);
}

void endStop()
{
  Serial.println("Endstop running");
  stepper.setSpeed(-100);
  while (ENDSTOP == LOW)
  {
    ENDSTOP = digitalRead(endStop_pin);
    Serial.println(stepper.currentPosition());
    stepper.runSpeed();
    if (ENDSTOP == HIGH)
    {
      stepper.setSpeed(0);
      break;
    }
  }
  curtainLength = abs(stepper.currentPosition());
  Serial.print("Length = ");
  Serial.println(curtainLength);
  Serial.println("End reached. Resetting 0-position.");
  stepper.setCurrentPosition(0);
}

void callback(char *topic, byte *byteArrayPayload, unsigned int length)
{
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  payload = ""; // Nulstil payload variablen så forloopet ikke appender til en allerede eksisterende payload
  for (int i = 0; i < length; i++)
  {
    payload += (char)byteArrayPayload[i];
  }
  Serial.println(payload);
  Serial.println();
  /*
  if(payload == "STOP"){
    endStop();
  }
  else {
  */
  for (int i = 0; i <= 100; i++)
  {
    String j = String(i);
    if (payload.indexOf(j) != -1)
    {
      Pval = i;
    }
  }
  //}
  Serial.println("-----------------------");
}

void irReceiver()
{
  if (IrReceiver.decode())
  {
    IrReceiver.resume();
    int command = IrReceiver.decodedIRData.command;
    switch (command)
    {
    case IR_BUTTON_0:
    {
      Serial.println("Pressed on button 0");
      Pval = 0;
      break;
    }
    case IR_BUTTON_1:
    {
      Serial.println("Pressed on button 1");
      Pval = 11;
      break;
    }
    case IR_BUTTON_2:
    {
      Serial.println("Pressed on button 2");
      Pval = 22;
      break;
    }
    case IR_BUTTON_3:
    {
      Serial.println("Pressed on button 3");
      Pval = 33;
      break;
    }
    case IR_BUTTON_4:
    {
      Serial.println("Pressed on button 4");
      Pval = 44;
      break;
    }
    case IR_BUTTON_5:
    {
      Serial.println("Pressed on button 5");
      Pval = 55;
      break;
    }
    case IR_BUTTON_6:
    {
      Serial.println("Pressed on button 6");
      Pval = 66;
      break;
    }
    case IR_BUTTON_7:
    {
      Serial.println("Pressed on button 7");
      Pval = 77;
      break;
    }
    case IR_BUTTON_8:
    {
      Serial.println("Pressed on button 8");
      Pval = 89;
      break;
    }
    case IR_BUTTON_9:
    {
      Serial.println("Pressed on button 9");
      Pval = 100;
      break;
    }
    case IR_BUTTON_PLAY_PAUSE:
    {
      Serial.println("Pressed on button play/pause");
      if (switch1 == 0)
      {
        client.publish(topic2, "on");
        switch1 = 1;
      }
      else
      {
        client.publish(topic2, "off");
        switch1 = 0;
      }
      break;
    }
    case IR_BUTTON_ARROW_DOWN:
    {
      Serial.println("ROLLING DOWN");
      Pval = 0;
      break;
    }
    case IR_BUTTON_ARROW_UP:
    {
      Serial.println("ROLLING UP");
      Pval = 100;
      break;
    }
    case IR_BUTTON_VOL_UP:
    {
      Serial.println("SPEED INCREASED");
      if (speed < 300)
      {
        speed = speed + 10;
      }
      break;
    }
    case IR_BUTTON_VOL_DOWN:
    {
      Serial.println("SPEED INCREASED");
      if (speed > 100)
      {
        speed = speed - 10;
      }
      break;
    }
    default:
    {
      Serial.println(IrReceiver.decodedIRData.command);
    }
    }
  }
}

void loop()
{
  currentMillis = millis();
  if (currentMillis - startMillis2 >= period)
  {
    connectStatus = WiFi.status();
    startMillis2 = currentMillis;
  }
  if (connectStatus != WL_CONNECTED)
  {
    if (currentMillis - startMillis >= connectTimeoutMs) // test whether the period has elapsed
    {
      connectWiFi();
      startMillis = currentMillis;
    }
  }
  else if (connectStatus == WL_CONNECTED)
  {
    client.loop();
  }
  irReceiver();
  // stepper.moveTo(Pval * curtainLength);
  stepper.moveTo(Pval * 100);
  stepper.setSpeed(speed);
  stepper.runSpeedToPosition();
}