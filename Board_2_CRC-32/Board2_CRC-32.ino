#include <HardwareSerial.h>
#include <string.h>

HardwareSerial SerialLP1(PB7, PB6);

uint32_t calculateCRC32(const String& data);

void setup() {
  Serial.begin(9600);
  SerialLP1.begin(9600);
  delay(1000);
  //Serial.println("Board 2 setup complete");
}


void loop() {
  // Check if data is available from Board 1
  if (SerialLP1.available()) {
    // Read the incoming message from Board 1 until newline character
    String receivedMessage = SerialLP1.readStringUntil('\n');
    Serial.print("Received from Board 1: ");
    Serial.println(receivedMessage);
    // Ensure the received message is longer than 1 character (data + CRC)
    if (receivedMessage.length() > 8) {
      // Extract the data part of the message
      String data = receivedMessage.substring(0, receivedMessage.length() - 8);
      // Extract the received CRC value (last 8 characters)
      String receivedCRCStr = receivedMessage.substring(receivedMessage.length() - 8);
      uint32_t receivedCRC = strtoul(receivedCRCStr.c_str(), NULL, 16);
      // Calculate the CRC for the data part
      uint32_t calculatedCRC = calculateCRC32(data);
      // Compare the received CRC with the calculated CRC
      if (receivedCRC == calculatedCRC) {
        // If CRC matches, print and forward the message to the PC
        Serial.print("CRC Match. Data: ");
        Serial.println(data);
      } else {
        // If CRC does not match, request retransmission
        SerialLP1.println("NACK");
        Serial.println("Error detected, requesting retransmission");
      }
    }
  }
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
