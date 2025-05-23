#include <Arduino.h>
#include "LcdDisplay.h"
#include "USB2UART.h"
#include "USBtoI2C.h"
#include "USBto1Wire.h"
#include "USBtoSPI.h"
#include "UARTtoI2C.h"
#include "UARTto1Wire.h"
#include "UARTtoSPI.h"

#define BUTTON_USB 0     // Nút chuyển các chế độ USB <-> ...
#define BUTTON_UART 4    // Nút chuyển các chế độ UART <-> ...

//git add -A; git commit -m " "; git push -u origin master    

// Chỉ định chế độ hoạt động
enum ModeType {
  MODE_USB,
  MODE_UART
};

enum USBMode {
  USB_UART,
  USB_I2C,
  USB_SPI,
  USB_ONEWIRE,
  USB_MODE_COUNT
};

enum UARTMode {
  UART_UART,
  UART_I2C,
  UART_SPI,
  UART_ONEWIRE,
  UART_MODE_COUNT
};

//Biến tốc độ truyền tải
uint32_t globaluartbaudrate = 115200;
uint32_t globali2cFrequency = 400000;
uint32_t globalspiFrequency = 1000000;
uint32_t onewirespeed = 16300;

// Biến toàn cục
volatile ModeType currentModeType = MODE_USB; // Mode mặc định là USB
volatile USBMode currentUSBMode = USB_UART;
volatile UARTMode currentUARTMode = UART_UART;
volatile bool usbButtonPressed = false;
volatile bool uartButtonPressed = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 100;

LcdDisplay lcd;

// ISR USB
void IRAM_ATTR ISR_usbButton() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    usbButtonPressed = true;
    lastDebounceTime = millis();
  }
}

// ISR UART
void IRAM_ATTR ISR_uartButton() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    uartButtonPressed = true;
    lastDebounceTime = millis();
  }
}

// Xử lý chuyển chế độ USB
void processUSBMode() {
  Serial.printf("[M] Chuyển chế độ USB: %d\n", currentUSBMode);
  lcd.clear();
  switch (currentUSBMode) {
    case USB_UART:
      Serial.println("Chế độ: USB <-> UART (serial2)");
      USB2UART_setup();
      break;
    case USB_I2C:
      Serial.println("Chế độ: USB <-> I2C (BH1750)");
      USBtoI2C_setup();
      break;
    case USB_SPI:
      Serial.println("Chế độ: USB <-> SPI (ADXL345)");
      USBtoSPI_setup();
      break;
    case USB_ONEWIRE:
      Serial.println("Chế độ: USB <-> 1-Wire (DS18B20)");
      USBto1Wire_setup();
      break;
    default:
      currentUSBMode = USB_UART;
      processUSBMode();
      break;
  }
}

// Xử lý chuyển chế độ UART
void processUARTMode() {
  lcd.clear();
  switch (currentUARTMode) {
    case UART_UART:
      Serial.println("Chế độ: UART1 <-> UART2");
      UART1_VS_UART2_setup();
      break;
    case UART_I2C:
      Serial.println("Chế độ: UART <-> I2C");
      UARTtoI2C_setup();
      break;
    case UART_SPI:
      Serial.println("Chế độ: UART <-> SPI");
      UARTtoSPI_setup();
      break;
    case UART_ONEWIRE:
      Serial.println("Chế độ: UART <-> 1-Wire");
      UARTto1Wire_setup();
      break;
    default:
      currentUARTMode = UART_UART;
      processUARTMode();
      break;
  }
}

void setup() {
  Serial.begin(globaluartbaudrate);
  while (!Serial && millis() < 5000);
  Wire.begin(21, 22, globali2cFrequency);
  Serial.println("[M] I2C bus initiliazed");

  // Cấu hình các nút nhấn
  pinMode(BUTTON_USB, INPUT_PULLUP);
  pinMode(BUTTON_UART, INPUT_PULLUP);

  // Đính kèm ngắt cho các nút nhấn
  attachInterrupt(digitalPinToInterrupt(BUTTON_USB), ISR_usbButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_UART), ISR_uartButton, FALLING);

  // Khởi tạo LCD
  lcd.init();

  // Khởi tạo chế độ mặc định
  currentModeType = MODE_USB;
  processUSBMode();

  Serial.println("Khởi động xong. Chế độ mặc định: USB <-> UART");
}

void loop() {
  // Xử lý nút USB
  if (usbButtonPressed) {
    usbButtonPressed = false;
    currentModeType = MODE_USB;
    currentUSBMode = (USBMode)((currentUSBMode + 1) % USB_MODE_COUNT);
    processUSBMode();
  }

  if (uartButtonPressed) {
    uartButtonPressed = false;
    currentModeType = MODE_UART;
    currentUARTMode = (UARTMode)((currentUARTMode + 1) % UART_MODE_COUNT);
    processUARTMode();
  }

  // Chỉ chạy loop của mode hiện tại
  if (currentModeType == MODE_USB) {
    switch (currentUSBMode) {
      case USB_UART: USB2UART_loop(); break;
      case USB_I2C: USBtoI2C_loop(); break;
      case USB_SPI: USBtoSPI_loop(); break;
      case USB_ONEWIRE: USBto1Wire_loop(); break;
    }
  } 
  else {
    switch (currentUARTMode) {
      case UART_UART: UART2UART_loop(); break;
      case UART_I2C: UARTtoI2C_loop(); break;
      case UART_SPI: UARTtoSPI_loop(); break;
      case UART_ONEWIRE: UARTto1Wire_loop(); break;
    }
  }

  delay(10);
}