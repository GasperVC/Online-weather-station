#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// Credenciales Wi-Fi
const char* ssid = "clifford";
const char* password = "cliff4yyyy";

// Pines GPIO
const int output26 = 26;
const int output27 = 27;

// Estados de los GPIOs
String output26State = "off";
String output27State = "off";

// Variables de sensores simuladas
int soilMoisture = 450; // Sensor de humedad de tierra
int rainSensor = 300;   // Sensor de lluvia
int humidity = 50;      // Humedad atmosférica
int temperature = 25;   // Temperatura
int pressure = 1013;    // Presión atmosférica
int altitude = 120;     // Altitud

// Crear servidor web y WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Función para notificar cambios a los clientes WebSocket
void notifyClients() {
  String message = "{\"output26\":\"" + output26State + "\", \"output27\":\"" + output27State + "\", ";
  message += "\"soilMoisture\":" + String(soilMoisture) + ",";
  message += "\"rainSensor\":" + String(rainSensor) + ",";
  message += "\"humidity\":" + String(humidity) + ",";
  message += "\"temperature\":" + String(temperature) + ",";
  message += "\"pressure\":" + String(pressure) + ",";
  message += "\"altitude\":" + String(altitude) + "}";
  ws.textAll(message);
}

// Función para manejar mensajes `POST`
void handlePostRequest(AsyncWebServerRequest *request) {
  if (request->hasParam("output26", true)) {
    String value = request->getParam("output26", true)->value();
    if (value == "on") {
      output26State = "on";
      digitalWrite(output26, HIGH);
    } else if (value == "off") {
      output26State = "off";
      digitalWrite(output26, LOW);
    }
  }

  if (request->hasParam("output27", true)) {
    String value = request->getParam("output27", true)->value();
    if (value == "on") {
      output27State = "on";
      digitalWrite(output27, HIGH);
    } else if (value == "off") {
      output27State = "off";
      digitalWrite(output27, LOW);
    }
  }

  notifyClients();
  request->send(200, "application/json", "{\"message\":\"POST recibido\"}");
}

void setup() {
  Serial.begin(115200);

  // Configuración de pines GPIO
  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);

  // Conexión Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Configuración del WebSocket
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.printf("WebSocket client #%u connected\n", client->id());
      notifyClients();
    } else if (type == WS_EVT_DISCONNECT) {
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
    } else if (type == WS_EVT_DATA) {
      AwsFrameInfo *info = (AwsFrameInfo*)arg;
      if (info->opcode == WS_TEXT) {
        String message = "";
        for (size_t i = 0; i < len; i++) {
          message += (char)data[i];
        }
        Serial.println("Received WebSocket Message: " + message);
      }
    }
  });
  server.addHandler(&ws);

  // Página web con dashboard
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Dashboard</title>
  <style>
    html { font-family: Helvetica; text-align: center; margin: 0; padding: 0; }
    body { padding: 20px; }
    h1 { font-size: 24px; }
    .sensor { margin: 20px auto; border: 1px solid #ccc; border-radius: 10px; padding: 15px; max-width: 300px; background-color: #f9f9f9; }
    .sensor-title { font-size: 18px; font-weight: bold; }
    .sensor-value { font-size: 16px; margin: 5px 0; }
    .sensor-description { font-size: 14px; color: #555; }
  </style>
</head>
<body>
  <h1>ESP32 Sensor Dashboard</h1>
  <div class="sensor">
    <div class="sensor-title">Sensor de Humedad de Tierra</div>
    <div class="sensor-value">Valor: <span id="soil-moisture">-</span></div>
    <div class="sensor-description" id="soil-moisture-desc">-</div>
  </div>
  <div class="sensor">
    <div class="sensor-title">Sensor de Lluvia</div>
    <div class="sensor-value">Valor: <span id="rain-sensor">-</span></div>
    <div class="sensor-description" id="rain-sensor-desc">-</div>
  </div>
  <div class="sensor">
    <div class="sensor-title">Humedad Atmosférica</div>
    <div class="sensor-value">Valor: <span id="humidity">-</span></div>
    <div class="sensor-description" id="humidity-desc">-</div>
  </div>
  <div class="sensor">
    <div class="sensor-title">Temperatura</div>
    <div class="sensor-value">Valor: <span id="temperature">-</span></div>
    <div class="sensor-description" id="temperature-desc">-</div>
  </div>
  <div class="sensor">
    <div class="sensor-title">Presión Atmosférica</div>
    <div class="sensor-value">Valor: <span id="pressure">-</span></div>
  </div>
  <div class="sensor">
    <div class="sensor-title">Altitud</div>
    <div class="sensor-value">Valor: <span id="altitude">-</span></div>
  </div>

  <script>
    const ws = new WebSocket(`ws://${window.location.hostname}/ws`);
    ws.onmessage = (event) => {
      const data = JSON.parse(event.data);

      document.getElementById("soil-moisture").innerText = data.soilMoisture;
      document.getElementById("soil-moisture-desc").innerText =
        data.soilMoisture < 200 ? "Muy húmedo" :
        data.soilMoisture < 400 ? "Húmedo" :
        data.soilMoisture < 600 ? "Moderado" :
        data.soilMoisture < 800 ? "Seco" : "Muy seco";

      document.getElementById("rain-sensor").innerText = data.rainSensor;
      document.getElementById("rain-sensor-desc").innerText =
        data.rainSensor < 200 ? "Sin lluvia" :
        data.rainSensor < 400 ? "Lluvia ligera" :
        data.rainSensor < 600 ? "Lluvia moderada" : "Lluvia intensa";

      document.getElementById("humidity").innerText = data.humidity;
      document.getElementById("humidity-desc").innerText =
        data.humidity < 30 ? "Baja" :
        data.humidity < 60 ? "Normal" : "Alta";

      document.getElementById("temperature").innerText = data.temperature;
      document.getElementById("temperature-desc").innerText =
        data.temperature < 10 ? "Muy frío" :
        data.temperature < 20 ? "Frío" :
        data.temperature < 30 ? "Temperatura ideal" :
        data.temperature < 40 ? "Calor" : "Ventanilla";

      document.getElementById("pressure").innerText = data.pressure;
      document.getElementById("altitude").innerText = data.altitude;
    };
  </script>
</body>
</html>
)rawliteral");
  });

  // Ruta para manejar `POST`
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request) {
    handlePostRequest(request);
  });

  server.begin();
}

void loop() {
  ws.cleanupClients();
  // Simula cambios en sensores para pruebas
  soilMoisture = random(0, 1024);
  rainSensor = random(0, 1024);
  humidity = random(0, 100);
  temperature = random(0, 50);
  delay(5000); // Enviar datos cada 5 segundos
  notifyClients();
}
