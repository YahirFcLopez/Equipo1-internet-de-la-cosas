#include <WiFi.h>
#include <HTTPClient.h>

// ---- CONFIGURACIÓN WIFI ----
const char* ssid = "nombre del internet";
const char* password = "contraseña";

// ---- CONFIGURACIÓN INFLUXDB ----
const char* influxURL = "poner aqui tu ip/api/v2/write?org=nombre de tu organizacion &bucket=nombre del bucketo&precision=s";
const char* authToken = "xskIIG_5eo7jswCPUerNg02QFNvw03qt_TrEIskAzTn1ETbh9n3ZzeuMxxKIWynvXNHRu5N3mb11MDzJL1Lwtg==";

// ---- CONFIGURACIÓN SENSOR ULTRASÓNICO ----
const int trigPin = 5;  // Pin TRIG del sensor
const int echoPin = 18; // Pin ECHO del sensor

long duration;
float distance;

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Conexión WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Conectado a WiFi!");
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // --- Lectura del sensor ultrasónico ---
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.0343) / 2; // Distancia en centímetros

  Serial.print("Distancia medida: ");
  Serial.print(distance);
  Serial.println(" cm");

  // --- Envío a InfluxDB ---
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(influxURL);
    http.addHeader("Authorization", "Token " + String(authToken));
    http.addHeader("Content-Type", "text/plain");

    // Line Protocol: measurement field=value
    String data = "sensor_ultrasonico distancia=" + String(distance, 2);

    int httpResponseCode = http.POST(data);
    Serial.print("📤 Enviando a InfluxDB... Código HTTP: ");
    Serial.println(httpResponseCode);

    http.end();
  } else {
    Serial.println("❌ WiFi desconectado. Intentando reconectar...");
    WiFi.reconnect();
  }

  // Esperar 1 segundos antes del siguiente envío de informacion ala pagina
  delay(1000);
}

