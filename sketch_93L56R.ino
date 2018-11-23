# include <SPI.h>

const int slave_pin = 10;
const int clock_pin = 13;
char command = '0';
String eeprom_data = "";
bool read = false;
bool write = false;

SPISettings spiSettings(450000, MSBFIRST, SPI_MODE0);

void setup() {
  Serial.begin(9600);
  eeprom_data.reserve(256);
  pinMode(slave_pin, OUTPUT);
  SPI.begin();
}

void eeprom_write() {
  SPI.beginTransaction(spiSettings);
  digitalWrite(slave_pin, HIGH);
  SPI.transfer(6); // Write command
  SPI.transfer(0); // Address 0

  // Read 256 bytes
  for (int i=0; i <= 255; i++){
      byte curbyte = SPI.transfer(0);
      Serial.write(curbyte >> 7 | dangling_byte);
      dangling_byte = curbyte << 1;
  }
  digitalWrite(slave_pin, LOW);
  SPI.endTransaction();
  write = false;
}

void eeprom_read() {
  SPI.beginTransaction(spiSettings);
  digitalWrite(slave_pin, HIGH);
  SPI.transfer(6); // Write command
  SPI.transfer(0); // Address 0

  // This eeprom throws out a dummy bit on the first byte read.
  byte dangling_byte = SPI.transfer(0) << 1;

  // Read 256 bytes
  for (int i=0; i <= 254; i++){
      byte curbyte = SPI.transfer(0);
      Serial.write(curbyte >> 7 | dangling_byte);
      dangling_byte = curbyte << 1;
  }

  byte last_byte = SPI.transfer(0);
  Serial.write(last_byte >> 7 | dangling_byte);
  digitalWrite(slave_pin, LOW);
  SPI.endTransaction();
  read = false;
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      if (command == 'r') {
        read = true;
      }
    }
    // add it to the command:
    command = inChar;
  }
}

void loop() {
  if (read) {
    eeprom_read();
  }
}
