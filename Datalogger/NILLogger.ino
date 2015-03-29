//RPM counter using ADC with ADC measurement at the same time
// target input 'A2'

#include <NilRTOS.h>
#include <NilSerial.h>
#define Serial NilSerial

volatile unsigned long injOn = 0;
volatile unsigned long injOff = 0;
volatile int lastinjState = 0;
volatile int injState = 0;
//RPM counter////////////////////////////////////////////////////////////////////////////////////////////
long previousMillis = 0; // will store last time of the cycle end
volatile unsigned long duration=0; // accumulates pulse width
volatile unsigned int pulsecount=0;
volatile unsigned long previousMicros=0;
volatile float Freq;
volatile unsigned long _duration = duration;
volatile unsigned long _pulsecount = pulsecount;

volatile int val;
//------------------------------------------------------------------------------
// Declare a stack with 64 bytes beyond context switch and interrupt needs.
NIL_WORKING_AREA(waThread1, 64);

// Declare thread function for thread 1.
NIL_THREAD(Thread1, arg) {

  while (TRUE) {
  
    // Sleep so lower priority threads can execute.
    nilThdSleep(100);
    
    // Dummy use of CPU - use nilThdSleep in normal app.
    val = analogRead(2);
    Serial.print(val);
    Serial.print("\t");
  }
}
//------------------------------------------------------------------------------
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
          Freq *= _pulsecount; // calculate F
        }
  }
}
//------------------------------------------------------------------------------
// Declare a stack with 64 bytes beyond context switch and interrupt needs.
NIL_WORKING_AREA(waThread3, 64);

// Declare thread function for thread 3.
NIL_THREAD(Thread3, arg) {

  while (TRUE) {

    // Sleep so lower priority threads can execute.
    nilThdSleep(100);

    // Dummy use of CPU - use nilThdSleep in normal app.
    injState = digitalRead(pin);
    if (injState != lastinjState)
    {

    if (injState == HIGH)
      {
        Serial.print();
        millis = injOn;
      }
      else
      {
        millis = injOff;
      }
    }
    lastinjState = injState;
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

  // Delay for one second.
  // Must not sleep in loop so use nilThdDelayMilliseconds().
  // Arduino delay() can also be used in loop().
  nilThdDelayMilliseconds(1000);
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