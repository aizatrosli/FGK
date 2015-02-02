//TPS module//
#include <EEPROM.h>
#include "eeany.h"
//Calibrate /////////////
volatile double minTPS;
volatile double maxTPS;
volatile double sensorValue;
int CALIBpin = 3;
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
  pinMode(CALIBpin, INPUT);
  CALIBstate = digitalRead(CALIBpin);
  if (CALIBstate == HIGH) 
  {
    calibrate();
  }
  loadTPS();
}

void calibrate ()
{
	while (millis() < 5000) 
  {
   sensorValue = analogRead(TPSpin);
 
   // record the maximum sensor value
   if (millis() < 2000) {
   		minTPS = sensorValue*100;
    	eeprom_write_word( & saveminTPS , minTPS);
   }

   // record the minimum sensor value
   if (millis() > 2000 && millis() < 4000) {
     	maxTPS = sensorValue*100;
    	eeprom_write_word( & savemaxTPS , maxTPS);
   }
  }
}

 void loadTPS()
 {
  int valMax =eeprom_read_word(& savemaxTPS);
  int valMin =eeprom_read_word(& saveminTPS);
  maxTPS = valMax/100;
  minTPS = valMin/100;
 }
	
	

void loop()
{
	RAWinTPS = analogRead(TPSpin);
	inTPS = map(RAWinTPS, minTPS, maxTPS, 0, 90);
	outTPS = (0.01*inTPS) - (0.01*inOldTPS) + (0.8*outTPS);
	inTPS = inOldTPS;
	
}


