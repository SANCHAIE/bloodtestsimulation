#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ESP8266WebServer.h>

#define AP_SSID "Blood_Simulator"
#define AP_PASS "12345678"  // Password for Access Point

ESP8266WebServer server(80);

// Broadcast MAC Address - ส่งไปทุก Node
uint8_t receiverMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

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
bool isValidTimeFormat(String timeStr);

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
                "  min-height: 100vh;"
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
                ".control-button:disabled {"
                "  background-color: #95a5a6;"
                "  cursor: not-allowed;"
                "}"
                ".time-input {"
                "  width: 100%;"
                "  padding: 10px;"
                "  margin: 10px 0;"
                "  font-size: 22px;"
                "  border: 1px solid #ddd;"
                "  border-radius: 5px;"
                "  box-sizing: border-box;"
                "}"
                ".control-buttons {"
                "  display: flex;"
                "  gap: 10px;"
                "}"
                ".status-bar {"
                "  padding: 10px;"
                "  margin: 10px 0;"
                "  border-radius: 8px;"
                "  font-size: 16px;"
                "  font-weight: bold;"
                "  display: none;"
                "}"
                ".status-sending {"
                "  background-color: #f39c12;"
                "  color: white;"
                "}"
                ".status-success {"
                "  background-color: #27ae60;"
                "  color: white;"
                "}"
                ".status-error {"
                "  background-color: #e74c3c;"
                "  color: white;"
                "}"
                ".history-section {"
                "  margin-top: 20px;"
                "  text-align: left;"
                "}"
                ".history-title {"
                "  font-size: 16px;"
                "  font-weight: bold;"
                "  color: #666;"
                "  margin-bottom: 10px;"
                "}"
                ".history-list {"
                "  max-height: 150px;"
                "  overflow-y: auto;"
                "}"
                ".history-item {"
                "  background-color: #f8f9fa;"
                "  padding: 8px 12px;"
                "  margin: 5px 0;"
                "  border-radius: 5px;"
                "  font-size: 14px;"
                "  display: flex;"
                "  justify-content: space-between;"
                "}"
                ".history-blood {"
                "  font-weight: bold;"
                "  color: #e74c3c;"
                "}"
                ".history-time {"
                "  color: #666;"
                "}"
                ".preset-section {"
                "  margin: 15px 0;"
                "}"
                ".preset-title {"
                "  font-size: 14px;"
                "  color: #666;"
                "  margin-bottom: 8px;"
                "}"
                ".preset-buttons {"
                "  display: flex;"
                "  gap: 8px;"
                "  flex-wrap: wrap;"
                "  justify-content: center;"
                "}"
                ".preset-button {"
                "  padding: 10px 15px;"
                "  font-size: 16px;"
                "  border: 2px solid #9b59b6;"
                "  border-radius: 20px;"
                "  background-color: white;"
                "  color: #9b59b6;"
                "  cursor: pointer;"
                "  transition: all 0.2s ease;"
                "}"
                ".preset-button:hover {"
                "  background-color: #9b59b6;"
                "  color: white;"
                "}"
                ".time-row {"
                "  display: flex;"
                "  gap: 10px;"
                "  margin: 10px 0;"
                "}"
                ".time-row .time-input {"
                "  flex: 1;"
                "  margin: 0;"
                "}"
                ".now-button {"
                "  padding: 10px 20px;"
                "  font-size: 16px;"
                "  border: none;"
                "  border-radius: 5px;"
                "  background-color: #3498db;"
                "  color: white;"
                "  cursor: pointer;"
                "}"
                ".now-button:hover {"
                "  background-color: #2980b9;"
                "}"
                ".modal-overlay {"
                "  display: none;"
                "  position: fixed;"
                "  top: 0;"
                "  left: 0;"
                "  width: 100%;"
                "  height: 100%;"
                "  background-color: rgba(0,0,0,0.5);"
                "  justify-content: center;"
                "  align-items: center;"
                "  z-index: 1000;"
                "}"
                ".modal-content {"
                "  background-color: white;"
                "  padding: 30px;"
                "  border-radius: 15px;"
                "  text-align: center;"
                "  max-width: 350px;"
                "}"
                ".modal-title {"
                "  font-size: 20px;"
                "  font-weight: bold;"
                "  margin-bottom: 15px;"
                "}"
                ".modal-info {"
                "  font-size: 24px;"
                "  margin: 20px 0;"
                "  padding: 15px;"
                "  background-color: #f8f9fa;"
                "  border-radius: 10px;"
                "}"
                ".modal-info .value {"
                "  color: #e74c3c;"
                "  font-weight: bold;"
                "}"
                ".modal-info .time {"
                "  color: #666;"
                "  font-size: 18px;"
                "}"
                ".modal-buttons {"
                "  display: flex;"
                "  gap: 15px;"
                "  margin-top: 20px;"
                "}"
                ".modal-btn {"
                "  flex: 1;"
                "  padding: 12px;"
                "  font-size: 18px;"
                "  border: none;"
                "  border-radius: 8px;"
                "  cursor: pointer;"
                "}"
                ".modal-btn-confirm {"
                "  background-color: #2ecc71;"
                "  color: white;"
                "}"
                ".modal-btn-cancel {"
                "  background-color: #e74c3c;"
                "  color: white;"
                "}"
                "</style>"
                "</head>"
                "<body>"
                "<div class='container'>"
                "<div class='app-title'>Blood Glucose Level Measurement</div>"
                
                "<div id='statusBar' class='status-bar'></div>"
                
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
                "<button class='number-button' onclick='backspace()'>&#9003;</button>"
                "</div>"
                
                "<div class='preset-section'>"
                "<div class='preset-title'>Quick Preset</div>"
                "<div class='preset-buttons'>"
                "<button class='preset-button' onclick='setPreset(70)'>70</button>"
                "<button class='preset-button' onclick='setPreset(100)'>100</button>"
                "<button class='preset-button' onclick='setPreset(120)'>120</button>"
                "<button class='preset-button' onclick='setPreset(150)'>150</button>"
                "<button class='preset-button' onclick='setPreset(200)'>200</button>"
                "</div>"
                "</div>"
                
                "<div class='time-row'>"
                "<input type='text' id='timeInput' class='time-input' placeholder='HH:MM' pattern='[0-9]{2}[:\\.][0-9]{2}'>\n"
                "<button class='now-button' onclick='setCurrentTime()'>NOW</button>"
                "</div>"
                
                "<div class='control-buttons'>"
                "<button id='sendBtn' class='control-button' onclick='sendMeasurement()'>SEND</button>"
                "<button class='control-button' onclick='clearMeasurement()'>CLEAR</button>"
                "</div>"
                
                "<div class='history-section'>"
                "<div class='history-title'>Recent History (Last 5)</div>"
                "<div id='historyList' class='history-list'></div>"
                "</div>"
                
                "</div>"
                
                "<div id='confirmModal' class='modal-overlay'>"
                "<div class='modal-content'>"
                "<div class='modal-title'>Confirm Send?</div>"
                "<div class='modal-info'>"
                "<div class='value' id='modalBlood'></div>"
                "<div class='time' id='modalTime'></div>"
                "</div>"
                "<div class='modal-buttons'>"
                "<button class='modal-btn modal-btn-cancel' onclick='cancelSend()'>CANCEL</button>"
                "<button class='modal-btn modal-btn-confirm' onclick='confirmSend()'>CONFIRM</button>"
                "</div>"
                "</div>"
                "</div>"
                
                "<script>"
                "let enteredNumber = '0';"
                "let history = [];"
                "const displayValue = document.getElementById('displayValue');"
                "const timeInput = document.getElementById('timeInput');"
                "const statusBar = document.getElementById('statusBar');"
                "const sendBtn = document.getElementById('sendBtn');"
                "const historyList = document.getElementById('historyList');"
                
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
                "  hideStatus();"
                "}"
                
                "function setPreset(value) {"
                "  enteredNumber = value.toString();"
                "  updateDisplay();"
                "}"
                
                "function setCurrentTime() {"
                "  const now = new Date();"
                "  const hours = String(now.getHours()).padStart(2, '0');"
                "  const minutes = String(now.getMinutes()).padStart(2, '0');"
                "  timeInput.value = hours + ':' + minutes;\n"
                "}"
                
                "function showModal() {"
                "  document.getElementById('modalBlood').textContent = enteredNumber + ' mg/dL';"
                "  document.getElementById('modalTime').textContent = timeInput.value;"
                "  document.getElementById('confirmModal').style.display = 'flex';"
                "}"
                
                "function hideModal() {"
                "  document.getElementById('confirmModal').style.display = 'none';"
                "}"
                
                "function cancelSend() {"
                "  hideModal();"
                "}"
                
                "function showStatus(message, type) {"
                "  statusBar.textContent = message;"
                "  statusBar.className = 'status-bar status-' + type;"
                "  statusBar.style.display = 'block';"
                "}"
                
                "function hideStatus() {"
                "  statusBar.style.display = 'none';"
                "}"
                
                "function addToHistory(blood, time) {"
                "  history.unshift({blood: blood, time: time});"
                "  if (history.length > 5) history.pop();"
                "  renderHistory();"
                "}"
                
                "function renderHistory() {"
                "  historyList.innerHTML = history.map(item => "
                "    '<div class=\"history-item\">' +"
                "    '<span class=\"history-blood\">' + item.blood + ' mg/dL</span>' +"
                "    '<span class=\"history-time\">' + item.time + '</span>' +"
                "    '</div>'"
                "  ).join('');"
                "}"
                
                "function sendMeasurement() {"
                "  const bloodValue = enteredNumber;"
                "  const labTime = timeInput.value;"
                
                "  if (bloodValue === '0' || bloodValue === '') {"
                "    showStatus('Please enter a valid glucose level.', 'error');"
                "    return;"
                "  }"
                
                "  if (!labTime) {"
                "    showStatus('Please enter lab time.', 'error');"
                "    return;"
                "  }"
                
                "  showModal();"
                "}"
                
                "function confirmSend() {"
                "  hideModal();"
                "  const bloodValue = enteredNumber;"
                "  const labTime = timeInput.value;"
                
                "  showStatus('Sending...', 'sending');"
                "  sendBtn.disabled = true;"
                "  sendBtn.textContent = 'SENDING...';"
                
                "  fetch('/submit', {"
                "    method: 'POST',"
                "    headers: {"
                "      'Content-Type': 'application/x-www-form-urlencoded',"
                "    },"
                "    body: 'blood_value=' + encodeURIComponent(bloodValue) + '&lab_time=' + encodeURIComponent(labTime)"
                "  })"
                "  .then(response => response.text())"
                "  .then(data => {"
                "    showStatus('Sent Successfully \\u2713', 'success');"
                "    addToHistory(bloodValue, labTime);"
                "    enteredNumber = '0';"
                "    timeInput.value = '';"
                "    updateDisplay();"
                "    sendBtn.disabled = false;"
                "    sendBtn.textContent = 'SEND';"
                "  })"
                "  .catch(error => {"
                "    showStatus('Error: ' + error, 'error');"
                "    sendBtn.disabled = false;"
                "    sendBtn.textContent = 'SEND';"
                "  });"
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

    Serial.printf("Received Data -> Blood: %s, Lab Time: %s\n", myData.blood.c_str(), myData.time.c_str());

    dataSubmitted = true;
    server.send(200, "text/plain", "Data Submitted Successfully");
    
    // รอให้ response ถูกส่งออกไปก่อน
    delay(100);
    server.handleClient();
    delay(100);
    
    switchToESPNow();
  } else {
    server.send(400, "text/plain", "Invalid Input!");
  }
}

// Time format validation function
bool isValidTimeFormat(String timeStr) {
  // Check if the string matches HH:MM or HH.MM format
  if (timeStr.length() != 5) return false;
  
  // Check if ':' or '.' is in the right place
  char separator = timeStr.charAt(2);
  if (separator != ':' && separator != '.') return false;
  
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
  Serial.println("\nSwitching to ESP-NOW Mode (Broadcast)...");
  server.stop();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  
  // Add specific Node peer
  esp_now_add_peer(receiverMAC, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  sendData();
}

void sendData() {
  Serial.println("Broadcasting to all Nodes...");
  Serial.printf("Sending ESP-NOW Data -> Blood: '%s' (len=%d), Lab Time: '%s' (len=%d)\n", 
                myData.blood.c_str(), myData.blood.length(), myData.time.c_str(), myData.time.length());
  Serial.printf("Struct size: %d bytes\n", sizeof(myData));
  
  // แสดง Hex dump
  Serial.print("Hex dump: ");
  uint8_t* ptr = (uint8_t*)&myData;
  for(size_t i = 0; i < sizeof(myData); i++) {
    Serial.printf("%02X ", ptr[i]);
  }
  Serial.println();
  
  // ส่งซ้ำ 3 ครั้ง เพื่อเพิ่มโอกาสรับสำเร็จ
  for (int i = 0; i < 3; i++) {
    Serial.printf("Attempt %d/3...\n", i + 1);
    esp_now_send(receiverMAC, (uint8_t *) &myData, sizeof(myData));
    delay(500);  // รอ 500ms ระหว่างการส่งแต่ละครั้ง
  }

  Serial.println("Data Broadcast Sent! Switching back to AP mode...");
  delay(1000);
  
  // ปิด ESP-NOW
  esp_now_deinit();
  
  // Switch กลับไป AP mode
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  
  // Start Web Server ใหม่
  server.begin();
  Serial.println("Back to Web Server mode!");
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();
}