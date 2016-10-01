#define NODEBUG
#include <IRremote.h>
int RECV_PIN = A5;
#include "UniversalIR.h"


int BUTTON_PIN = 4;
int SPEAKER_PIN = 13;

int buttonspin[] = {8, 7, 5, 2, 9};

MyMenu myMenu(BUTTON_PIN, SPEAKER_PIN, buttonspin);



void setup()
{
  myMenu.Setup();
}


void loop() {
  myMenu.ProcessButtons();
}
