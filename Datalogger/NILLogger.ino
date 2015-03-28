//RPM counter using ADC with ADC measurement at the same time
// target input 'A2'

#include <NilRTOS.h>
#include <NilAnalog.h>
#include <NilSerial.h>
#define Serial NilSerial

volatile byte newData = 0;
volatile byte prevData = 0;

//freq variables
volatile unsigned int timer = 0;//counts period of wave
volatile double period;
volatile float frequency;
volatile int rpm;
//------------------------------------------------------------------------------
// Declare a stack with 64 bytes beyond context switch and interrupt needs.
NIL_WORKING_AREA(waThread1, 64);

// Declare thread function for thread 1.
NIL_THREAD(Thread1, arg) {

  while (TRUE) {
  
    // Sleep so lower priority threads can execute.
    nilThdSleep(100);
    
    // Dummy use of CPU - use nilThdSleep in normal app.
    int val = analogRead(2);
    Serial.print(val);
    Serial.print("\t");
    val = nilAnalogRead(2);
    Serial.print(val);
    Serial.print("\t");
  }
}
//------------------------------------------------------------------------------
// Declare a stack with 64 bytes beyond context switch and interrupt needs.
NIL_WORKING_AREA(waThread2, 64);

// Declare thread function for thread 2.
NIL_THREAD(Thread2, arg) {
srpm();
  while (TRUE) {

    // Sleep so lower priority threads can execute.
  nilThdSleep(100);
  frequency = 38462/period;//timer rate/period
  rpm = (int)(frequency*60);
  
  Serial.println(rpm);
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
    nilThdDelay(100);
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

void srpm()
{
  cli();//diable interrupts

  ADCSRA = 0;
  ADCSRB = 0;
  
  ADMUX |= (1 << REFS0); //set reference voltage
  ADMUX |= (1 << ADLAR); //left align the ADC value- so we can read highest 8 bits from ADCH register only
  ADMUX |= (1 << MUX1); //ADC2- analog 2
  
  ADCSRA |= (1 << ADPS2) | (1 << ADPS0); //set ADC clock with 32 prescaler- 16mHz/32=500kHz
  ADCSRA |= (1 << ADATE); //enabble auto trigger
  ADCSRA |= (1 << ADIE); //enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN); //enable ADC
  ADCSRA |= (1 << ADSC); //start ADC measurements
  
  sei();//enable interrupts
 }

 ISR(ADC_vect) 
 {//when new ADC value ready

  prevData = newData;//store previous value
  newData = ADCH;//get value from A0
  if (prevData < 127 && newData >=127){//if increasing and crossing midpoint
    period = timer;//get period
    timer = 0;//reset timer
  }
  
  timer++;//increment timer at rate of 38.5kHz
}