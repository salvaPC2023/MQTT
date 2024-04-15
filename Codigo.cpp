#include <WiFi.h>
#include <PubSubClient.h>

const char * WIFI_SSID = "wifi name";
const char * WIFI_PASS = "wifi password";

const char * MQTT_BROKER_HOST = "broker.hivemq.com";
const int MQTT_BROKER_PORT = 1883;

const char * MQTT_CLIENT_ID = "grupo9.1";       // Unique CLIENT_ID
const char * PUBLISH_TOPIC = "cba/ucb/edu/bo/testqos";    // TOPIC where ESP32 publishes
const char * SUBSCRIBE_TOPIC = "cba/ucb/edu/bo/testqos";// TOPIC where ESP32 receive

WiFiClient wiFiClient;
PubSubClient mqttClient(wiFiClient);

class LED{ 
public: 
  LED(int pin): pin(pin){
    pinMode(pin, OUTPUT);
    estado = LOW;
  }
  
  void encenderLed(){
    digitalWrite(pin, HIGH);
    estado = HIGH;
  }
  
  void apagarLed(){
    digitalWrite(pin, LOW);
    estado = LOW;
  }
  
  void parpadearLed(int intervalo){
    encenderLed();
    delay(intervalo / 2);
    apagarLed();
    
  }
  
  int obtenerEstado(){
    return estado;
  }
  
private:
  int pin;
  int estado;
};

long readUltrasonicDistance(int triggerPin, int echoPin)
{
  pinMode(triggerPin, OUTPUT);  // Clear the trigger
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  return pulseIn(echoPin, HIGH);
}

// Sensor Ultrasonico inicializando PINES
int echoPin = 12;
int triggerPin = 13;

int inches = 0;
int cm = 0;


LED led(33);

void callback(const char * topic, byte * payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) message += String((char) payload[i]);
  if (String(topic) == SUBSCRIBE_TOPIC) {
    Serial.println("Message from topic " + String(topic) + ":" + message);
    if (message == "LED_ON") {
        led.encenderLed();
    } else if (message == "LED_OFF") {
        led.apagarLed();
    }
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  Serial.print("Connecting to " + String(WIFI_SSID));
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  mqttClient.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
  mqttClient.setCallback(callback);

  Serial.print("Connecting to " + String(MQTT_BROKER_HOST));
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println(" Connected!");
    mqttClient.subscribe(SUBSCRIBE_TOPIC);
  }  
}

unsigned char counter = 0;

unsigned long previousPublishMillis = 0;

void loop() {
  if (mqttClient.connected()) {
    unsigned long now = millis();
    if (now - previousPublishMillis >= 2000) {
      previousPublishMillis = now;
      cm = 0.01723 * readUltrasonicDistance(triggerPin, echoPin);
      String message = String(counter++);
      mqttClient.publish(PUBLISH_TOPIC, message.c_str());
      String jsonMessage = "{\"distance_cm\": " + String(cm) + "}";
      mqttClient.publish(PUBLISH_TOPIC, jsonMessage.c_str());
    }
    mqttClient.loop();
  }
  else {
    Serial.println("MQTT broker not connected!");
    delay(2000);
  }
}