#define NODEBUG  //Режим отладки на последовательном порту
#include <IRremote.h>
int RECV_PIN = A5; // Пин на который подключен ИК приемник
#include "UniversalIR.h"


int BUTTON_PIN = 4;  // Пин на который подключена доп. кнопка включения режима обучения, если к примеру невозможно нажать все 
int SPEAKER_PIN = 13; // Динамик для воспроизведения звуков

int buttonspin[] = {8, 7, 5, 2, 9}; //пины куда подключены кнопки пульта ДУ

MyMenu myMenu(BUTTON_PIN, SPEAKER_PIN, buttonspin);



void setup()
{
  myMenu.Setup();
}


void loop() {
  myMenu.ProcessButtons();
}
