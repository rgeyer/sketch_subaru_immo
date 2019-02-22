#include <PacketSerial.h>
#include <SPI.h>
#include <Wire.h>

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
uint8_t MW_READ_ACK_PACKET[1] = {129};
uint8_t MW_WRITE_ACK_PACKET[1] = {130};

const int slave_pin = 10;
PacketSerial pSer;

SPISettings spiSettings(450000, MSBFIRST, SPI_MODE0);

void reset() {
  // TODO: Is there really any need for this? Nothing to reset, really...
  pSer.send(RESET_ACK_PACKET, 1);
}

void onSerPacketReceived(const uint8_t* buffer, size_t size) {
  unsigned int devAddr = 0x50;
  unsigned int addr = 0;
  unsigned int len = 256;
  switch (buffer[0]) {
    case 0x00: // RESET_REQ_PACKET Command ID
      reset();
      break;
    case 0x01: // MW_READ_PACKET Command ID
      addr = (int)(buffer[1] << 8) | (int)(buffer[2]);
      len = (int)(buffer[3] << 8) | (int)(buffer[4]);
      mw_eeprom_read(addr, len);
      break;
    case 0x02: // MW_WRITE_PACKET Command ID
      addr = (int)(buffer[1] << 8) | (int)(buffer[2]);
      len = (int)(buffer[3] << 8) | (int)(buffer[4]);
      mw_eeprom_write(addr, len, buffer);
      break;
    case 0x03: // I2C_READ_PACKET Command ID
      devAddr = buffer[1];
      addr = (int)(buffer[2] << 8) | (int)(buffer[3]);
      len = (int)(buffer[4] << 8) | (int)(buffer[5]);
      i2c_eeprom_read(devAddr, addr, len);
      break;
  }
}

void setup() {
  Serial.begin(9600);
  pSer.setStream(&Serial);
  pSer.setPacketHandler(&onSerPacketReceived);
  pinMode(slave_pin, OUTPUT);
  SPI.begin();
  Wire.begin();
}

void mw_eeprom_write(unsigned int addr, unsigned int len, uint8_t* buffer) {
  // Write enable first!
  SPI.beginTransaction(spiSettings);
  digitalWrite(slave_pin, HIGH);
  SPI.transfer(0x04);
  SPI.transfer(0xc0);
  digitalWrite(slave_pin, LOW);

  // Write length bytes
  for(int i=0; i < len/2; i++) {
    byte curAddr = i+addr;
    byte oneIdx = i*2+5;
    byte twoIdx = i*2+6;
    byte one = buffer[oneIdx];
    byte two = buffer[twoIdx];

    digitalWrite(slave_pin, HIGH);
    SPI.transfer(0x05); // Write command
    SPI.transfer(curAddr);
    SPI.transfer(one);
    SPI.transfer(two);
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
  }
  pSer.send(MW_WRITE_ACK_PACKET, 1);
  SPI.endTransaction();
}

void mw_eeprom_read(unsigned int addr, unsigned int length) {
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

void i2c_eeprom_read(int devAddr, int addr, int len) {
  // TODO: Maybe? This is a big heavy handed and inelegant, but it covers
  // all possible scenarios and stays within the 32byte I2C limit of the
  // Arduino. This could probably be more elegantly expressed with a single
  // loop if I spent a few more cycles thinking about it and testing.
  //
  // See: https://github.com/arduino/Arduino/issues/5871

  // TODO: This only works with I2C EEPROMs with up to 256 addresses since it
  // only sends a single address byte in the read request. Need to allow the
  // write command to indicate the eeprom size, so that we can determine how
  // many bytes to send for the address in the read request.
  int loops = len/32;
  int remainder = len % 32;
  int i = 0;
  for (int l=0; l < loops; l++) {
    Wire.beginTransmission(devAddr);
    Wire.write(addr+(l*32));
    Wire.endTransmission();
    Wire.requestFrom(devAddr,32);
    for (i=i; i < l*32+32; i++) {
      if(Wire.available()) {
        Serial.write(Wire.read());
      } else {
        Serial.write(i);
      }
    }
  }
  Wire.beginTransmission(devAddr);
  Wire.write(addr+(loops*32));
  Wire.endTransmission();
  Wire.requestFrom(devAddr,remainder);
  for (i=i; i < loops*32+remainder; i++) {
    if(Wire.available()) {
      Serial.write(Wire.read());
    } else {
      Serial.write(i);
    }
  }
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
