# Arduino-MAX7219-Ticker
## Objective of the project
Learn how to work with wireless data transmission using bluetooth technology on the example of creating an android application for managing a ticker.

### Tasks
1. Development of a mobile android application for data exchange using bluetooth technology.
2. Development of firmware and construction of a circuit of elements.

## The main controller: Arduino NANO
Arduino NANO is a compact microelectronic prototyping platform designed for use with a breadboard.
The basis of the Arduino Nano is an ATmega328-based microcontroller, a logic chip for processing data with a clock frequency of 16 MHz, which has 8 analog and 14 general-purpose digital pins on board, as well as all the necessary interfaces: I2C, SPI and UART.
![alt tag](https://github.com/Thermazote/Arduino-MAX7219-Ticker/raw/develop/Pics/Arduino_NANO.jpg)
## Dot matrix controller: MAX7219
The MAX7219/MAX7221 are compact, serial input/output common-cathode display drivers that interface
microprocessors (µPs) to 7-segment numeric LED displays of up to 8 digits, bar-graph displays, or 64 individual LEDs. Included on-chip are a BCD code-B
decoder, multiplex scan circuitry, segment and digit
drivers, and an 8x8 static RAM that stores each digit.
Only one external resistor is required to set the segment current for all LEDs. The MAX7221 is compatible
with SPI™, QSPI™, and MICROWIRE™, and has slewrate-limited segment drivers to reduce EMI.
![alt tag](https://github.com/Thermazote/Arduino-MAX7219-Ticker/raw/develop/Pics/MAX7219_LED8x32.jpg)
## Bluetooth module: HC-06
HC-06 is a Bluetooth module designed for establishing short range wireless data communication between two systems. The module works on Bluetooth 2.0 communication protocol and it can only act as a slave device. This is cheapest method for wireless data transmission and more flexible compared to other methods and it even can transmit files at speed up to 2.1Mb/s.
HC-06 uses frequency hopping spread spectrum technique (FHSS) to avoid interference with other devices and to have full duplex transmission. The device works on the frequency range from 2.402 GHz to 2.480GHz.
![alt tag](https://github.com/Thermazote/Arduino-MAX7219-Ticker/raw/develop/Pics/HC-06.jpg)
## Circuit of elements
![alt tag](https://github.com/Thermazote/Arduino-MAX7219-Ticker/raw/develop/Pics/Scheme.png)
## Firmware design features
* The firmware uses the library **SofwareSerial**. Macro `_SS_MAX_RX_BUFF` is redefined from `64` to `256`. This is necessary to increase the buffer size from 64 to 256 bytes. Such a measure is necessary in order for the data packet to fit into the buffer. File **SofwareSerial.h** is in _"C:\Program Files (x86)\Arduino\hardware\arduino\avr\libraries\SoftwareSerial\src"_ path.
## Android application development
Application powered by [Kotlin](https://kotlinlang.org/)
![alt tag](https://github.com/Thermazote/Arduino-MAX7219-Ticker/raw/develop/Pics/App_LOGO.png)
