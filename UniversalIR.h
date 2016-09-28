
#include <EEPROM.h>
#include <IRremote.h>

IRsend irsend; //irsend = 3 pin on arduino nano
IRrecv irrecv(RECV_PIN);

decode_results results;
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
    int *usePins;
    int speakerpin;
    MyMode myMode;
    unsigned int *myFat;
    int configButtonPin;
    int learnCount, nowLearnPin;
    int buttonsCount;
    //tempcode
    int *rawCodes;//[,];//RAWBUF]; // The durations
    int codeLen; // The length of the code

    // Stores the code for later playback
    // Most of this code is just logging
    void storeCode(decode_results *results, unsigned int index) {
      int j;
      for (j = 0; j < buttonsCount; j++)
        if (index == usePins[j])break;


      codeLen = results->rawlen - 1;
      // To store raw codes:
      // Drop first value (gap)
      // Convert from ticks to microseconds
      // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
      for (int i = 1; i <= codeLen; i++) {
        if (i % 2) {
          // Mark
          rawCodes[j, i ] = results->rawbuf[i] * USECPERTICK - MARK_EXCESS;
        }
        else {
          // Space
          rawCodes[j, i ] = results->rawbuf[i] * USECPERTICK + MARK_EXCESS;
        }
      }
      rawCodes[j, 0 ] = codeLen;
    }


    void SaveMyEEPROM() {
      myFat[0] = sizeof(myFat);
      for (int i = 1; i < buttonsCount; i++)  myFat[i] = myFat[i - 1] + rawCodes[i - 1, 0 ];
      EEPROM.put(0, myFat);
      for (int i = 0; i < buttonsCount; i++)
        for (int j = 0; j < rawCodes[i, 0]; j++)
          EEPROM.write(myFat[i] + j,  rawCodes[i, j]);
    }



    void ReadMyEEPROM() {
      EEPROM.get(0, myFat);
    }

    //Buzzer for arduino nano , there is no tone method on it
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


    void ProcessSendButtons() {
      int countPressedButtons = 0;
      for (int i = 0; i < buttonsCount; i++) {
        int buttonPin = usePins[i];
        if (!digitalRead(buttonPin)) {
          Serial.println("press");

          countPressedButtons++;
          unsigned int adr = myFat[i] >> 8;
          unsigned int len = (myFat[i] & 0xff) - adr;
          unsigned int *code = new unsigned int[len];
          EEPROM.get(adr, code);
          irsend.sendRaw(code, len, 38);
           Serial.print(buttonPin);
           Serial.println(" send");
        }
      }
      //if (countPressedButtons == buttonsCount)

      if (!digitalRead(configButtonPin)) {
        myMode = Config;
        Serial.println("Config");

        learnCount = 0, nowLearnPin = 0;
        memset(rawCodes, 0 , sizeof(rawCodes));
        melody2();
        irrecv.enableIRIn(); // Start the receiver
      }
    }


    void  LearnButtons() {
      for (int i = 0; i < buttonsCount; i++) {
        int buttonPin = usePins[i];
        if (!digitalRead(buttonPin)) nowLearnPin = buttonPin;
      }
      if (nowLearnPin == 0 )return;
      if (irrecv.decode(&results)) {
        irrecv.enableIRIn(); // Re-enable receiver
        storeCode(&results, nowLearnPin);
        SaveMyEEPROM();
        irrecv.resume(); // resume receiver
        melody3();
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
      buzz( 5, 400);
      buzz( 1500, 400);
      buzz( 5, 400);
      buzz( 2000, 500);
    }

    void melody3() {
      buzz( 3000, 300);
      buzz( 1500, 300);
    }

  public:

    //Constructor
    template <unsigned S>
    MyMenu(int configPin, int speaker, int (&buttonsPins)[S]) {
      speakerpin = speaker;
      buttonsCount = S ;
      Serial.begin(9600);
      Serial.println(buttonsCount);
      usePins = buttonsPins;
      configButtonPin = configPin;
      myMode = Play;
      rawCodes = new int[S, RAWBUF];
      memset(rawCodes, 0 , sizeof(rawCodes));
      myFat = new unsigned int[S];
      InitAllPinsIn(buttonsPins);
      ReadMyEEPROM();
      //if (IsMyEEPROMEmpty(S))        myMode = Config;
      pinMode(configButtonPin, INPUT);
      digitalWrite(configButtonPin, HIGH);
      melody1();
      // irrecv = new IRrecv(recvPin);
    }





    void ProcessButtons() {
      if (myMode == Config) {
        LearnButtons();
      } else {
        ProcessSendButtons();
      }
    }




};







