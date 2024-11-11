#include <HardwareSerial.h>
#include <string.h>
#define MAX_TRANSMISSION_ATTEMPTS 10 
//#include "stm32h7xx.h"


// Define a HardwareSerial object for the second LPUART interface
HardwareSerial SerialLP1(PB7, PB6);

// Function heads
uint32_t calculateCRC32(const String& data);
void restartProgram();
void sendMessage(const String& message);
void sendMessageWrong(const String& message);

// Globals
String lastMessage;
unsigned int attempts = 0;

void setup() {
  // Initialize serial communication with the PC
  Serial.begin(9600);
  // Initialize LPUART communication with Board 2
  SerialLP1.begin(9600);
  // Delay to establish the connection
  delay(1000);
  //Serial.println("Board 1 setup complete");
}

void loop() {
  // Check if data is available from the PC
  if (Serial.available()) {
    lastMessage = Serial.readStringUntil('\n');
    attempts = 0; // Reset for new messages
    sendWrongMessage(lastMessage);
    attempts++;
  }

  // Check if data is available from Board 2 (NACK response)
  if (SerialLP1.available()) {
    String response = SerialLP1.readStringUntil('\n');
    response.trim();  // Remove any leading/trailing whitespace or control characters
    Serial.print("Received from Board 2: ");
    Serial.println(response);
    if (response == "NACK") {
      if (attempts >= MAX_TRANSMISSION_ATTEMPTS) {
        Serial.println("Max attempts reached, stopping the program and reseting.");
        delay(1000);
        restartProgram();
      } else {
        Serial.println("NACK received, resending message");
        sendWrongMessage(lastMessage);
        attempts++;
      }
    }
  }
}
// Function to simulate transmission error for testing
void sendWrongMessage(const String& message) {
  uint32_t crc = calculateCRC32(message);
  char crcString[9];
  snprintf(crcString, sizeof(crcString), "%08X", crc);  // Convert CRC to 8-character hex string
  // here the error
  String messageWithCRC = message + String(crcString) + 'e';
  SerialLP1.print(messageWithCRC + '\n');  // Ensure newline character is sent
  Serial.print("Sent to Board 2: ");
  Serial.println(messageWithCRC);
}

void sendMessage(const String& message) {
  uint32_t crc = calculateCRC32(message);
  char crcString[9];
  snprintf(crcString, sizeof(crcString), "%08X", crc);  // Convert CRC to 8-character hex string
  String messageWithCRC = message + String(crcString);
  SerialLP1.print(messageWithCRC + '\n');  // Ensure newline character is sent
  Serial.print("Sent to Board 2: ");
  Serial.println(messageWithCRC);
}

void restartProgram() {
    // Set the SYSRESETREQ bit in the AIRCR register to request a system reset
    NVIC_SystemReset();
}

// Function to calculate CRC-32 using polynomial 0xEDB88320
uint32_t calculateCRC32(const String& data) {
  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < data.length(); i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 1) {
        crc = (crc >> 1) ^ 0xEDB88320;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc ^ 0xFFFFFFFF;
}
