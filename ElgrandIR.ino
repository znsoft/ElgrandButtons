/*
   IRrecord: record and play back IR signals as a minimal
   An IR detector/demodulator must be connected to the input RECV_PIN.
   An IR LED must be connected to the output PWM pin 3.
   A button must be connected to the input BUTTON_PIN; this is the
   send button.
   A visible LED can be connected to STATUS_PIN to provide status.

   The logic is:
   If the button is pressed, send the IR code.
   If an IR code is received, record it.

   Version 0.11 September, 2009
   Copyright 2009 Ken Shirriff
   http://arcfn.com


*/

#include <IRremote.h>

#include "UniversalIR.h"

int RECV_PIN = A5;
int BUTTON_PIN = 9;
int STATUS_PIN = 13;
int SPEAKER_PIN = 4;
//irsend = 3

IRrecv irrecv(RECV_PIN);


decode_results results;



void setup()
{


  Serial.begin(9600);

  irrecv.enableIRIn(); // Start the receiver
  pinMode(3, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN, HIGH);       // turn on pullup resistors
  pinMode(STATUS_PIN, OUTPUT);

}

// Storage for the recorded code
//int codeType = -1; // The type of code
//unsigned long codeValue; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen; // The length of the code
//int toggle = 0; // The RC5/6 toggle state


// Stores the code for later playback
// Most of this code is just logging
void storeCode(decode_results *results) {
  //codeType = results->decode_type;
  //int count = results->rawlen;
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
      rawCodes[i - 1] = results->rawbuf[i] * USECPERTICK - MARK_EXCESS;
      Serial.print(" m");
    }
    else {
      // Space
      rawCodes[i - 1] = results->rawbuf[i] * USECPERTICK + MARK_EXCESS;
      Serial.print(" s");
    }
    Serial.print(rawCodes[i - 1], DEC);
  }
  Serial.println("");

}



int lastButtonState;

void loop() {
  // If button pressed, send the code.
  int buttonState = !digitalRead(BUTTON_PIN);

  if (buttonState) {
    Serial.println("Pressed, sending");
    digitalWrite(STATUS_PIN, HIGH);

    sendCode();
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
