#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// Configurações do LED RGB
#define red 12  // Pino do canal vermelho
#define green 13   // Pino do canal verde
#define blue 14  // Pino do canal azul


//Configurações sensores
#define KY026 34
#define MQ2PIN_A 32
#define MQ2PIN_D 35
#define DHTPIN 27
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

bool dSensor;
int aSensor;

const unsigned long duration = 60000;  // 60 seconds in milliseconds
unsigned long startTime;

// WiFi
const char *ssid = "iPhone Luz"; // Enter your WiFi name
const char *password = "analuz2110";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io"; // broker address
const char *topic = "ufabc/becn_incendio"; // define topic 
const char *mqtt_username = "emqx"; // username for authentication
const char *mqtt_password = "public"; // password for authentication
const int mqtt_port = 1883; // port of MQTT over TCP

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {

  pinMode(red, OUTPUT);//Define a variável azul como saída
  pinMode(green, OUTPUT);//Define a variável verde como saída
  pinMode (blue, OUTPUT);//Define a variável vermelho como saída

  acenderLedVermelho();

  delay(500);

  Serial.begin(115200);
  startTime = millis();

  while (!Serial)
  delay(10); // will pause Zero, Leonardo, etc until serial console opens

  setup_wifi();
  client.publish(topic, "becn_incendio"); // publish to the topic
  client.subscribe(topic); // subscribe from the topic

  Serial.println(F("DHTxx test!"));
  dht.begin();
}

void loop() {

 // Wait a few seconds between measurements.
  delay(2000);

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperatura: ");
  Serial.println(temperature);
  Serial.print("Umidade: ");
  Serial.println(humidity);

  // Read the analog value from the MQ2 sensor
  int fumacaAnalog = analogRead(MQ2PIN_A);

  // Read the digital value from the MQ2 sensor
  int sensorFumacaDigital = digitalRead(MQ2PIN_D);

  // Print the sensor values to the Serial Monitor
  Serial.print("Fumaça: ");
  Serial.println(fumacaAnalog);
  // Serial.print(" | MQ2 Digital Value: ");
  // Serial.println(sensorFumacaDigital);

  int sensorChamas = analogRead(KY026); 
  Serial.print("Chamas: ");
  Serial.println(sensorChamas);

  // Format the data into a single string
  String payload = "Temperatura: " + String(temperature) + ", " +
                   "Humidade: " + String(humidity) + ", " +
                   "Fumaça: " + String(fumacaAnalog) + ", " +
                   "Chamas: " + String(sensorChamas);

  //Convert the payload string to a C-string (const char*)
  client.publish(topic, payload.c_str());


  if (WiFi.status() == WL_CONNECTED){
    // Esperar por uma mensagem
    if (client.loop() && client.connected()) {
      acenderLedVerde();
    } else {
      acenderLedAzul();
      setup_broker();
    }
  } else{
    acenderLedVermelho();
    setup_wifi();
  }

  client.loop();  
}

void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println("\"" + String(ssid) + "\"");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  acenderLedAzul();
  setup_broker();
}

void setup_broker(){
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Public emqx mqtt broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }

  acenderLedVerde();
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}

void acenderLedAzul() {
  digitalWrite(blue, HIGH);
  digitalWrite(green, LOW);
  digitalWrite(red, LOW);
}

void acenderLedVermelho() {
  digitalWrite(blue, LOW);
  digitalWrite(green, LOW);
  digitalWrite(red, HIGH);
}

void acenderLedVerde() {
  digitalWrite(blue, LOW);
  digitalWrite(green, HIGH);
  digitalWrite(red, LOW);
}