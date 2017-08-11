#define LOG_OUT 1
#include <FHT.h>
#include <LedControlMS.h>

#define DIN_PIN 12
#define CS_PIN 11
#define CLK_PIN 10

#define DISPLAYCNT 4
#define BRIGHTNESS 0 // 0-15
#define COLUMNS 32
#define HEIGHT 8
#define SAMPLES 256
#define OFFSET 60 // trim output
#define OUTPUT_DIV 7 // divide output to match 8 leds

LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, DISPLAYCNT);
const byte levels[] = { 0, 128, 192, 224, 240, 248, 252, 254, 255 }; // full bars
//const byte levels[] = { 0, 128, 64, 32, 16, 8, 4, 2, 1 }; // only top led
const char borders[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 23, 25, 28, 31, 35, 39, 43, 48, 53, 59, 66, 74, 82, 92, 102, 114, 127 }; // frequency borders limits, first 2 are bonkers, just ignore them
char columns[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void setup()
{
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0b11100101; // set the adc to free running mode, prescaler = 32 (doesn't matter lol)
  ADMUX = 0b01000111; // aref, internal Vref turned off, A7
  DIDR0 = 0b11111111; // turn off all digital inputs, we don't need them
  for(int i = 0; i < DISPLAYCNT; i++)
  {
    lc.shutdown(i, false);
    lc.setIntensity(i, BRIGHTNESS);
    lc.clearDisplay(i);
  }
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
      fht_input[i] <<= 6; // form into a 16b signed int
    }
    fht_window();
    fht_reorder();
    fht_run();
    fht_mag_log();
    sei();

    for(int i = 0; i < COLUMNS; i++)
    {
      int maxH = 0;
      for(int x = borders[i]; x < borders[i+1]; x++)
      {
        int h = (fht_log_out[x] - OFFSET)/OUTPUT_DIV;
        if(h > 8) h = 8;
        if(h < 0) h = 0;
        if(h > maxH) maxH = h;
      }
      
      bool changeded = false;
      if(maxH > columns[i])
      {
        columns[i] = maxH; // rise bar instantly
        // columns[i]++; // alt rising 'animation' (should also check if columns[i] < 8)
        changeded = true;
      }
      else if(maxH < columns[i] && columns[i] > 0)
      {
        columns[i]--; // fading 'animation'
        changeded = true;
      }
      
      if(changeded) lc.setRow(i / HEIGHT, i % HEIGHT, levels[columns[i]]); // update only if height changed
    }
  }
}

