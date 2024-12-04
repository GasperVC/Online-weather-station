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

// Crear servidor web y WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Función para notificar cambios a los clientes WebSocket
void notifyClients() {
  String message = "{\"output26\":\"" + output26State + "\", \"output27\":\"" + output27State + "\"}";
  ws.textAll(message);
}

// Función para manejar mensajes `POST`
void handlePostRequest(AsyncWebServerRequest *request) {
  // Verificar parámetros en el cuerpo de la solicitud
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

  // Notificar a los clientes WebSocket
  notifyClients();

  // Responder al cliente
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

  // Página web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 WebSocket</title>
  <style>
    html { font-family: Helvetica; text-align: center; margin: 0; }
    .button { padding: 16px 40px; font-size: 20px; margin: 5px; cursor: pointer; }
    .on { background-color: #4CAF50; color: white; }
    .off { background-color: #555555; color: white; }
  </style>
</head>
<body>
  <h1>ESP32 WebSocket</h1>
  <p>GPIO 26 - State: <span id="output26">off</span></p>
  <button class="button on" onclick="sendMessage('output26:on')">ON</button>
  <button class="button off" onclick="sendMessage('output26:off')">OFF</button>
  <p>GPIO 27 - State: <span id="output27">off</span></p>
  <button class="button on" onclick="sendMessage('output27:on')">ON</button>
  <button class="button off" onclick="sendMessage('output27:off')">OFF</button>
  <script>
    const ws = new WebSocket(`ws://${window.location.hostname}/ws`);
    ws.onmessage = (event) => {
      const data = JSON.parse(event.data);
      document.getElementById("output26").innerText = data.output26;
      document.getElementById("output27").innerText = data.output27;
    };
    function sendMessage(message) {
      ws.send(message);
    }
  </script>
</body>
</html>
)rawliteral");
  });

  // Ruta para manejar `POST`
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
    handlePostRequest(request);
  });

  server.begin();
}

void loop() {
  ws.cleanupClients();
}
