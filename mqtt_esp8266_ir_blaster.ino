// Set configuration options for pins, WiFi, and MQTT in the following file:
#include "config.h"

// https://github.com/bblanchon/ArduinoJson
#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// http://pubsubclient.knolleary.net/
#include <PubSubClient.h>

#include <IRremoteESP8266.h>

const int irLedPin = CONFIG_PIN_IR_LED;

const char* ssid = CONFIG_WIFI_SSID;
const char* password = CONFIG_WIFI_PASS;

const char* mqtt_server = CONFIG_MQTT_HOST;
const char* mqtt_username = CONFIG_MQTT_USER;
const char* mqtt_password = CONFIG_MQTT_PASS;
const char* client_id = CONFIG_MQTT_CLIENT_ID;

// Topics
const char* light_state_topic = CONFIG_MQTT_TOPIC_STATE;
const char* light_set_topic = CONFIG_MQTT_TOPIC_SET;

const int BUFFER_SIZE = JSON_OBJECT_SIZE(10);

char irEncoding = UNKNOWN;
unsigned long irCode;
const int txPin = 2; // On-board blue LED

WiFiClient espClient;
PubSubClient client(espClient);

IRsend irsend(16); //an IR led is connected to GPIO pin 0

void setup() {
  irsend.begin();
  pinMode(txPin, OUTPUT);
  digitalWrite(txPin, HIGH); // Turn off the on-board LED
  
  Serial.begin(115200);
  setup_wifi();

  setupOTA();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setupOTA() {
  // OTA setup
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start OTA");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd OTA");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(txPin, LOW);
    delay(500);
    digitalWrite(txPin, HIGH);
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

  /*
  SAMPLE PAYLOAD:
    {
      "brightness": 120,
      "color": {
        "r": 255,
        "g": 100,
        "b": 100
      },
      "flash": 2,
      "transition": 5,
      "state": "ON"
    }
  */
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  if (!processJson(message)) {
    return;
  }

  Serial.print("Sending IR encoding: ");
  Serial.print(irEncoding, DEC);
  Serial.print(", code: ");
  Serial.println(irCode, HEX);
  
  digitalWrite(txPin, LOW);
  irsend.send(irEncoding, irCode, 32);
  digitalWrite(txPin, HIGH);
//  if(digitalRead(txPin))
//    digitalWrite(txPin, LOW);
//  else
//    digitalWrite(txPin, HIGH);
  
}

bool processJson(char* message) {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(message);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return false;
  }
Serial.println("Check encoding");
// get IR encoding type
  if (root.containsKey("encoding")) {
    if (strcmp(root["encoding"], "unused") == 0) { irEncoding = UNUSED; }
    else if (strcmp(root["encoding"], "rc5") == 0) { irEncoding = RC5; }
    else if (strcmp(root["encoding"], "rc6") == 0) { irEncoding = RC6; }
    else if (strcmp(root["encoding"], "nec") == 0) { irEncoding = NEC; Serial.println("NEC encoding"); }
    else if (strcmp(root["encoding"], "sony") == 0) { irEncoding = SONY; }
    else if (strcmp(root["encoding"], "panasonic") == 0) { irEncoding = PANASONIC; }
    else if (strcmp(root["encoding"], "jvc") == 0) { irEncoding = JVC; }
    else if (strcmp(root["encoding"], "samsung") == 0) { irEncoding = SAMSUNG; }
    else if (strcmp(root["encoding"], "whynter") == 0) { irEncoding = WHYNTER; }
    else if (strcmp(root["encoding"], "aiwa_rc_t501") == 0) { irEncoding = AIWA_RC_T501; }
    else if (strcmp(root["encoding"], "lg") == 0) { irEncoding = LG; }
    else if (strcmp(root["encoding"], "sanyo") == 0) { irEncoding = SANYO; }
    else if (strcmp(root["encoding"], "mitsubishi") == 0) { irEncoding = MITSUBISHI; }
    else if (strcmp(root["encoding"], "dish") == 0) { irEncoding = DISH; }
    else if (strcmp(root["encoding"], "sharp") == 0) { irEncoding = SHARP; }
    else if (strcmp(root["encoding"], "coolix") == 0) { irEncoding = COOLIX; }
    else if (strcmp(root["encoding"], "daikin") == 0) { irEncoding = DAIKIN; }
    else if (strcmp(root["encoding"], "denon") == 0) { irEncoding = DENON; }
    else if (strcmp(root["encoding"], "kelvinator") == 0) { irEncoding = KELVINATOR; }
    else if (strcmp(root["encoding"], "sherwood") == 0) { irEncoding = SHERWOOD; }
    else if (strcmp(root["encoding"], "mitsubishi_ac") == 0) { irEncoding = MITSUBISHI_AC; }
    else if (strcmp(root["encoding"], "rcmm") == 0) { irEncoding = RCMM; }
  }
  else {
    Serial.println("Missing IR encoding");
    return false;
  }

  Serial.println("Check code");
  // get IR code to be sent
  if (root.containsKey("code")) {
    irCode = root["code"];
  }
  else { // Not flashing
    Serial.println("Missing IR code");
    return false;
  }
  Serial.println("Return");
  return true;
}

void reconnect() {
  unsigned char i;
  // Loop until we're reconnected
  while (!client.connected()) {
    digitalWrite(txPin, LOW);
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(client_id, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(light_set_topic);
      digitalWrite(txPin, HIGH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      for(i=0; i<5; i++)
      {
        digitalWrite(txPin, LOW);
        delay(500);
        digitalWrite(txPin, HIGH);
        delay(500);
      }
    }
  }
}

void loop() {
  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
