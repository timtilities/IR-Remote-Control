/*
  This uses interrupts to wake from sleep, flash an LED, then go back to sleep... using a 3x4 keypad

  #### Keypad Setup ####
          7 - D6 (pin 7)	- Col 3
          6 - D5 (pin 8)	- Col 2
  Keypad  5 - D4 (pin 9) 	- Col 1
  Conn    4 - D3 (pin 10) 	- Row 4
          3 - D2 (pin 11) 	- Row 3
          2 - D1 (pin 12) 	- Row 2
          1 - D0 (pin 13) 	- Row 1
*/

#include "PinDefinitionsAndMore.h"  // Define macros for input and output pin etc.
/* These must be set BEFORE including IRremote.hpp! */
#define IR_SEND_PIN 7 // pin 6

/* Requirements:
*  https://github.com/Arduino-IRremote/Arduino-IRremote/
*  https://github.com/Chris--A/Keypad
*  https://github.com/NicoHood/PinChangeInterrupt  
*/
#include <TinyIRSender.hpp>
#include <avr/sleep.h>
#include <Keypad.h>
#include "PinChangeInterrupt.h"

/* number of items in an array */
#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

/* LED pin */
#define LED 8
/* How long should LED stay on after sending IR signals */
#define LED_WAIT 100

/* Initialize keypad matrix variables */
const byte ROWS = 4; 
const byte COLS = 3; 
char hexaKeys[] = {
  "123"
  "456"
  "789"
  "*0#"
};
byte rowPins[ROWS] = {0, 1, 2, 3}; 
byte colPins[COLS] = {4, 5, 6}; 
/* This must be done from loop() to avoid interrupt issues */
/* Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); */

/* Initialize a callback variable */
byte Pressed = 0;

void setup() {
  /* set ROW pins to input with a pullup */
  for (byte i = 0; i < NUMITEMS (rowPins); i++){
    pinMode (rowPins[i], INPUT_PULLUP);
  }
  
  /* Initialize LED pin */
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  /* Attach the new PinChangeInterrupt and enable event function below */
  for (byte i = 0; i < NUMITEMS (rowPins); i++){
    attachPCINT(digitalPinToPCINT(rowPins[i]), WakeUp, CHANGE);
  }

  /* ATTiny84 sleep */
  ADCSRA &= ~(1<<ADEN);			// Disable ADC
  PRR = 1<<PRUSI | 1<<PRADC;	// Turn off clocks to unused peripherals
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void WakeUp(void) {
  Pressed = 1;
}

void loop() {
  /* Initialize IRCode */
  uint8_t IRCode;

  if (Pressed == 1){
    /* Restore Keypad operation after wake */
    for (byte i = 0; i < NUMITEMS (rowPins); i++){
      pinMode (rowPins[i], INPUT_PULLUP);
    }
    for (byte i = 0; i < NUMITEMS (colPins); i++){
      pinMode (colPins[i], INPUT_PULLUP);
    }

    /* Initialize Keypad class - this must be done here (rather than in setup) to avoid interrupt issues) */
    Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
    
	/* Determine which button was pressed and send appropriate IR commands */
    /*char KeyPressed = customKeypad.getKey(); */
    switch (customKeypad.getKey()) {
      case '1':
        IRCode = 0xC;
        break;
      /*case '2':
        IRCode = 0x??;
        break;*/
      case '3':
        IRCode = 0x8;
        break;
      case '4':
        IRCode = 0x7;
        break;
      /*case '5':
        IRCode = 0x??;
        break;*/
      case '6':
        IRCode = 0x6;
        break;
      case '7':
        IRCode = 0x1C;
        break;
      /*case '8':
        IRCode = 0x??;
        break;*/
      case '9':
        digitalWrite(LED, HIGH);
        IRCode = 0x15;
        break;
      /*case '0':
        IRCode = 0x??;
        break;
      case '*':
        IRCode = 0x??;
        break;
      case '#':
        IRCode = 0x??;
        break;
      case 'A':
        IRCode = 0x??;
        break;
      case 'B':
        IRCode = 0x??;
        break;
      case 'C':
        IRCode = 0x??;
        break;
      case 'D':
        IRCode = 0x??;
        break;*/
      default: /* Keypress was not recognized/invalid */
        IRCode = 0;
    }
    if (IRCode != 0){
      digitalWrite(LED, HIGH);
      sendNEC(IR_SEND_PIN, 0xCF10, IRCode, 0);
      delay(LED_WAIT);
      digitalWrite(LED, LOW);
    }
  }  

  /* Clear callback variable */
  Pressed = 0;

  /* debounce */
  delay(100);
  
  /* Reset pins for interrupts */
  for (byte i = 0; i < NUMITEMS (rowPins); i++){
    pinMode (rowPins[i], INPUT_PULLUP);
  }
  for (byte i = 0; i < NUMITEMS (colPins); i++){
    pinMode (colPins[i], OUTPUT); digitalWrite (colPins[i], LOW);
  }
    
  /* Going to sleep */
  sleep_enable();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);//Setting the sleep mode, in our case full sleep
  sleep_cpu();
}
