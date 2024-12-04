#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

//POST en CMD: curl -X POST -d "soilMoisture=500&rainSensor=300&humidity=45&temperature=25&pressure=1013&altitude=100" http://192.168.16.24/post

// Credenciales Wi-Fi
const char* ssid = "clifford";
const char* password = "cliff4yyyy";

// Variables de sensores
int soilMoisture = 0; // Sensor de humedad de tierra
int rainSensor = 0;   // Sensor de lluvia
int humidity = 0;     // Humedad atmosférica
int temperature = 0;  // Temperatura
int pressure = 0;     // Presión atmosférica
int altitude = 0;     // Altitud

// Pines de los LEDs RGB (agrupados por sensor)
const int rgbPins[4][3] = {
  {25, 26, 27}, // soilMoisture: {Red, Green, Blue}
  {32, 33, 34}, // rainSensor: {Red, Green, Blue}
  {14, 12, 13}, // humidity: {Red, Green, Blue}
  {22, 23, 21}  // temperature: {Red, Green, Blue}
};

// Variables de sensores
int sensorValues[4] = {0, 0, 0, 0}; // {soilMoisture, rainSensor, humidity, temperature}

// Crear servidor web y WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Función para controlar el color de un LED RGB
void setRGB(int sensorIndex, int redValue, int greenValue, int blueValue) {
  analogWrite(rgbPins[sensorIndex][0], redValue);
  analogWrite(rgbPins[sensorIndex][1], greenValue);
  analogWrite(rgbPins[sensorIndex][2], blueValue);
}

// Actualizar LEDs RGB según los valores de los sensores
void updateRGB() {
  // LED para soilMoisture
  if (soilMoisture < 200) setRGB(0, 0, 255, 0); // Verde
  else if (soilMoisture < 400) setRGB(0, 255, 255, 0); // Amarillo
  else setRGB(0, 255, 0, 0); // Rojo

  // LED para rainSensor
  if (rainSensor < 200) setRGB(1, 0, 0, 255); // Azul
  else if (rainSensor < 400) setRGB(1, 0, 255, 255); // Cian
  else setRGB(1, 255, 0, 255); // Magenta

  // LED para humidity
  if (humidity < 30) setRGB(2, 255, 0, 0); // Rojo
  else if (humidity < 60) setRGB(2, 0, 255, 0); // Verde
  else setRGB(2, 0, 0, 255); // Azul

  // LED para temperature
  if (temperature < 10) setRGB(3, 0, 0, 255); // Azul
  else if (temperature < 20) setRGB(3, 0, 255, 0); // Verde
  else if (temperature < 30) setRGB(3, 255, 255, 0); // Amarillo
  else setRGB(3, 255, 0, 0); // Rojo
}

// Función para notificar cambios a los clientes WebSocket
void notifyClients() {
  String message = "{\"soilMoisture\":" + String(soilMoisture) + ",";
  message += "\"rainSensor\":" + String(rainSensor) + ",";
  message += "\"humidity\":" + String(humidity) + ",";
  message += "\"temperature\":" + String(temperature) + ",";
  message += "\"pressure\":" + String(pressure) + ",";
  message += "\"altitude\":" + String(altitude) + "}";
  ws.textAll(message);

  // Actualizar LEDs RGB
  updateRGB();
}

// Función para manejar mensajes `POST`
void handlePostRequest(AsyncWebServerRequest *request) {
  // Verificar parámetros de sensores
  if (request->hasParam("soilMoisture", true)) {
    soilMoisture = request->getParam("soilMoisture", true)->value().toInt();
  }
  if (request->hasParam("rainSensor", true)) {
    rainSensor = request->getParam("rainSensor", true)->value().toInt();
  }
  if (request->hasParam("humidity", true)) {
    humidity = request->getParam("humidity", true)->value().toInt();
  }
  if (request->hasParam("temperature", true)) {
    temperature = request->getParam("temperature", true)->value().toInt();
  }
  if (request->hasParam("pressure", true)) {
    pressure = request->getParam("pressure", true)->value().toInt();
  }
  if (request->hasParam("altitude", true)) {
    altitude = request->getParam("altitude", true)->value().toInt();
  }

  // Notificar a los clientes WebSocket
  notifyClients();

  // Responder al cliente
  request->send(200, "application/json", "{\"message\":\"Datos actualizados correctamente\"}");
}

void setup() {
  Serial.begin(115200);

  // Configuración de los pines de los LEDs RGB
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      pinMode(rgbPins[i][j], OUTPUT);
    }
  }

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
    <div class="sensor-title">Humedad Atmosferica</div>
    <div class="sensor-value">Valor: <span id="humidity">-</span></div>
    <div class="sensor-description" id="humidity-desc">-</div>
  </div>
  <div class="sensor">
    <div class="sensor-title">Temperatura</div>
    <div class="sensor-value">Valor: <span id="temperature">-</span></div>
    <div class="sensor-description" id="temperature-desc">-</div>
  </div>
  <div class="sensor">
    <div class="sensor-title">Presion Atmosférica</div>
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
}
