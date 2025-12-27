# Blood Test Simulation - ESP-NOW Wireless Glucose Meter

โปรเจคเครื่องวัดน้ำตาลในเลือดแบบไร้สาย ใช้ ESP-NOW สำหรับส่งข้อมูลระหว่าง Server และ Node

## 📋 ส่วนประกอบ

### 1. **Blood_test_Server.ino** (Transmitter)
- ESP8266/NodeMCU ตัวส่งข้อมูล
- จำลองค่าน้ำตาล (80-200 mg/dL)
- ส่งข้อมูลผ่าน ESP-NOW ทุก 5 วินาที

### 2. **Blood_test_Node.ino** (Receiver/Display)
- ESP8266/NodeMCU ตัวรับและแสดงผล
- จอ LCD ST7567 128x64
- แสดงค่าน้ำตาลด้วย 7-segment font

## ⚡ ฟีเจอร์ Node (Display)

### 🎯 การแสดงผล
- ✅ **7-Segment Digital Font** - เหมือนเครื่องวัดจริง
- ✅ **Read/Unread Indicator** - เครื่องหมาย ✓ (ข้อมูลใหม่) และ "R" (อ่านแล้ว)
- ✅ **Time Display** - แสดงเวลาจาก Server
- ✅ **Battery Indicator** - แสดง % แบตเตอรี่

### 🔘 ปุ่มควบคุม (3 ปุ่ม)
- **SW1 (GPIO2)** - กดแสดงค่าน้ำตาล (5 วินาที)
- **SW2 (GPIO12)** - สลับระดับไฟหน้าจอ (Dim ↔ Bright)
- **SW3 (GPIO0)** - กดค้าง 3 วิ แสดง MAC/RSSI, กดค้าง 5 วิ เข้า OTA Mode

### 💤 Auto Sleep Mode
- หน้าจอดับอัตโนมัติหลัง **60 วินาที** ไม่มีการใช้งาน
- ประหยัดพลังงานและยืดอายุจอ LCD
- กดปุ่มใดก็ได้เพื่อปลุก หรือรับข้อมูลใหม่จะปลุกอัตโนมัติ

### 📶 MAC/RSSI Display
- กด SW3 ค้าง 3 วินาที
- แสดง MAC Address (2 บรรทัด)
- แสดงระดับสัญญาณ WiFi RSSI และคุณภาพ

### 🔄 OTA Update
- กด SW3 ค้าง 5 วินาที เข้า OTA Mode
- เชื่อมต่อ WiFi AP: "OTAUPATE" (password: 12345678)
- อัพโหลด firmware ผ่าน Web: http://192.168.4.1

## 🛠️ Hardware Requirements

### Node (Display):
- ESP8266 NodeMCU
- ST7567 LCD 128x64
- 3 ปุ่มกด (SW1, SW2, SW3)
- Voltage divider สำหรับวัดแบต (optional)

### Server (Transmitter):
- ESP8266 NodeMCU

## 📚 Library Dependencies

สำหรับ **PlatformIO**:
```ini
lib_deps = 
    arkhipenko/TaskScheduler@^3.7.0
    ayushsharma82/ElegantOTA@^3.1.7
    ottowinter/ESPAsyncWebServer-esphome@^3.2.2
    https://github.com/cbm80amiga/ST7567_FB
    https://github.com/cbm80amiga/PropFonts
```

สำหรับ **Arduino IDE**:
- TaskScheduler
- ElegantOTA
- ESPAsyncWebServer
- ST7567_FB (Manual install from GitHub)
- PropFonts (Manual install from GitHub)

## 🔌 Pin Connections (Node)

```
ST7567 LCD:
- LCD_DC       → GPIO15 (D8)
- LCD_RST      → GPIO5  (D1)
- LCD_CS       → GPIO16 (D0)
- LCD_BACKLIGHT→ GPIO4  (D2)

Buttons:
- SW1 → GPIO2  (D4)
- SW2 → GPIO12 (D6)
- SW3 → GPIO0  (Flash button)

Battery (Optional):
- ADC → A0 (voltage divider)
```

## 📖 การใช้งาน

### 1. Setup Server:
1. เปิด `Blood_test_Server.ino`
2. ใส่ **MAC Address ของ Node** ในโค้ด
3. Upload ไป ESP8266 ตัวที่ 1

### 2. Setup Node:
1. เปิด `Blood_test_Node.ino`
2. ต่อจอ LCD ตาม Pin Connections
3. Upload ไป ESP8266 ตัวที่ 2
4. เปิด Serial Monitor ดู MAC Address ของ Node
5. Copy MAC Address ไปใส่ใน Server code

### 3. ทดสอบ:
- Server จะส่งข้อมูลอัตโนมัติทุก 5 วินาที
- Node แสดงเครื่องหมาย ✓ เมื่อมีข้อมูลใหม่
- กด SW1 เพื่อดูค่าน้ำตาล
- หลัง 5 วิ จะกลับเป็น Black Strip และแสดง "R"

## 🎨 Display Modes

### Black Strip Mode (ปกติ):
- แถบดำกลางจอ
- แสดง "R" (อ่านแล้ว) หรือ ✓ (ข้อมูลใหม่)
- แสดงเวลาด้านบน

### Blood Value Mode (กด SW1):
- แสดงค่าน้ำตาลด้วย 7-segment font ขนาดใหญ่
- แสดงหน่วย "mg/dL"
- แสดง 5 วินาที แล้วกลับ Black Strip

### Info Mode (กด SW3 ค้าง 3 วิ):
- MAC Address (2 บรรทัด)
- RSSI signal strength
- Signal quality (Excellent/Good/Fair/Weak)

### Sleep Mode (ไม่มีกิจกรรม 60 วิ):
- หน้าจอดำ
- ปิด backlight
- กดปุ่มใดก็ได้เพื่อปลุก

## 📝 Files

- `Blood_test_Server.ino` - Server/Transmitter code
- `Blood_test_Node.ino` - Node/Display code
- `font8x8.h` - PropFont-compatible 8x8 font
- `ST7567_FB.h` - LCD library header (backup)
- `c64enh_font.h` - C64 enhanced font (legacy)

## 🔧 Development

### PlatformIO Setup:
```bash
# Clone repository
git clone https://github.com/SANCHAIE/bloodtestsimulation.git

# Use PlatformIO project
cd bloodtestsimulation-node
pio run --target upload
```

### Arduino IDE Setup:
1. Install ESP8266 board support
2. Install required libraries
3. Open .ino file
4. Select "NodeMCU 1.0 (ESP-12E Module)"
5. Upload

## 📊 Memory Usage

```
RAM:   [====      ] 38.3% (31,336 bytes)
Flash: [===       ] 32.4% (338,471 bytes)
```

## 🔄 Version History

### v2.0 (Current - SE-dev branch)
- ✅ 7-segment digital font
- ✅ Read/unread data indicators
- ✅ MAC/RSSI display (SW3 hold 3s)
- ✅ Auto sleep mode (60s timeout)
- ✅ Battery indicator
- ✅ Data state management
- ✅ Wake from sleep on button/data

### v1.0
- Basic ESP-NOW communication
- Simple LCD display
- OTA update support

## 📄 License

MIT License

## 👤 Author

SANCHAI (SE-dev)