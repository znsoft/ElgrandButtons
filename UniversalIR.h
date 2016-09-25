
#include <EEPROM.h>

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

    template <unsigned S>
    int GetAllButtons(int (&P)[S]) {
      int ret = 0, bit1 = 1;
      for (int i = 0; i < S; i++) {
        ret = ret | (bit1 & !digitalRead(P[i]));
        bit1 = bit1 << 1;
      }
      return ret;
    }

  public:

    //Constructor
    template <unsigned S>
    MyMenu(int speaker, int (&P)[S]) {
      speakerpin = speaker;
      myFat = new int[S];
      InitAllPinsIn(P);
      myMode = Play;
      ReadMyEEPROM();
      if (IsMyEEPROMEmpty(S))
        myMode = Config;
      usePins = P;
    }


    ;

    template <unsigned S>
    void ProcessPlayButtons(int (&P)[S], void (*MyCallbackSendIR)(unsigned int *sendcode,unsigned int len)) {
      for (int i = 0; i < S; i++)
        if (!digitalRead(P[i])) {
          unsigned int adr = i == 0 ? sizeof(myFat) : myFat[i - 1];
          unsigned int len = myFat[i] - adr;
          int *code = new unsigned int[len];
          EEPROM.get(adr, code);
          MyCallbackSendIR(code,len);
        }


    }

    void melody1() {
      buzz( 2000, 500);
      buzz( 1000, 500);
      buzz( 1500, 500);
      buzz( 3000, 500);
    }

    void ReadMyEEPROM() {

      EEPROM.get(0, myFat);

    }


    bool IsMyEEPROMEmpty(int S) {
      for (int i = 0; i < S; i++)if (myFat[i] != 0)return false;
      return true;
      //return (myFat.code1 == 0 || myFat.code2 == 0 || myFat.code3 == 0 || myFat.code4 == 0 || myFat.code5 == 0 || myFat.endof == 0);
    }


};


