
#include <IRremote.h>
int RECV_PIN = A5;
#include "UniversalIR.h"


int BUTTON_PIN = 9;
int STATUS_PIN = 13;
int SPEAKER_PIN = 4;
int buttonspin[4] = {8, 7, 5, 2};
MyMenu myMenu(BUTTON_PIN, SPEAKER_PIN, buttonspin);



void setup()
{


 

  //myMenu = new MyMenu

}


void loop() {
  myMenu.ProcessButtons();
}
