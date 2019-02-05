# include <SPI.h>

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

const int slave_pin = 10;
String command = "00";
bool execute_command = false;
int eeprom_idx = 0;
byte eeprom_data[256];

SPISettings spiSettings(450000, MSBFIRST, SPI_MODE0);

void reset() {
  command = "00";
  execute_command = false;
  eeprom_idx = 0;
  memset(eeprom_data, 0, 256);
}

void setup() {
  Serial.begin(9600);
  pinMode(slave_pin, OUTPUT);
  SPI.begin();
}

void eeprom_write() {
  // Write enable first!
  SPI.beginTransaction(spiSettings);
  digitalWrite(slave_pin, HIGH);
  SPI.transfer(0x04);
  SPI.transfer(0xc0);
  digitalWrite(slave_pin, LOW);

  // Write 256 bytes
  for (int i=0; i <= 127; i++) {
    byte one = eeprom_data[i * 2];
    byte two = eeprom_data[i * 2 + 1];
    digitalWrite(slave_pin, HIGH);
    SPI.transfer(0x05); // Write command
    SPI.transfer(i); // Address 0
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
  SPI.endTransaction();
  reset();
  blink_led(1000);
  blink_led(250);
  blink_led(250);
  Serial.write("WRIT");
  Serial.flush();
}

void eeprom_read() {
  SPI.beginTransaction(spiSettings);
  digitalWrite(slave_pin, HIGH);
  SPI.transfer(0x06); // Write command
  SPI.transfer(0x00); // Address 0

  // This eeprom throws out a dummy bit on the first byte read.
  byte dangling_byte = SPI.transfer(0x00) << 1;

  // Read 256 bytes
  for (int i=0; i <= 254; i++){
    byte curbyte = SPI.transfer(0x00);
    Serial.write(curbyte >> 7 | dangling_byte);
    dangling_byte = curbyte << 1;
  }

  byte last_byte = SPI.transfer(0x00);
  Serial.write(last_byte >> 7 | dangling_byte);
  digitalWrite(slave_pin, LOW);
  SPI.endTransaction();
  Serial.flush();
  reset();
}

void buffer_load() {
  if (eeprom_idx < 255) {
    delay(1);
  } else {
    Serial.write("LOAD");
    Serial.flush();
    command = "00";
    execute_command = false;
    eeprom_idx = 0;
  }
}

void buffer_print() {
  Serial.write(eeprom_data, 256);
  Serial.flush();
  command = "00";
  execute_command = false;
  eeprom_idx = 0;
}

void blink_led(int ms) {
  int half = ms/2;
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(half);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(half);
}

void interoperableSerialEvent() {
  while (Serial.available()) {
    // get the new byte:
    byte inChar = Serial.read();
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (!execute_command & (char)inChar == '\n') {
      if (command == "r") {
        Serial.write("READ");
        Serial.flush();
      }
      if (command == "l") {
        eeprom_idx = 0;
      }
      if (command == "p") {
        Serial.write("PRNT");
        Serial.flush();
      }
      command += (char)inChar;
      execute_command = true;
    } else {
      if (command == "l\n") {
        eeprom_data[eeprom_idx] = inChar;
        eeprom_idx++;
      } else {
        // set the command
        command = (char)inChar;
      }
    }
  }
}

void loop() {
  interoperableSerialEvent();
  if (execute_command) {
    if (command == "r\n") {
      eeprom_read();
    }
    if (command == "w\n") {
      eeprom_write();
    }
    if (command == "l\n") {
      buffer_load();
    }
    if (command == "p\n") {
      buffer_print();
    }
    if (command == "0\n") {
      // TODO: More heavy handed reset, like reading everything left in
      // the serial buffer, and throwing it away, etc.      
      reset();
      Serial.write("Reset");
      Serial.flush();
      blink_led(2000);
    }
  }
}
