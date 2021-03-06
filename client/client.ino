#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Pi's access point name
const char* ssid = "friendly-raspberry";

// Pi's access point password
const char* password = "hologram";

// broker IP address
const char* mqtt_server = "192.168.42.1";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[120];
int value = 0;

void connect_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    if (WiFi.status() == WL_NO_SSID_AVAIL) {
      Serial.println();
      break;
    } else  {
      delay(1000);
      Serial.print(".");
    }
  }

  if(WiFi.status() == WL_CONNECTED) {
    randomSeed(micros());
  
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Network not found, trying again...");
  }
}

void setup_wifi() {
  while (WiFi.status() != WL_CONNECTED) {
    connect_wifi();
    delay(2000);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);
    // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    // Turn the LED off by making the voltage HIGH
    digitalWrite(BUILTIN_LED, HIGH);
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {

    // First check for wifi connectivity
    if(WiFi.status() == WL_DISCONNECTED){
      Serial.print("Lost WiFi connection, attempting new connection..");
      setup_wifi();
    }
    
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  long now = millis();

  // verify device is connected to the Broker
  if (!client.connected()) {
    reconnect();
  }

  // MQTT client library loops through checking the queue
  client.loop();

  // publish a message every 2 seconds
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 120, "hello hologram #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);

    // publish msg to the topic "node/value"
    client.publish("node/value", msg);
  }
}