# Sketch Subaru IMMO
Assets for PCBs, and arduino sketch(es) relating to hacking my 2005 Subaru Outback XT engine computer.

## EAGLE
The `eagle` directory contains the PCBs. Including the following.

1. `immo_programmer` - A (reasonbly mature) immobilizer EEPROM programming tool.
2. `Bench` - A PCB which is designed to allow bench testing of the ECU/BIU/CM, which breaks out key signals (CANbus, IMMO serial, security light, etc), and includes a Teensy 3.5/3.6 for reading and manipulating singnals.
3. `ArduinoLogger` - A VERY incomplete prototype for logging CANbus, SSM, and immobilizer signals. Meant as a one-stop-shop for all data logging. Still needs lots of work.

## Sketches
There is really only one sketch, which is in the root of the repo currently. It's the arduino firmware for the `immo_programmer` board. It is meant to be interfaced by the [93l56r-cli](https://github.com/rgeyer/93l56r-cli/) golang app to read/write the EEPROMs related to the immobilizer system.

## Fritzing diagrams
The `fritzing` directory includes visual diagrams for the wiring schematics on breadboards, for those who may not be using my manufactured PCB.