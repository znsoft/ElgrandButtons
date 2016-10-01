
#include <EEPROM.h>
#include <IRremote.h>

IRsend irsend; //irsend = 3 pin on arduino nano, передающий ШИМ пин по умолчанию для ардуино Нано это 3 пин
IRrecv irrecv(RECV_PIN);

decode_results results;


enum MyMode {
  Config,
  Play
};

struct MyFAT {
  int adr; byte len;
};

struct MyRAW {
  byte len;
  int d[RAWBUF];
};


class MyMenu {
  private:
    int *usePins;
    int speakerpin;
    MyMode myMode;
    MyFAT *myFat;
    int configButtonPin;
    int learnCount, nowLearnPin;
    int buttonsCount;
    //tempcode
    MyRAW *rawCodes;
    int codeLen; // The length of the code

    // Stores the code for later playback
    // Most of this code is just logging

    template<typename T>
    void MyDebug(T s) {
#ifndef NODEBUG
      Serial.print(s);
#endif
    }



    template<typename T>
    void MyDebugln(T s) {
#ifndef NODEBUG
      Serial.println(s);
#endif
    }

    //декодирование сигналов ИК пульта
    void storeCode(decode_results *results, unsigned int index) {
      int j;
      for (j = 0; j < buttonsCount; j++)
        if (index == usePins[j])break;
      MyDebugln("Store");
      MyDebugln(j);
      codeLen = results->rawlen - 1;
      // To store raw codes:
      // Drop first value (gap)
      // Convert from ticks to microseconds
      // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
      for (int i = 1; i <= codeLen; i++) {
        if (i % 2) {
          // Mark
          rawCodes[ j ].d[i - 1] = results->rawbuf[i] * USECPERTICK - MARK_EXCESS;
          MyDebug(" m"); MyDebug(rawCodes[ j ].d[i - 1]);
        }
        else {
          // Space
          rawCodes[ j ].d[i - 1] = results->rawbuf[i] * USECPERTICK + MARK_EXCESS;
          MyDebug(" s"); MyDebug(rawCodes[ j ].d[i - 1]);
        }
      }
      MyDebugln(" ");
      rawCodes[j].len = codeLen;
      MyDebugln(codeLen);

    }


    //#region Файловая система на встроенной флеш памяти
    //simple filesystem fat write, 
    void SaveMyEEPROM() {
      nowLearnPin = 0;
      MyDebugln("Save");
      int fatSize = sizeof(myFat[0]) * buttonsCount;
      myFat[0].adr = fatSize;
      myFat[0].len = rawCodes[0].len;
      EEPROM.put(0, myFat[0]);
      for (int i = 1; i < buttonsCount; i++) {
        myFat[i].adr = myFat[i - 1].adr + rawCodes[i - 1].len * sizeof(rawCodes[0].d[0]);
        myFat[i].len = rawCodes[i].len;
        EEPROM.put(i * sizeof(myFat[0]), myFat[i]);
        MyDebugln(myFat[i].adr);
      }


      for (int i = 0; i < buttonsCount; i++) {
        MyDebugln(rawCodes[i].len);
        MyDebugln(myFat[i].adr);

        for (int j = 0; j < rawCodes[i].len; j++) {
          int t = rawCodes[i].d[ j];
          EEPROM.put(myFat[i].adr + (j) * sizeof(t), t);
          MyDebug(t);
          MyDebug(" , ");
        }
        MyDebugln("   =  ");
        MyDebugln(i);

      }
      melody4();
    }


//чтение заголовка указателей файловой системы 
    void ReadMyEEPROM() {
      MyDebugln("ReadMyEEPROM");
      for (int i = 0; i < buttonsCount; i++) {
        EEPROM.get(i * sizeof(myFat[0]), myFat[i]);
        MyDebug(myFat[i].len);
        MyDebug(" ");
        MyDebugln(myFat[i].adr);
      }
    }

//#endregion Файловая системана на встроенной флеш памяти

    void InitAllPinsIn(int *P) {
      for (int i = 0; i < buttonsCount; i++) {
        pinMode(P[i], INPUT);
        digitalWrite(P[i], HIGH);
      }
    }

    //#region Кнопки
    void ProcessSendButtons() {
      int countPressedButtons = 0;
      for (int i = 0; i < buttonsCount; i++) {
        int buttonPin = usePins[i];
        if (!digitalRead(buttonPin)) {
          MyDebugln("press");
          MyDebugln(buttonPin);

          countPressedButtons++;
          unsigned int adr = myFat[i].adr;
          unsigned int len = myFat[i].len;
          MyDebugln(len);
          unsigned int code[len];// = new int[len];
          for (int i = 0; i < len; i++)EEPROM.get(adr + i * sizeof(code[i]), code[i]);


          for (int i = 0; i < len; i++) {
            MyDebug(code[i]);
            MyDebug(" , ");
          }

          irsend.sendRaw(code, len, 38);
          melody6(buttonPin);
          //for (int j = 0; j < len; j++)
          //MyDebug(code[j]);
          MyDebugln(" sended ");
        }
      }

      if (countPressedButtons == (buttonsCount - 1) || !digitalRead(configButtonPin)) {
        myMode = Config;
        MyDebugln("Config");

        learnCount = 0, nowLearnPin = 0;
        //memset(rawCodes, 0 , sizeof(rawCodes[0, 0])*buttonsCount * RAWBUF);
        melody2();
        irrecv.enableIRIn(); // Start the receiver
      }
    }


    void  LearnButtons() {
      for (int i = 0; i < buttonsCount; i++) {
        int buttonPin = usePins[i];
        if (!digitalRead(buttonPin)) {
          nowLearnPin = buttonPin;
          melody5();
          MyDebug("Learn ");
          MyDebugln(buttonPin);

        }
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

    //#endregion Кнопки

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


    void melody1() {
      buzz( 2000, 200);
      buzz( 1200, 200);
      buzz( 1500, 100);
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

    void melody4() {
      buzz( 2000, 300);
      buzz( 3500, 300);
    }

    void melody5() {
      buzz( 1200, 100);
      buzz( 2500, 100);
    }

    void melody6(int pin) {
      pin *= 13;
      buzz( 2200 + pin, 30);
      buzz( 700 + pin, 30);
    }

  public:
    void Setup() {
#ifndef NODEBUG
      Serial.begin(57600);
#endif
      MyDebug("Starting ");
      MyDebugln(buttonsCount);
      myMode = Play;
      rawCodes = new MyRAW[buttonsCount];

      //rawCodes = new int[buttonsCount];
      myFat = new MyFAT[buttonsCount];
      InitAllPinsIn(usePins);
      ReadMyEEPROM();
      pinMode(configButtonPin, INPUT);
      digitalWrite(configButtonPin, HIGH);
      melody1();
      MyDebugln("Setup set over");

    }


    //Constructor
    template <unsigned S>
    MyMenu(int configPin, int speaker, int (&buttonsPins)[S]) {
      speakerpin = speaker;
      buttonsCount = S ;
      usePins = buttonsPins;
      configButtonPin = configPin;
    }

    void ProcessButtons() {
      if (myMode == Config) {
        LearnButtons();
      } else {
        ProcessSendButtons();
      }
    }
};







