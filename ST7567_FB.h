/*
 * ST7567_FB - ST7567 LCD Driver with FrameBuffer
 * For 128x64 Monochrome LCD Display
 */

#ifndef ST7567_FB_h
#define ST7567_FB_h

#include <Arduino.h>
#include <SPI.h>

// Alignment constants
#define ALIGN_LEFT    0
#define ALIGN_CENTER  1
#define ALIGN_RIGHT   2

class ST7567_FB {
  public:
    ST7567_FB(uint8_t dc, uint8_t rst, uint8_t cs);
    
    void init();
    void cls();
    void display();
    void drawBitmap(const uint8_t* bitmap, uint8_t x, uint8_t y);
    void setFont(const uint8_t* font);
    void printStr(uint8_t align, uint8_t y, const char* str);
    void printStr(uint8_t align, uint8_t y, char* str);
    void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
    void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
    void setPixel(uint8_t x, uint8_t y, uint8_t color);
    
  private:
    uint8_t _dc, _rst, _cs;
    uint8_t framebuffer[1024]; // 128x64/8
    const uint8_t* currentFont;
    
    void writeCommand(uint8_t cmd);
    void writeData(uint8_t data);
    void setPosition(uint8_t x, uint8_t y);
    uint8_t getCharWidth(char c);
    uint8_t getStringWidth(const char* str);
};

// Constructor
ST7567_FB::ST7567_FB(uint8_t dc, uint8_t rst, uint8_t cs) {
  _dc = dc;
  _rst = rst;
  _cs = cs;
  currentFont = nullptr;
}

// Initialize LCD
void ST7567_FB::init() {
  pinMode(_dc, OUTPUT);
  pinMode(_rst, OUTPUT);
  pinMode(_cs, OUTPUT);
  
  digitalWrite(_cs, HIGH);
  digitalWrite(_rst, HIGH);
  delay(10);
  digitalWrite(_rst, LOW);
  delay(10);
  digitalWrite(_rst, HIGH);
  delay(10);
  
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  
  // ST7567 init sequence
  writeCommand(0xE2); // Reset
  writeCommand(0xA2); // Bias 1/9
  writeCommand(0xA0); // ADC normal
  writeCommand(0xC8); // COM output reverse
  writeCommand(0xA6); // Display normal
  writeCommand(0xA4); // Display all points off
  writeCommand(0x2F); // Power control
  writeCommand(0x27); // Contrast
  writeCommand(0x81); // Set contrast
  writeCommand(0x16); // Contrast value
  writeCommand(0xAF); // Display on
  
  cls();
  display();
}

// Clear screen
void ST7567_FB::cls() {
  memset(framebuffer, 0, sizeof(framebuffer));
}

// Write command
void ST7567_FB::writeCommand(uint8_t cmd) {
  digitalWrite(_dc, LOW);
  digitalWrite(_cs, LOW);
  SPI.transfer(cmd);
  digitalWrite(_cs, HIGH);
}

// Write data
void ST7567_FB::writeData(uint8_t data) {
  digitalWrite(_dc, HIGH);
  digitalWrite(_cs, LOW);
  SPI.transfer(data);
  digitalWrite(_cs, HIGH);
}

// Set cursor position
void ST7567_FB::setPosition(uint8_t x, uint8_t y) {
  writeCommand(0x10 | ((x >> 4) & 0x0F));
  writeCommand(0x00 | (x & 0x0F));
  writeCommand(0xB0 | (y & 0x0F));
}

// Update display
void ST7567_FB::display() {
  for (uint8_t page = 0; page < 8; page++) {
    setPosition(0, page);
    for (uint8_t col = 0; col < 128; col++) {
      writeData(framebuffer[page * 128 + col]);
    }
  }
}

// Set pixel
void ST7567_FB::setPixel(uint8_t x, uint8_t y, uint8_t color) {
  if (x >= 128 || y >= 64) return;
  
  uint16_t pos = (y / 8) * 128 + x;
  uint8_t bit = y % 8;
  
  if (color) {
    framebuffer[pos] |= (1 << bit);
  } else {
    framebuffer[pos] &= ~(1 << bit);
  }
}

// Fill rectangle
void ST7567_FB::fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
  for (uint8_t i = 0; i < w; i++) {
    for (uint8_t j = 0; j < h; j++) {
      setPixel(x + i, y + j, color);
    }
  }
}

// Draw rectangle
void ST7567_FB::drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
  for (uint8_t i = 0; i < w; i++) {
    setPixel(x + i, y, color);
    setPixel(x + i, y + h - 1, color);
  }
  for (uint8_t i = 0; i < h; i++) {
    setPixel(x, y + i, color);
    setPixel(x + w - 1, y + i, color);
  }
}

// Draw bitmap
void ST7567_FB::drawBitmap(const uint8_t* bitmap, uint8_t x, uint8_t y) {
  uint8_t width = pgm_read_byte(bitmap);
  uint8_t height = pgm_read_byte(bitmap + 1);
  
  for (uint8_t j = 0; j < (height / 8); j++) {
    for (uint8_t i = 0; i < width; i++) {
      uint8_t data = pgm_read_byte(bitmap + 2 + j * width + i);
      for (uint8_t bit = 0; bit < 8; bit++) {
        if (data & (1 << bit)) {
          setPixel(x + i, y + j * 8 + bit, 1);
        }
      }
    }
  }
}

// Set font
void ST7567_FB::setFont(const uint8_t* font) {
  currentFont = font;
}

// Get character width
uint8_t ST7567_FB::getCharWidth(char c) {
  if (!currentFont) return 8;
  return pgm_read_byte(currentFont); // Width from font data
}

// Get string width
uint8_t ST7567_FB::getStringWidth(const char* str) {
  if (!currentFont) return strlen(str) * 8;
  
  uint8_t width = pgm_read_byte(currentFont);
  return strlen(str) * width;
}

// Print string
void ST7567_FB::printStr(uint8_t align, uint8_t y, const char* str) {
  if (!currentFont || !str) return;
  
  uint8_t width = pgm_read_byte(currentFont);
  uint8_t height = pgm_read_byte(currentFont + 1);
  uint8_t strWidth = getStringWidth(str);
  uint8_t x = 0;
  
  if (align == ALIGN_CENTER) {
    x = (128 - strWidth) / 2;
  } else if (align == ALIGN_RIGHT) {
    x = 128 - strWidth;
  }
  
  for (uint8_t i = 0; str[i] != '\0'; i++) {
    char c = str[i];
    uint8_t charIndex = c - 32; // ASCII offset
    
    for (uint8_t col = 0; col < width; col++) {
      for (uint8_t row = 0; row < height; row++) {
        uint8_t data = pgm_read_byte(currentFont + 2 + (charIndex * width * (height / 8)) + (row / 8) * width + col);
        if (data & (1 << (row % 8))) {
          setPixel(x + col, y + row, 1);
        }
      }
    }
    x += width;
  }
}

void ST7567_FB::printStr(uint8_t align, uint8_t y, char* str) {
  printStr(align, y, (const char*)str);
}

#endif
