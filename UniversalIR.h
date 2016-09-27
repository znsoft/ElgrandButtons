
#include <EEPROM.h>
#include <IRremote.h>
#include <HashMap.h>
IRsend irsend;

// bad
#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))

// good
template <unsigned S> inline unsigned arraysize(int (&v)[S]) {
  return S;
}


enum MyMode {
  Config,
  Play

};


class MyMenu {
  private:
    int usePins[];
    int speakerpin;
    MyMode myMode;
    int myFat[];
    int configButtonPin;
    int learnCount, nowLearnPin;
    int buttonsCount;

    //Buzzer
    void buzz( long frequency, long length) {
      long delayValue = 1000000 / frequency / 2; // calculate the delay value between transitions
      //// 1 second's worth of microseconds, divided by the frequency, then split in half since
      //// there are two phases to each cycle
      long numCycles = frequency * length / 1000; // calculate the number of cycles for proper timing
      //// multiply frequency, which is really cycles per second, by the number of seconds to
      //// get the total number of cycles to produce
      for (long i = 0; i < numCycles; i++) { // for the calculated length of time...
        digitalWrite(speakerpin, HIGH); // write the buzzer pin high to push out the diaphram
        delayMicroseconds(delayValue); // wait for the calculated delay value
        digitalWrite(speakerpin, LOW); // write the buzzer pin low to pull back the diaphram
        delayMicroseconds(delayValue); // wait again or the calculated delay value
      }
    }

    template <unsigned S>
    void InitAllPinsIn(int (&P)[S]) {
      for (int i = 0; i < S; i++) {
        pinMode(P[i], INPUT);
        digitalWrite(P[i], HIGH);
      }
    }


  public:

    //Constructor
    template <unsigned S>
    MyMenu(int configPin, int speaker, int (&buttonsPins)[S]) {
      speakerpin = speaker;
      buttonsCount = S ;
      usePins = buttonsPins;
      configButtonPin = configPin;
      myMode = Play;
      myFat = new int[S];
      InitAllPinsIn(buttonsPins);
      ReadMyEEPROM();
      //if (IsMyEEPROMEmpty(S))        myMode = Config;
      pinMode(configButtonPin, INPUT);
      digitalWrite(configButtonPin, HIGH);
      melody1();
    }


    void ProcessSendButtons() {
      //usePins
      int S = ARRAY_SIZE(usePins);

      //int S = ARRAY_SIZE(myFat);
      for (int i = 0; i < S; i++) {
        int buttonPin = usePins[i];
        if (!digitalRead(buttonPin)) {
          unsigned int adr = myFat[i];
          unsigned int len = myFat[i + 1] - adr;
          int *code = new unsigned int[len];
          EEPROM.get(adr, code);
          irsend.sendRaw(code, len, 38);
        }
      }
      if (!digitalRead(configButtonPin)) {
        myMode = Config;
        learnCount = 0, nowLearnPin = 0;
        melody2();
      }
    }


    void ProcessButtons() {
      if (myMode == Config) {
        LearnButtons();
      } else {
        ProcessSendButtons();
      }
    }


    void  LearnButtons() {
      int S = ARRAY_SIZE(usePins);
      for (int i = 0; i < S; i++) {
        int buttonPin = usePins[i];
        if (!digitalRead(buttonPin)) nowLearnPin = buttonPin;
      }
    }


    void melody1() {
      buzz( 2000, 500);
      buzz( 1000, 500);
      buzz( 1500, 500);
      buzz( 3000, 500);
    }

    void melody2() {
      buzz( 2000, 500);
      buzz( 1, 500);
      buzz( 1500, 500);
      buzz( 3, 500);
      buzz( 2000, 500);
    }


    void ReadMyEEPROM() {

      EEPROM.get(0, myFat);

    }

};


