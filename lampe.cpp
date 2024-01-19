#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>
#include <ctype.h>

#define lamp 18

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
const char *topic1 = "victor.hald97@gmail.com/lampe";
const char *mqtt_username = "victor.hald97@gmail.com";
const char *mqtt_password = "mqtt";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

String payload; // Definerer variablen 'payload' i det globale scope (payload er navnet på besked-variablen)

// Callback function header
void callback(char *topic, byte *byteArrayPayload, unsigned int length);
void connectWiFi();

void setup()
{
  startMillis = millis(); // initial start time

  pinMode(lamp, OUTPUT);
  // Set software serial baud to 115200;
  Serial.begin(115200);
  // connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("FastSpeed2", "30566395");
  wifiMulti.addAP("WiFimodem-9CC1", "tz4ydmjhzy");
  wifiMulti.addAP("Pixel 6", "kombareind");
  // WiFi.scanNetworks will return the number of networks found

  // connectWiFi();
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
  client.subscribe(topic1);
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
  
  if(payload == "on"){
    digitalWrite(lamp, HIGH);
  }
  else if(payload == "off") {
    digitalWrite(lamp, LOW);
  }
  Serial.println("-----------------------");
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
  
}