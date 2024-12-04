#include <WiFi.h>

const char* ssid = "clifford";
const char* password = "cliff4yyyy";

WiFiServer server(80);

String header;
String postData;
String output26State = "off";
String output27State = "off";

const int output26 = 26;
const int output27 = 27;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void handlePostData(String data) {
  if (data.indexOf("output26=on") >= 0) {
    Serial.println("POST: GPIO 26 on");
    output26State = "on";
    digitalWrite(output26, HIGH);
  } else if (data.indexOf("output26=off") >= 0) {
    Serial.println("POST: GPIO 26 off");
    output26State = "off";
    digitalWrite(output26, LOW);
  }

  if (data.indexOf("output27=on") >= 0) {
    Serial.println("POST: GPIO 27 on");
    output27State = "on";
    digitalWrite(output27, HIGH);
  } else if (data.indexOf("output27=off") >= 0) {
    Serial.println("POST: GPIO 27 off");
    output27State = "off";
    digitalWrite(output27, LOW);
  }
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";
    postData = "";
    boolean isPost = false;

    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;

        if (header.indexOf("POST") >= 0) {
          isPost = true;
        }

        if (c == '\n' && currentLine.length() == 0) {
          if (isPost) {
            while (client.available()) {
              postData += (char)client.read();
            }
            handlePostData(postData);
          }

          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println();

          client.println("<!DOCTYPE html><html>");
          client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
          client.println("<link rel=\"icon\" href=\"data:,\">");
          client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
          client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
          client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
          client.println(".button2 {background-color: #555555;}</style></head>");

          client.println("<body><h1>ESP32 Web Server</h1>");
          client.println("<p>GPIO 26 - State " + output26State + "</p>");
          if (output26State == "off") {
            client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
          } else {
            client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
          }

          client.println("<p>GPIO 27 - State " + output27State + "</p>");
          if (output27State == "off") {
            client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
          } else {
            client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
          }

          client.println("</body></html>");
          client.println();
          break;
        } else if (c == '\n') {
          currentLine = "";
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    header = "";
    postData = "";
    client.stop();
    Serial.println("Client disconnected.");
  }
}
