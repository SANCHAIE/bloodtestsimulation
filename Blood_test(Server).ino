#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ESP8266WebServer.h>

#define AP_SSID "Blood_Simulator"
#define AP_PASS "12345678"  // Password for Access Point

ESP8266WebServer server(80);

uint8_t receiverMAC[] = {0xC4, 0xD8, 0xD5, 0x2D, 0xC6, 0xA2};  // Replace with your ESP-NOW receiver MAC

// Structure for ESP-NOW data
typedef struct struct_message {
  String time;
  String blood;
} struct_message;

struct_message myData;
bool dataSubmitted = false;

// Function Prototypes
void handleRoot();
void handleSubmit();
void switchToESPNow();
void sendData();

// Callback when ESP-NOW data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  } else {
    Serial.println("Delivery fail");
  }
}

void setup() {
  Serial.begin(115200);
  
  // Start Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.println("Access Point Started!");
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Start Web Server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/submit", HTTP_POST, handleSubmit);
  server.begin();
  Serial.println("Web Server Started!");
}

void handleRoot() {
  String html = "<html><head>"
                "<title>ESP Data Input</title>"
                "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<style>"
                "body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f4; padding: 50px; }"
                "h1 { color: #333; font-size: 26px; }"
                "form { background: white; padding: 20px; max-width: 400px; margin: auto; border-radius: 10px; box-shadow: 0px 0px 10px 0px #999; }"
                "input[type='text'], input[type='number'] { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ddd; border-radius: 5px; font-size: 16px; }"
                "input[type='submit'] { background-color: #007bff; color: white; padding: 10px; border: none; cursor: pointer; border-radius: 5px; font-size: 18px; }"
                "input[type='submit']:hover { background-color: #0056b3; }"
                "</style>"
                "</head><body>"
                "<h1>Blood Test Input</h1>"
                "<form action='/submit' method='POST'>"
                "<label>Blood Value:</label><br>"
                "<input type='number' name='blood_value' required><br>"
                "<label>Lab Time (HH:MM):</label><br>"
                "<input type='text' name='lab_time' pattern='[0-9]{2}:[0-9]{2}' required><br><br>"
                "<input type='submit' value='Submit'>"
                "</form></body></html>";

  server.send(200, "text/html", html);
}

void handleSubmit() {
  if (server.hasArg("blood_value") && server.hasArg("lab_time")) {
    String bloodStr = server.arg("blood_value");
    String timeStr = server.arg("lab_time");

    myData.blood = bloodStr;
    myData.time = timeStr;

    Serial.printf("Received Data -> Blood: %s, Lab Time: %s\n", myData.blood, myData.time);

    dataSubmitted = true;
    server.send(200, "text/html", "<html><body><h1>Data Submitted! Switching to ESP-NOW...</h1></body></html>");
    
    delay(2000);
    switchToESPNow();
  } else {
    server.send(400, "text/html", "<html><body><h1>Invalid Input!</h1></body></html>");
  }
}

void switchToESPNow() {
  Serial.println("\nSwitching to ESP-NOW Mode...");
  server.stop();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(receiverMAC, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  sendData();
}

void sendData() {
  Serial.printf("Sending ESP-NOW Data -> Blood: %s, Lab Time: %s\n", myData.blood, myData.time);
  esp_now_send(receiverMAC, (uint8_t *) &myData, sizeof(myData));

  Serial.println("Data Sent! Restarting ESP...");
  delay(3000);
  ESP.restart();
}

void loop() {
  server.handleClient();
}
