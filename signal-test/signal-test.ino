#define LOG_OUT 1
#include <FHT.h>
#define SAMPLES 256
#define DEBUG_BINS 32
//#define OUTPUT_RAW_DATA

void setup()
{
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0b11100101; // set the adc to free running mode, prescaler = 32 (doesn't matter)
  ADMUX = 0b01000111; // aref, internal Vref turned off, A7
  DIDR0 = 0b11111111; // turn off all digital inputs, we don't need them
  Serial.begin(9600);
}

void loop()
{
  while(1) // supposedly reduces jitter
  {
    cli();
    for (int i = 0; i < SAMPLES; i++)
    {
      while(!(ADCSRA & 0b00010000)); // wait for adc to be ready
      ADCSRA = 0b11110101; // restart adc
      byte m = ADCL; // fetch adc data
      fht_input[i] = (ADCH << 8) | m; // form into an int
      //fht_input[i] -= 0x0200;
      fht_input[i] <<= 6; // form into a 16b signed int
      #ifdef OUTPUT_RAW_DATA
        Serial.println(fht_input[i]);
      #endif
    }
    fht_window();
    fht_reorder();
    fht_run();
    fht_mag_log();
    sei();

    #ifndef OUTPUT_RAW_DATA
      for(int i = 0; i < DEBUG_BINS; i++)
      {
        Serial.print(fht_log_out[i]);
        Serial.print(',');
      }
      Serial.println();
    #endif
  }
}
