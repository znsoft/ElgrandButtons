/*
 * IRrecord: record and play back IR signals as a minimal 
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * An IR LED must be connected to the output PWM pin 3.
 * A button must be connected to the input BUTTON_PIN; this is the
 * send button.
 * A visible LED can be connected to STATUS_PIN to provide status.
 *
 * The logic is:
 * If the button is pressed, send the IR code.
 * If an IR code is received, record it.
 *
 * Version 0.11 September, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 * 

 */

#include <EEPROM.h>

/** the current address in the EEPROM (i.e. which byte we're going to write to next) **/
int addr = 0;
#include <IRremote.h>

#include "UniversalIR.h"

int RECV_PIN = A5;
int BUTTON_PIN = 9;
int STATUS_PIN = 13;
int SPEAKER_PIN = 8;

IRrecv irrecv(RECV_PIN);
IRsend irsend;

decode_results results;

void buzz(int targetPin, long frequency, long length) {
  digitalWrite(3, HIGH);
  long delayValue = 1000000 / frequency / 2; // calculate the delay value between transitions
  //// 1 second's worth of microseconds, divided by the frequency, then split in half since
  //// there are two phases to each cycle
  long numCycles = frequency * length / 1000; // calculate the number of cycles for proper timing
  //// multiply frequency, which is really cycles per second, by the number of seconds to
  //// get the total number of cycles to produce
  for (long i = 0; i < numCycles; i++) { // for the calculated length of time...
    digitalWrite(targetPin, HIGH); // write the buzzer pin high to push out the diaphram
    delayMicroseconds(delayValue); // wait for the calculated delay value
    digitalWrite(targetPin, LOW); // write the buzzer pin low to pull back the diaphram
    delayMicroseconds(delayValue); // wait again or the calculated delay value
  }
  digitalWrite(3, LOW);
 
}


void setup()
{
  
    //EEPROM.write(addr, val);
  Serial.begin(9600);
  buzz(SPEAKER_PIN, 2000, 500);
  buzz(SPEAKER_PIN, 1000, 500);
  buzz(SPEAKER_PIN, 1500, 500);
  buzz(SPEAKER_PIN, 3000, 500);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(3, OUTPUT);
 pinMode(BUTTON_PIN, INPUT);
digitalWrite(BUTTON_PIN, HIGH);       // turn on pullup resistors
  pinMode(STATUS_PIN, OUTPUT);
//  noTone(SPEAKER_PIN);
}

// Storage for the recorded code
int codeType = -1; // The type of code
unsigned long codeValue; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen; // The length of the code
int toggle = 0; // The RC5/6 toggle state


// Stores the code for later playback
// Most of this code is just logging
void storeCode(decode_results *results) {
  codeType = results->decode_type;
  int count = results->rawlen;
 // if (codeType == UNKNOWN) {
    Serial.println("Received code, saving ");
    codeLen = results->rawlen - 1;
    // To store raw codes:
    // Drop first value (gap)
    // Convert from ticks to microseconds
    // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
    for (int i = 1; i <= codeLen; i++) {
      if (i % 2) {
        // Mark
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
        Serial.print(" m");
      } 
      else {
        // Space
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
        Serial.print(" s");
      }
      Serial.print(rawCodes[i - 1], DEC);
    }
    Serial.println("");

}

void sendCode() {

    // Assume 38 KHz
    irsend.sendRaw(rawCodes, codeLen, 38);
    Serial.println("Sent raw");

}

int lastButtonState;

void loop() {
  // If button pressed, send the code.
  int buttonState = !digitalRead(BUTTON_PIN);

  if (buttonState) {
    Serial.println("Pressed, sending");
    digitalWrite(STATUS_PIN, HIGH);
    
    sendCode(lastButtonState == buttonState);
    digitalWrite(STATUS_PIN, LOW);
    delay(150); // Wait a bit between retransmissions
  } 
  else if (irrecv.decode(&results)) {
    irrecv.enableIRIn(); // Re-enable receiver
    digitalWrite(STATUS_PIN, HIGH);
    storeCode(&results);
    irrecv.resume(); // resume receiver
    digitalWrite(STATUS_PIN, LOW);
  }
  lastButtonState = buttonState;
}
