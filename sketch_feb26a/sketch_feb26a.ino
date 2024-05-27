#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <TM1637Display.h>

#define firstMode 63 //Soutěž reakčního času
#define secondMode 149 //Ukazatel vzdálenosti
#define thirdMode 324 //Pokojový teploměr
#define fourthMode 676 //

int (*loopFunction[1]) ();

Servo myservo; 

/*
-------------------------------------------------------------------- First Mode ------------------------------------------------------------------------
*/
class FirstMode {

  static inline uint8_t Player1 = 2;
  static inline uint8_t Player2 = 3;

  // Definujeme si PINy pro ovládání LEDek
  static inline uint8_t Player1LED = 13;
  static inline uint8_t Player2LED = 8;
  static inline uint8_t LedPIN = 11;
  static inline uint8_t napajeni = A5;

  static inline bool started = false;
  static inline bool ended = false;

  public:
  
     // Definujeme si na kterých PINech budeme číst tlačítka
    static void firstModeSetup() {

      // Nastavíme si PINy na čtení tlačítek
      pinMode(Player1, INPUT);
      pinMode(Player2, INPUT);


      // Nastavíme si PINy na ovládání LEDek
      pinMode(Player1LED, OUTPUT);
      pinMode(Player2LED, OUTPUT);
      pinMode(LedPIN, OUTPUT);
      pinMode(napajeni, OUTPUT);

      digitalWrite(napajeni, HIGH);


      /*
      * Tímto řekneme že pokud nám poklesne napětí na čtecích PINech tak
      * se má spustit metoda buttonPressedPlayer1/2 (FALLING můžeme nahradit RISING
      * a poté se metoda spustí po zvýšení)
      */
      attachInterrupt(digitalPinToInterrupt(2), buttonPressedPlayer1, RISING);
      attachInterrupt(digitalPinToInterrupt(3), buttonPressedPlayer2, RISING);
      

      // Definujeme si proměnou jež nám bude vracet náhodná čílsa
      randomSeed(analogRead(0));
      srand(random(300));


      // LEDka nám třikrát blikne a pak se zhasne
      for(int i = 1; i <= 3; i++){
        digitalWrite(LedPIN, HIGH);
        delay(1000);
        digitalWrite(LedPIN, LOW);
        delay(1000);
      }
      
      // Program se na náhodnou dobu do 20s zastaví a poté se zapne LEDka a
      //stav hry se změní na probíhající
      delay(((rand() % 18) + 3) * 1000);
      started = true;

      if(!ended)
        digitalWrite(LedPIN, HIGH);

      while(true);
    }

    static void firstModeLoop() {
      while(true){}
    }

    /*
    * Pokud zmáčkne hráč 1 tlačítko
    */
    static int buttonPressedPlayer1(){

      digitalWrite(napajeni, LOW);
      ended = true;

      // Pokud  hra probíhala tak hráč 1 vyhraje pokud ne tak prohraje
      if(started == false){
        digitalWrite(Player2LED, HIGH);
      }
      else{
        digitalWrite(Player1LED, HIGH);
      }

      // Prostřední LEDka se vypne
      digitalWrite(LedPIN, LOW);

      return 0;
    }

    /*
    * Pokud zmáčkne hráč 2 tlačítko
    */
    static int buttonPressedPlayer2(){

      digitalWrite(napajeni, LOW);
      ended = true;

      // Pokud  hra probíhala tak hráč 2 vyhraje pokud ne tak prohraje
      if(started == false){
        digitalWrite(Player1LED, HIGH);
      }
      else{
        digitalWrite(Player2LED, HIGH);
      }

      // Prostřední LEDka se vypne
      digitalWrite(LedPIN, LOW);

      return 0;
    }
};

/*
----------------------------------------------------------------------- Second Mode --------------------------------------------------------------------------
*/

class SecondMode{
  // tyto piny ovládají a čtou z ultrazvukového senzoru
  static inline uint8_t trigPin = A1; 
  static inline uint8_t echoPin = A2;
  static inline uint8_t servoPin = A3; 


  // definuje potřebné číselné proměné
  static inline float duration_us, distance_cm, last_pos;
 
  public:
    static void secondModeSetup() {
      // přiřadí PIN 3 pro ovládání serva
      myservo.attach(servoPin);
      // nastavuje trigPin na mód výstupu
      pinMode(trigPin, OUTPUT);
      // nastavuje echoPin na mód vstupu
      pinMode(echoPin, INPUT_PULLUP);
      
      pinMode(2, OUTPUT);
      pinMode(3, OUTPUT);
      pinMode(4, OUTPUT);
      pinMode(5, OUTPUT);
      pinMode(6, OUTPUT);
      pinMode(7, OUTPUT);

    }


    static void secondModeLoop() {
      // vytvoří 10 milisekundový puls na trigPin
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      // změří dobu trvání pulsu na echoPin
      duration_us = pulseIn(echoPin, HIGH);

      // výočet vzdálenosti
      distance_cm = 0.017 * duration_us;


      // nastavíme strop na 5Ocm
      if(distance_cm > 50){
        distance_cm = 50;
      }

      // vypočítá procento vzdálenosti z maxima a z toho vypočítá úhel
      if(distance_cm <= last_pos - 2 || distance_cm >= last_pos + 2)
      {
        last_pos = distance_cm;
        int procentoMaxVzdalenosti = distance_cm * 4;
        myservo.write((180 / 100) * procentoMaxVzdalenosti);

        int ledNum = 7 - (distance_cm / 8.333333333333333333333);

        for(int i = 2; i <= 7; i++){
          if(ledNum == 0){
            digitalWrite(i, LOW);
          }
          else{
            digitalWrite(i, HIGH);
            ledNum--;
          }
        }
      }

    }
};

/*
----------------------------------------------------------------------- Third Mode --------------------------------------------------------------------------
*/

class ThirdMode{

static inline int CLK = 2;
static inline int DIO = 3;

static inline uint8_t segments[] = {
  SEG_G | SEG_A | SEG_B | SEG_F,
  SEG_A | SEG_F | SEG_E | SEG_D
};

static inline Adafruit_BMP085 bmp180;
static inline TM1637Display displej = TM1637Display(CLK, DIO);
static inline int korekce = 32;
static inline int teplota;

public:

static void thirdModeSetup() {

  Wire.begin(); 
  Serial.begin(9600);
  bmp180.begin();
  displej.clear();
  displej.setBrightness(10);

}

static void thirdModeLoop() {
  int temprature = bmp180.readTemperature();
  Serial.println(temprature);
  displej.showNumberDecEx(temprature, false, 2, 2);
  displej.setSegments(segments, 2, 2);
  delay(1000);
}

};

/*
--------------------------------------------------------------------------- Main --------------------------------------------------------------------------
*/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A0, INPUT);
  bool again = true;
  Serial.begin(9600);
  while(again)
  {
    int choice = int(analogRead(A0));
    switch(choice)
    {
      case firstMode:
        loopFunction[0] = FirstMode::firstModeLoop;
        FirstMode::firstModeSetup();
        again = false;
        break;

      case secondMode:
        loopFunction[0] = SecondMode::secondModeLoop;
        SecondMode::secondModeSetup();
        again = false;
        break;

      case thirdMode:
        loopFunction[0] = ThirdMode::thirdModeLoop;
        ThirdMode::thirdModeSetup();
        again = false;
        break;

      case fourthMode:
        break;
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  loopFunction[0]();
}



