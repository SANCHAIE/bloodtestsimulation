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
  String html = "<!DOCTYPE html><html lang='en'>"
                "<head>"
                "<meta charset='UTF-8'>"
                "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<title>Blood Glucose Level Measurement</title>"
                "<style>"
                "body {"
                "  font-family: 'Arial', sans-serif;"
                "  display: flex;"
                "  justify-content: center;"
                "  align-items: center;"
                "  height: 100vh;"
                "  margin: 0;"
                "  background-color: #f4f4f4;"
                "  color: #333;"
                "}"
                ".container {"
                "  text-align: center;"
                "  background-color: white;"
                "  padding: 30px;"
                "  border-radius: 15px;"
                "  box-shadow: 0 10px 20px rgba(0,0,0,0.1);"
                "  width: 400px;"
                "}"
                ".app-title {"
                "  background-color: #3498db;"
                "  color: white;"
                "  padding: 15px;"
                "  border-radius: 10px;"
                "  margin-bottom: 20px;"
                "  font-size: 24px;"
                "  font-weight: bold;"
                "}"
                ".result-display {"
                "  background-color: #e0e0e0;"
                "  padding: 20px;"
                "  margin: 15px 0;"
                "  font-size: 36px;"
                "  border-radius: 10px;"
                "  min-height: 60px;"
                "  display: flex;"
                "  justify-content: center;"
                "  align-items: center;"
                "  position: relative;"
                "}"
                ".result-unit {"
                "  position: absolute;"
                "  right: 20px;"
                "  font-size: 24px;"
                "  color: #666;"
                "}"
                ".number-pad {"
                "  display: grid;"
                "  grid-template-columns: repeat(3, 1fr);"
                "  gap: 15px;"
                "  margin-bottom: 15px;"
                "}"
                ".number-button, .control-button {"
                "  padding: 15px;"
                "  font-size: 22px;"
                "  border: none;"
                "  border-radius: 10px;"
                "  cursor: pointer;"
                "  transition: background-color 0.3s ease;"
                "}"
                ".number-button {"
                "  background-color: #3498db;"
                "  color: white;"
                "}"
                ".number-button:hover {"
                "  background-color: #2980b9;"
                "}"
                ".control-button {"
                "  background-color: #2ecc71;"
                "  color: white;"
                "  width: 100%;"
                "  margin-bottom: 10px;"
                "}"
                ".control-button:hover {"
                "  background-color: #27ae60;"
                "}"
                ".time-input {"
                "  width: 100%;"
                "  padding: 10px;"
                "  margin: 10px 0;"
                "  font-size: 22px;"
                "  border: 1px solid #ddd;"
                "  border-radius: 5px;"
                "}"
                ".control-buttons {"
                "  display: flex;"
                "  gap: 10px;"
                "}"
                "</style>"
                "</head>"
                "<body>"
                "<div class='container'>"
                "<div class='app-title'>Blood Glucose Level Measurement</div>"
                
                "<div class='result-display'>"
                "<span id='displayValue'>0</span>"
                "<span class='result-unit'>mg/dL</span>"
                "</div>"
                
                "<div class='number-pad'>"
                "<button class='number-button' onclick='addNumber(1)'>1</button>"
                "<button class='number-button' onclick='addNumber(2)'>2</button>"
                "<button class='number-button' onclick='addNumber(3)'>3</button>"
                "<button class='number-button' onclick='addNumber(4)'>4</button>"
                "<button class='number-button' onclick='addNumber(5)'>5</button>"
                "<button class='number-button' onclick='addNumber(6)'>6</button>"
                "<button class='number-button' onclick='addNumber(7)'>7</button>"
                "<button class='number-button' onclick='addNumber(8)'>8</button>"
                "<button class='number-button' onclick='addNumber(9)'>9</button>"
                "<button class='number-button' onclick='addDecimal()'>.</button>"
                "<button class='number-button' onclick='addNumber(0)'>0</button>"
                "<button class='number-button' onclick='backspace()'>⌫</button>"
                "</div>"
                
                "<div class='control-buttons'>"
                "<button class='control-button' onclick='sendMeasurement()'>SEND</button>"
                "<button class='control-button' onclick='showMeasurement()'>SHOW</button>"
                "<button class='control-button' onclick='clearMeasurement()'>CLEAR</button>"
                "</div>"
                
                "<input type='text' id='timeInput' class='time-input' placeholder='Lab Time (HH:MM)' pattern='[0-9]{2}:[0-9]{2}'>"
                
                "</div>"
                
                "<script>"
                "let enteredNumber = '0';"
                "const displayValue = document.getElementById('displayValue');"
                "const timeInput = document.getElementById('timeInput');"
                
                "function addNumber(num) {"
                "  if (enteredNumber.length < 5) {"
                "    if (num === '.' && enteredNumber.includes('.')) return;"
                "    if (enteredNumber === '0' && num !== '.') {"
                "      enteredNumber = '';"
                "    }"
                "    enteredNumber += num;"
                "    updateDisplay();"
                "  }"
                "}"
                
                "function addDecimal() {"
                "  if (!enteredNumber.includes('.') && enteredNumber !== '') {"
                "    enteredNumber += '.';"
                "    updateDisplay();"
                "  }"
                "}"
                
                "function backspace() {"
                "  enteredNumber = enteredNumber.slice(0, -1);"
                "  if (enteredNumber === '') {"
                "    enteredNumber = '0';"
                "  }"
                "  updateDisplay();"
                "}"
                
                "function clearMeasurement() {"
                "  enteredNumber = '0';"
                "  updateDisplay();"
                "}"
                
                "function sendMeasurement() {"
                "  const bloodValue = enteredNumber;"
                "  const labTime = timeInput.value;"
                
                "  if (bloodValue === '0' || bloodValue === '') {"
                "    alert('Please enter a valid glucose level.');"
                "    return;"
                "  }"
                
                "  if (!labTime) {"
                "    alert('Please enter lab time.');"
                "    return;"
                "  }"
                
                "  fetch('/submit', {"
                "    method: 'POST',"
                "    headers: {"
                "      'Content-Type': 'application/x-www-form-urlencoded',"
                "    },"
                "    body: 'blood_value=' + encodeURIComponent(bloodValue) + '&lab_time=' + encodeURIComponent(labTime)"
                "  })"
                "  .then(response => response.text())"
                "  .then(data => {"
                "    alert('Measurement Saved');"
                "  })"
                "  .catch(error => {"
                "    alert('Error: ' + error);"
                "  });"
                "}"
                
                "function showMeasurement() {"
                "  displayValue.textContent = enteredNumber;"
                "}"
                
                "function updateDisplay() {"
                "  displayValue.textContent = enteredNumber || '0';"
                "}"
                
                "updateDisplay();"
                "</script>"
                "</body>"
                "</html>";

  server.send(200, "text/html", html);
}

void handleSubmit() {
  if (server.hasArg("blood_value") && server.hasArg("lab_time")) {
    String bloodStr = server.arg("blood_value");
    String timeStr = server.arg("lab_time");

    // Validate blood value
    float bloodValue = bloodStr.toFloat();
    if (bloodValue < 0 || bloodValue > 500) {
      server.send(400, "text/plain", "Invalid Blood Glucose Level!");
      return;
    }

    // Validate time format
    if (!isValidTimeFormat(timeStr)) {
      server.send(400, "text/plain", "Invalid Time Format!");
      return;
    }

    myData.blood = bloodStr;
    myData.time = timeStr;

    Serial.printf("Received Data -> Blood: %s, Lab Time: %s\n", myData.blood, myData.time);

    dataSubmitted = true;
    server.send(200, "text/plain", "Data Submitted Successfully");
    
    switchToESPNow();
  } else {
    server.send(400, "text/plain", "Invalid Input!");
  }
}

// Time format validation function
bool isValidTimeFormat(String timeStr) {
  // Check if the string matches HH:MM format
  if (timeStr.length() != 5) return false;
  
  // Check if ':' is in the right place
  if (timeStr.charAt(2) != ':') return false;
  
  // Extract hours and minutes
  int hours = timeStr.substring(0, 2).toInt();
  int minutes = timeStr.substring(3).toInt();
  
  // Check hours range
  if (hours < 0 || hours > 23) return false;
  
  // Check minutes range
  if (minutes < 0 || minutes > 59) return false;
  
  return true;
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
