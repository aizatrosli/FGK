

#include <NilRTOS.h>
#include <NilSerial.h>
#define MainPeriod 100
#define Serial NilSerial

//Pulse injector/////////////////////////////////////////////////////////////////////////////////////////
const byte FIRE_SENSOR = 2;
const byte injectPLUG = 11;

volatile unsigned int injectDelayTime;
volatile unsigned long injectOnTime;
volatile boolean injectOn;
volatile int alternate = 0;
volatile int lastlow = 1;
volatile double rps;
volatile double AF;
volatile unsigned long periodz;
//RPM counter////////////////////////////////////////////////////////////////////////////////////////////
long previousMillis = 0; // will store last time of the cycle end
volatile unsigned long duration=0; // accumulates pulse width
volatile unsigned int pulsecount=0;
volatile unsigned long previousMicros=0;
volatile float Freq;
volatile unsigned long _duration = duration;
volatile unsigned long _pulsecount = pulsecount;

//----------------------------------------------------------------------------------------------------//
//Injection thread(1)///////////////////////////////////////////////////////////////////////////////////
NIL_WORKING_AREA(waThread1, 64);
NIL_THREAD(Thread1, arg) 
{
sInject();

  while (TRUE) 
  {
    nilThdSleep(100);
    fInject();
  }
}
//----------------------------------------------------------------------------------------------------//
//AirFlow Estimation thread(2)//////////////////////////////////////////////////////////////////////////
NIL_WORKING_AREA(waThread2, 64);
NIL_THREAD(Thread2, arg) 
{

  while (TRUE) 
  {
    nilThdSleep(100);
    //Estimation AirFlow
    AF=(1.875*Freq)/603136.0605;
    //50% of ignition period
    injectDelayTime = ((62500/(Freq/2))-1);
    //
    injectOnTime = ((62500*AF*30)-1); 
  }
}
//----------------------------------------------------------------------------------------------------//
//AirFlow Estimation thread(3)//////////////////////////////////////////////////////////////////////////
NIL_WORKING_AREA(waThread3, 64);
NIL_THREAD(Thread3, arg) {
 
attachInterrupt(1, myinthandler, RISING);
  while (TRUE) 
  {
    nilThdSleep(100);
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= MainPeriod) 
        {
          previousMillis = currentMillis;   
          unsigned long _duration = duration;
          unsigned long _pulsecount = pulsecount;
          duration = 0; // clear counters
          pulsecount = 0;
          Freq = 1e6 / float(_duration);
          Freq *= _pulsecount; // calculate F
        }
  }
}
//----------------------------------------------------------------------------------------------------//
//NilRTOS Library Initialization////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------------//
NIL_THREADS_TABLE_BEGIN()
NIL_THREADS_TABLE_ENTRY(NULL, Thread1, NULL, waThread1, sizeof(waThread1))
NIL_THREADS_TABLE_ENTRY(NULL, Thread2, NULL, waThread2, sizeof(waThread2))
NIL_THREADS_TABLE_ENTRY(NULL, Thread3, NULL, waThread3, sizeof(waThread3))
NIL_THREADS_TABLE_END()
//----------------------------------------------------------------------------------------------------//
//AirFlow Estimation thread(3)//////////////////////////////////////////////////////////////////////////
void setup() 
{
  Serial.begin(9600);
  nilSysBegin();
}
//----------------------------------------------------------------------------------------------------//
//AirFlow Estimation thread(3)//////////////////////////////////////////////////////////////////////////
void loop() 
{
  //Show freq in serial monitor
  Serial.print(Freq);
  Serial.println("Hz");
}
//----------------------------------------------------------------------------------------------------//
//Injection Interrupt///////////////////////////////////////////////////////////////////////////////////
void sInject()
{
  TCCR4A = 0;
  TCCR4B = 0;  
  TIMSK4 = 0;   
  pinMode (injectPLUG, OUTPUT);
  attachInterrupt (0, fireinject, RISING);
}

ISR (TIMER4_COMPA_vect)
{

  // if currently on, turn off
  if (injectOn)
    {
      digitalWrite (injectPLUG, LOW); 
      TCCR4B = 0;                    
      TIMSK4 = 0;                    
      EIFR = bit (INTF0);                
      attachInterrupt (0, fireinject, RISING);   
    }
  else
    // hold-off time must be up
    {
      digitalWrite (injectPLUG, HIGH);    
      TCCR4B = 0;                        
      TCCR4B = bit(WGM42) | bit(CS42);   
      OCR4A = injectOnTime;               
    }
    injectOn = !injectOn;  // toggle

}  // end of TIMER1_COMPA  

// ISR for when to fire
void fireinject ()
{
  injectOn = false;                  
  // set up Timer 1
  TCCR4A = 0;  
  TCCR4B = bit(WGM42) | bit(CS42);  
  OCR4A = injectDelayTime;          
  TIMSK4 = bit (OCIE4A);           
  detachInterrupt (0);   

void fInject()
{ 
  
  if (false) 
    {
    noInterrupts (); 
    interrupts ();
    }
} 
//----------------------------------------------------------------------------------------------------//
//RPM counter Interrupt///////////////////////////////////////////////////////////////////////////////////
void myinthandler() // interrupt handler
{
  NIL_IRQ_PROLOGUE();
  unsigned long currentMicros = micros();
  duration += currentMicros - previousMicros;
  previousMicros = currentMicros;
  pulsecount++;
  NIL_IRQ_EPILOGUE();
}