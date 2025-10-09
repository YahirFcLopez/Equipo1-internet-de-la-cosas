#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ==================== CONFIGURACIÓN WIFI ====================
const char* ssid = "";           // Reemplaza con tu WiFi
const char* password = "";         // Reemplaza con tu contraseña

// ==================== CONFIGURACIÓN MQTT ====================c
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* topic = "casa/sensor_tactil/datos";

WiFiClient espClient;
PubSubClient client(espClient);

// ==================== VARIABLES DEL SENSOR ====================
unsigned long lastMsg = 0;
const long interval = 5000;  // Intervalo de envío: 5 segundos
int batteryLevel = 100;      // Batería inicial: 100%
int messageCount = 0;        // Contador de mensajes

// ==================== FUNCIÓN PARA CONECTAR WIFI ====================
void setup_wifi() {
  Serial.begin(115200);
  
  // Conectar a WiFi
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

// ==================== FUNCIÓN PARA RECONEXIÓN MQTT ====================
void reconnect() {
  // Loop hasta que estemos reconectados
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    
    // Crear un Client ID único
    String clientId = "ESP32-Sensor-";
    clientId += String(random(0xffff), HEX);
    
    // Intentar conectar
    if (client.connect(clientId.c_str())) {
      Serial.println("conectado!");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando nuevamente en 5 segundos");
      delay(5000);
    }
  }
}

// ==================== SETUP PRINCIPAL ====================
void setup() {
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  randomSeed(analogRead(0)); // Semilla para números aleatorios
}

// ==================== LOOP PRINCIPAL ====================
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;
    messageCount++;

    // ==================== SIMULAR DATOS DEL SENSOR ====================
    int sensorReading = random(100, 601);  // valor entre 100-600
    
    // Simular consumo de batería
    if (batteryLevel > 0) {
      batteryLevel -= 1; // Reduce 1% cada mensaje
    }
    if (batteryLevel < 0) batteryLevel = 0;
    
    // Ocasionalmente simular una recarga
    if (messageCount % 20 == 0 && batteryLevel < 30) {
      batteryLevel = 90; // Recarga simulada
    }

    // ==================== CREAR MENSAJE JSON ====================
    StaticJsonDocument<200> doc;
    doc["id"] = "sensor_tactil_01";
    doc["lectura"] = sensorReading;
    doc["bateria"] = batteryLevel;
    doc["timestamp"] = millis();

    // Convertir JSON a string
    char jsonBuffer[200];
    serializeJson(doc, jsonBuffer);

    // ==================== PUBLICAR MENSAJE ====================
    Serial.print("Publicando mensaje: ");
    Serial.println(jsonBuffer);
    
    client.publish(topic, jsonBuffer);
  }
}  

