#include <PacketSerial.h>
#include <SPI.h>

#if defined(TEENSYDUINO)
  #if defined(__AVR_ATmega32U4__)
    int led = 11;
  #else
    #error
  #endif
#elif defined(ARDUINO_ARCH_AVR)
  int led = 13;
#else
  #error
#endif

uint8_t RESET_REQ_PACKET[1] = {0};
uint8_t RESET_ACK_PACKET[1] = {128};
uint8_t READ_ACK_PACKET[1] = {129};
uint8_t WRITE_ACK_PACKET[1] = {130};

const int slave_pin = 10;
PacketSerial pSer;

SPISettings spiSettings(450000, MSBFIRST, SPI_MODE0);

void reset() {
  // TODO: Is there really any need for this? Nothing to reset, really...
  pSer.send(RESET_ACK_PACKET, 1);
}

void onSerPacketReceived(const uint8_t* buffer, size_t size) {
  unsigned int addr = 0;
  unsigned int len = 256;
  switch (buffer[0]) {
    case 0x00:
      reset();
      break;
    case 0x01:
      if (size >= 3) {
        addr = (int)(buffer[1] << 8) | (int)(buffer[2]);
      }
      if (size >= 5) {
        len = (int)(buffer[3] << 8) | (int)(buffer[4]);
      }
      eeprom_read(addr, len);
      break;
    case 0x02:
      addr = (int)(buffer[1] << 8) | (int)(buffer[2]);
      eeprom_write(addr, buffer);
      break;
  }
}

void setup() {
  Serial.begin(9600);
  pSer.setStream(&Serial);
  pSer.setPacketHandler(&onSerPacketReceived);
  pinMode(slave_pin, OUTPUT);
  SPI.begin();
}

void eeprom_write(unsigned int addr, uint8_t* buffer) {
  // Write enable first!
  SPI.beginTransaction(spiSettings);
  digitalWrite(slave_pin, HIGH);
  SPI.transfer(0x04);
  SPI.transfer(0xc0);
  digitalWrite(slave_pin, LOW);

  // Write length bytes
  digitalWrite(slave_pin, HIGH);
  SPI.transfer(0x05); // Write command
  SPI.transfer(addr);
  SPI.transfer(buffer[3]);
  SPI.transfer(buffer[4]);
  digitalWrite(slave_pin, LOW);
  // Wait for a "ready state"
  delay(5);
  digitalWrite(slave_pin, HIGH);
  byte status = 0x00;
  while(status == 0x00) {
    delay(5);
    status = SPI.transfer(0x00);
  }
  digitalWrite(slave_pin, LOW);
  pSer.send(WRITE_ACK_PACKET, 1);
  SPI.endTransaction();
}

void eeprom_read(unsigned int addr, unsigned int length) {
  Serial.flush();
  SPI.beginTransaction(spiSettings);
  digitalWrite(slave_pin, HIGH);
  SPI.transfer(0x06); // Write command
  SPI.transfer(addr); // Address 0

  // This eeprom throws out a dummy bit on the first byte read.
  byte dangling_byte = SPI.transfer(0x00) << 1;

  // Read length bytes
  for (int i=0; i <= length-2; i++){
    byte curbyte = SPI.transfer(0x00);
    Serial.write(curbyte >> 7 | dangling_byte);
    dangling_byte = curbyte << 1;
  }

  byte last_byte = SPI.transfer(0x00);
  Serial.write(last_byte >> 7 | dangling_byte);
  digitalWrite(slave_pin, LOW);
  SPI.endTransaction();
  Serial.flush();
}

void blink_led(int ms) {
  int half = ms/2;
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(half);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(half);
}

void loop() {
  pSer.update();
}
