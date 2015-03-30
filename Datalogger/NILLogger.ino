//6 thread
// A2 (MAP) A3 (TPS) D4 (inj) D5&D2 (cdi) 
//|time|cdi pulse|freq|max inj pulse|inj pulse|est inj pulse|RAW MAP|MAP|TPS
#include <NilRTOS.h>
#include <NilSerial.h>
#define MainPeriod 100
#define Serial NilSerial


volatile int cdipulseState = 0;
volatile int lastcdiState = 0;
volatile int cdiState = 0;
//inj pulse////////////////////////////////////////////////////////////////////////////////////////////
volatile int injpulseState;
volatile unsigned long injOn = 0;
volatile unsigned long injOff = 0;
volatile unsigned long injTime = 0;
volatile int lastinjState = 0;
volatile int injState = 0;
//RPM counter///////////////////////////////////////////////////////////////////////////////////////////
long previousMillis = 0; // will store last time of the cycle end
volatile unsigned long duration=0; // accumulates pulse width
volatile unsigned int pulsecount=0;
volatile unsigned long previousMicros=0;
volatile float Freq;
volatile unsigned long _duration = duration;
volatile unsigned long _pulsecount = pulsecount;
volatile unsigned long RPM;
//Airflow///////////////////////////////////////////////////////////////////////////////////////////////
volatile int RAWTPS;//print 7
volatile int RAWMAP;//print 5
volatile double MAP;//print 6
//est airflow///////////////////////////////////////////////////////////////////////////////////////////
volatile double esttime;//print 5
volatile double K = 0.005;//<---------------- set
volatile double VE = 0.5;//<----------------- set (range 0-1)
volatile int TEMP = 311;//<------------------ set (default 311K)
volatile double spec = 3.83;//<-------------- set (old=3.83)
//max airflow///////////////////////////////////////////////////////////////////////////////////////////
volatile double maxtime;

//--analog---------------------------------------------------------------------
// Declare a stack with 64 bytes beyond context switch and interrupt needs.
NIL_WORKING_AREA(waThread1, 64);

// Declare thread function for thread 1.
NIL_THREAD(Thread1, arg) {

  while (TRUE) {
  
    // Sleep so lower priority threads can execute.
    nilThdSleep(100);
    RAWMAP = analogRead(2);
    MAP = (1/0.045)*(((MAP*5)/1023)+0.425);
    RAWTPS = analogRead(3);
  }
}
//--RPM------------------------------------------------------------------------
// Declare a stack with 64 bytes beyond context switch and interrupt needs.
NIL_WORKING_AREA(waThread2, 64);

// Declare thread function for thread 2.
NIL_THREAD(Thread2, arg) {
attachInterrupt(1, RPMint, RISING);
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
          RPM = Freq*60;
          Freq *= _pulsecount; // calculate F
        }
  }
}
//--inj pulse-------------------------------------------------------------------
// Declare a stack with 64 bytes beyond context switch and interrupt needs.
NIL_WORKING_AREA(waThread3, 64);

// Declare thread function for thread 3.
NIL_THREAD(Thread3, arg) {

  while (TRUE) {

    // Sleep so lower priority threads can execute.
    nilThdSleep(100);

    // Dummy use of CPU - use nilThdSleep in normal app.
    injState = digitalRead(4);
    if (injState != lastinjState)
    {

      if (injState == HIGH)
      {
        injpulseState = 1;
        injOn = millis();
      }
      else
      {
        injOff = millis();
        injpulseState = 0;
      }
    }
    lastinjState = injState;
    injTime = injOff - injOn;
  }
}

//--cdi pulse-------------------------------------------------------------------
// Declare a stack with 64 bytes beyond context switch analogRead interrupt needs.
NIL_WORKING_AREA(waThread4, 64);

// Declare thread function for thread 4.
NIL_THREAD(Thread4, arg) {

  while (TRUE) {
  
    // Sleep so lower priority threads can execute.
    nilThdSleep(100);
    cdiState = digitalRead(5);
    if (cdiState != lastcdiState)
    {

      if (cdiState == HIGH)
      {
        cdipulseState = 1;
      }
      else
      {
        cdipulseState = 0;
      }
    }
    lastcdiState = cdiState;
  }  
}
//--est inj pulse---------------------------------------------------------------
// Declare a stack with 64 bytes beyond context switch analogRead interrupt needs.
NIL_WORKING_AREA(waThread5, 64);

// Declare thread function for thread 5.
NIL_THREAD(Thread5, arg) {

  while (TRUE) {
  
    // Sleep so lower priority threads can execute.
    nilThdSleep(100);
    esttime = (0.125*VE*MAP*K)/(0.28705*120*TEMP*spec);
  }
}
//------------------------------------------------------------------------------
// Declare a stack with 64 bytes beyond context switch analogRead interrupt needs.
NIL_WORKING_AREA(waThread6, 64);

// Declare thread function for thread 6.
NIL_THREAD(Thread6, arg) {

  while (TRUE) {
  
    // Sleep so lower priority threads can execute.
    nilThdSleep(100);
    maxtime =  1/(Freq*4);// 0.25 from period
    
  }
}
//------------------------------------------------------------------------------
/*
 * Threads static table, one entry per thread.  A thread's priority is
 * determined by its position in the table with highest priority first.
 *
 * These threads start with a null argument.  A thread's name is also
 * null to save RAM since the name is currently not used.
 */
NIL_THREADS_TABLE_BEGIN()
NIL_THREADS_TABLE_ENTRY(NULL, Thread1, NULL, waThread1, sizeof(waThread1))
NIL_THREADS_TABLE_ENTRY(NULL, Thread2, NULL, waThread2, sizeof(waThread2))
NIL_THREADS_TABLE_ENTRY(NULL, Thread3, NULL, waThread3, sizeof(waThread3))
NIL_THREADS_TABLE_ENTRY(NULL, Thread4, NULL, waThread4, sizeof(waThread4))
NIL_THREADS_TABLE_ENTRY(NULL, Thread5, NULL, waThread5, sizeof(waThread5))
NIL_THREADS_TABLE_ENTRY(NULL, Thread6, NULL, waThread6, sizeof(waThread6))
NIL_THREADS_TABLE_END()
//------------------------------------------------------------------------------
void setup() {

  Serial.begin(9600);
  // start kernel
  nilSysBegin();
}
//------------------------------------------------------------------------------
// Loop is the idle thread.  The idle thread must not invoke any
// kernel primitive able to change its state to not runnable.
void loop() {
  nilPrintStackSizes(&Serial);
  nilPrintUnusedStack(&Serial);
  Serial.println();
  unsigned long timenow = millis();
  Serial.print(timenow);
  Serial.print(",");
  Serial.print(cdipulseState);
  Serial.print(",");
  Serial.print(Freq);
  Serial.print(",");
  Serial.print(maxtime);
  Serial.print(",");
  Serial.print(injTime);
  Serial.print(",");
  Serial.print(esttime);
  Serial.print(",");
  Serial.print(RAWMAP);
  Serial.print(",");
  Serial.print(MAP, 4);
  Serial.print(",");
  Serial.println(RAWTPS); 
 // nilThdDelayMilliseconds(1000);
}
//------------------------------------------------------------------------------

void RPMint() // interrupt handler
{
  NIL_IRQ_PROLOGUE();
  unsigned long currentMicros = micros();
  duration += currentMicros - previousMicros;
  previousMicros = currentMicros;
  pulsecount++;
  NIL_IRQ_EPILOGUE();
}