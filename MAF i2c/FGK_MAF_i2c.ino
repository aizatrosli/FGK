///////////////////////////////////////////FGK MAF I2C//////////////////////////////////////////////
//Pin assign RPM detection = pin 2 & 3, injection = pin 4, pump = pin 5, MAP = pin A0, TPS = pin A5
//Timer prescaler based 256
///////////////////////////////////////////INITIALIZE///////////////////////////////////////////////
//Header////////////////////////////////////////////////////////////////////////////////////////////
#include "Wire.h";
//i2C///////////////////////////////////////////////////////////////////////////////////////////////
volatile int ATtinyAddress=0x26;
volatile unsigned long marktime;
//inject////////////////////////////////////////////////////////////////////////////////////////////
const byte pullup = 2;
const byte injectPIN = 4;
volatile unsigned int injectDelayTime;
volatile unsigned long injectOnTime = 30;
volatile boolean injectOn;
//RPM///////////////////////////////////////////////////////////////////////////////////////////////
volatile double rps;
volatile unsigned long periodz;
volatile int rpm;
//Airflow///////////////////////////////////////////////////////////////////////////////////////////
volatile double AF;
volatile int RAWMAP;
volatile double MAP;
volatile double K = 0.1;
volatile double InjSpec = 14.3625; // (4.5/1.2)*3.83
//Oil pump//////////////////////////////////////////////////////////////////////////////////////////
const byte oilPUMP = 5;




//Setup/////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  Wire.begin();
  pinMode(oilPUMP, OUTPUT);
  digitalWrite(oilPUMP, HIGH);//Pump
  sInject();
}
//Loop//////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  rRPM();
  rMAP();
  rAF(); 
  fInject();
}

///////////////////////////////////////////SUBROUTINES/////////////////////////////////////////////
//i2C//////////////////////////////////////////////////////////////////////////////////////////////
void rRPM()
{
  if (millis()>marktime)
  {
    byte hb;
    byte lb;
    Wire.requestFrom(ATtinyAddress,2); 
    if (Wire.available()) 
    {
      lb=Wire.read();
      hb=Wire.read();
    }
    rpm=word(hb,lb);
    marktime+=100;
  }
}
//inject//////////////////////////////////////////////////////////////////////////////////////////
void sInject()
{
  TCCR1A = 0;  // normal mode
  TCCR1B = 0;  // stop timer
  TIMSK1 = 0;   // cancel timer interrupt

  pinMode (injectPIN, OUTPUT);
  digitalWrite (pullup, HIGH);  // pull-up
  attachInterrupt (0, reinject, RISING);
}
ISR (TIMER1_COMPA_vect)
{
  // if currently on, turn off
  if (injectOn)
    {
    digitalWrite (injectPIN, LOW);   // inject off
    TCCR1B = 0;                      // stop timer
    TIMSK1 = 0;                      // cancel timer interrupt
    EIFR = bit (INTF0);              // delete any pending interrupt on D2  
    attachInterrupt (0, reinject, RISING);    // re-instate interrupts for firing time
    }
  else
    // hold-off time must be up
    {
    digitalWrite (injectPIN, HIGH);    // inject on
    TCCR1B = 0;                        // stop timer
    TCCR1B = bit(WGM12) | bit(CS12);   // CTC, scale to clock / 256
    OCR1A = injectOnTime;               // time before timer fires
    }
    
  injectOn = !injectOn;  // toggle
}  
void reinject ()
{
  injectOn = false;                  // make sure flag off just in case
  TCCR1A = 0;  // normal mode
  TCCR1B = bit(WGM12) | bit(CS12);  // CTC, scale to clock / 8
  OCR1A = injectDelayTime;           // time before timer fires
  TIMSK1 = bit (OCIE1A);            // interrupt on Compare A Match
  detachInterrupt (0);   // cancel any existing falling interrupt (interrupt 0)
}
void fInject()
{   
  if (false)  // if we need to change the time, insert condition here ...
    {
    noInterrupts ();  // atomic change of the time amount
    interrupts ();
    }
} 
//Airflow///////////////////////////////////////////////////////////////////////////////////////
void rAF()
{
    rps = (rpm/60);//change RPM to frequency
    periodz = (1000/rps);//ms
    injectDelayTime = periodz*31.25;//31.25 = ((1x10-3/16000x10-9)/2)
    AF=(62.5*MAP*rpm*K)/(InjSpec*157476.7782); //in mS
    injectOnTime = AF*62.5;
    
}
//MAP//////////////////////////////////////////////////////////////////////////////////////////
void rMAP()
{
    RAWMAP = analogRead(A0);
    MAP = (1/0.045)*(((RAWMAP*5)/1023)+0.425);
    
}