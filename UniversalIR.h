
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


struct FAT {
  int code1, pin1;
  int code2, pin2;
  int code3, pin3;
  int code4, pin4;
  int code5, pin5;
  int endof, nopin;
};

class MyMenu {
  private:
    int allButtons[25];

  public:
    FAT myFat;
    int speakerpin;
    MyMode myMode;
    int *excludePins;



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



    //Constructor
    template <unsigned S>
    MyMenu(int speaker, int (&excludeP)[S]) {
      speakerpin = speaker;
      InitAllPinsIn(excludeP);
      myMode = Play;
      ReadMyEEPROM();
      if (IsMyEEPROMEmpty())
        {
          myMode = Config;
        int *A = GetAllButtons(excludeP);
         for (int e = 0; e < arraysize(A)); e++)
        
        
        }
      
      excludePins = new int[S];
      //memcpy(

    }

    template <unsigned S>
    void InitAllPinsIn(int (&excludeP)[S]) {
      for (int i = 0; i < 24; i++) {
        bool isExclude = false;
        for (int e = 0; e < S; e++)
          isExclude = isExclude || (excludeP[e] == i);
        if (isExclude)continue;
        pinMode(i, INPUT);
        digitalWrite(i, HIGH);
      }
    }

    template <unsigned S>
    int *GetAllButtons(int (&excludeP)[S]) {
      for (int i = 0; i < 24; i++) {
        bool isExclude = false;
        for (int e = 0; e < S); e++)
        isExclude = isExclude || (excludeP[e] == i);
        if (isExclude)continue;
        allButtons[i] = digitalRead(i);
      }
    return allButtons;
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

    bool IsMyEEPROMEmpty() {
      return (myFat.code1 == 0 || myFat.code2 == 0 || myFat.code3 == 0 || myFat.code4 == 0 || myFat.code5 == 0 || myFat.endof == 0);
    }


};


