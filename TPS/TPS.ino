//TPS module//
#include <EEPROM.h>
#include "eeany.h"
//Calibrate /////////////
volatile int minTPS;
volatile int maxTPS;
volatile int sensorValue;
int CALIBpin = 4;
int CALIBstate = 0;
//TT/////////////////////
volatile double RAWinTPS;
volatile double inTPS;
volatile double inOldTPS = 0;
volatile double outTPS = 0;
int TPSpin = 2;
//MEMory/////////////////
EEMEM uint16_t savemaxTPS; //integer in eeprom; avr will control address
EEMEM uint16_t saveminTPS;

void setup()
{
  Serial.begin(9600);
  pinMode(CALIBpin, INPUT);
  CALIBstate = digitalRead(CALIBpin);
  if (CALIBstate == HIGH) 
  {
    Serial.println("Calibrating..");
    calibrate();
    Serial.println("Done");
  }
  loadTPS();
}

void calibrate ()
{
	while (millis() < 10000) 
  {
   sensorValue = analogRead(TPSpin);
 
   // record the maximum sensor value
   if (millis() < 4000) {
   		minTPS = sensorValue;
       Serial.print("minTPSCAL ; ");
       Serial.println(minTPS);
    	eeprom_write_word( & saveminTPS , minTPS);
   }

   // record the minimum sensor value
   if (millis() > 5000 && millis() < 9000) {
     	maxTPS = sensorValue;
       Serial.print("maxTPSCAL ; ");
       Serial.println(maxTPS);
    	eeprom_write_word( & savemaxTPS , maxTPS);
   }
  }
}

 void loadTPS()
 {
  unsigned int valMax =eeprom_read_word(& savemaxTPS);
  unsigned int valMin =eeprom_read_word(& saveminTPS);
  maxTPS = valMax;
  minTPS = valMin;
  Serial.print("Valax ; ");
  Serial.println(valMax);
  Serial.print("Valin ; ");
  Serial.println(valMin);
         Serial.print("minTPS ; ");
       Serial.println(minTPS);
              Serial.print("maxTPS ; ");
       Serial.println(maxTPS);
 }
	
	

void loop()
{
	RAWinTPS = analogRead(TPSpin);
        Serial.print("RAW ; ");
        Serial.println(RAWinTPS);
	inTPS = map(RAWinTPS, minTPS, maxTPS, 0, 90);
        Serial.print("inTPS ; ");
        Serial.println(inTPS);
	outTPS = (0.01*inTPS) - (0.01*inOldTPS) + (0.8*outTPS);//TPS eq
	inTPS = inOldTPS;
        Serial.print("outTPS ; ");
        Serial.println(outTPS);
	
}


