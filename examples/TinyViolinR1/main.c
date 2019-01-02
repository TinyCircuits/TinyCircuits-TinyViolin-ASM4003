#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>

#define true 1
#define false 0

int main(void);
unsigned int getCapReading(uint8_t pin);
unsigned int capRead(uint8_t pin);


uint8_t counter = 0;
uint8_t countMax = 30;
uint8_t buttonPressed = 0;
uint16_t capThresh = 535;

#define numCapChannels 9
int capTouchCal[numCapChannels];
int capTouchCurrent[numCapChannels];
int overCalCount[numCapChannels];
void play_RTTTL(const char* p);

unsigned long avg;
const int amtAvg = 5;
int avgBuf[5];
int avgPos = 0;

#include "pitches.h"
#include "songs.h"

uint16_t notesAll[7*5]={NOTE_C3,NOTE_D3,NOTE_E3,NOTE_F3,NOTE_G3,NOTE_A3, NOTE_B3,
                   NOTE_C4,NOTE_D4,NOTE_E4,NOTE_FS4,NOTE_G4,NOTE_A4, NOTE_B4,
                   NOTE_C5,NOTE_D5,NOTE_E5,NOTE_FS5,NOTE_G5,NOTE_A5, NOTE_B5,
                   NOTE_C6,NOTE_D6,NOTE_E6,NOTE_FS6,NOTE_G6,NOTE_A6, NOTE_B6,
                   NOTE_C7,NOTE_D7,NOTE_E7,NOTE_F7,NOTE_G7,NOTE_A7, NOTE_B7};

int maxReading = 0;
int primary = 0;
    
void updateCapTouch(){
    maxReading = 0;
    primary = 0;
    int j;
    for (j = 0; j < numCapChannels; j++) {
    int val = getCapReading(j);
    if (val < capTouchCal[j]) {
      capTouchCal[j] = val;
    } else if (val > capTouchCal[j] + 1) {
      overCalCount[j]++;
      if (overCalCount[j] > 3) {
        overCalCount[j] = 0;
        capTouchCal[j] += 1;
      } else {
        //overCalCount[j]=0;
      }
      
      if (j<9 && capTouchCurrent[j] > maxReading) {
        maxReading = capTouchCurrent[j];
        primary = j;
      }
    }
    capTouchCurrent[j] = val  - capTouchCal[j];
  }
}
int main(void) {
  int biggestMagnitude = 0;
  int isTouch = 0;
  int initialTouchPos = 0;
  int finalTouchPos = 0;
  int currentTouchedPositions = 0;

  CCP = 0xD8;
  CLKPR = 0;

  
  DDRA=(1<<1);
  ADCSRA = _BV(ADEN) | 4 | 1;//250KHz clock

  int pin, i, j;
  for (pin = 0; pin < numCapChannels; pin++) {
    capTouchCal[pin] = 1000;//getCapReading(pin);
    overCalCount[pin] = 0;
  }
  
  TOCPMSA0 = _BV(TOCC0S1);
  TCCR2A = _BV(COM2A1)|_BV(COM2B1)|_BV(WGM21);
  TCCR2B = _BV(WGM23)|_BV(WGM22)|_BV(CS20);
  //TIFR2 = _BV(TOV2);
  //31250
  uint8_t pulseWidth=128;
  
  ICR2 = 255-1;
  
    if(pulseWidth){
      OCR2B=pulseWidth-1;
      TOCPMCOE|=((1<<TOCC0OE));
    }else{
      TOCPMCOE&=~((1<<TOCC0OE));
      OCR2B=0;
    }
    
  
  while (1)
  {
    _delay_ms(10);
  
    updateCapTouch();
    
    int ledToLight = primary*3;
    if (primary < 5)
      ledToLight += (capTouchCurrent[primary + 1] / (maxReading/2));
    if (primary > 0)
      ledToLight -= (capTouchCurrent[primary - 1] / (maxReading/2));
      
    int completedTouch = 0;
    int onSwipe = 0;
    int offSwipe = 0;
      
    uint16_t tone=0;
    
    int s1=capTouchCurrent[5];int s1v=30;
    int s2=capTouchCurrent[6];int s2v=85;
    int s3=capTouchCurrent[7];int s3v=12;
    int s4=capTouchCurrent[0];int s4v=40;
      
    if(s1>s1v && s1-s1v>s2-s2v){
      tone=4;
    }
    if(s4>s4v && s4-s4v>s3-s3v){
      tone=14+2;
    }
    if(s3>s3v && s3-s3v>=s2-s2v){// s3-s3v>=s4-s4v && 
      tone=7+5;
    }
    if(s2>s2v && s2-s2v>=s3-s3v){// && s2-s2v>=s1-s1v
      tone=7+1;
    }
    
    if(tone){
      if(capTouchCurrent[4]>100 && capTouchCurrent[3]>110/2){
        tone+=4;
      }
      else if(capTouchCurrent[3]>110){
        tone+=3;
      }
      else if(capTouchCurrent[2]>110){
        tone+=2;
      }
      else if(capTouchCurrent[1]>100){
        tone+=1;
      }
    }
    if(!tone){
      if(capTouchCurrent[8]>150){
        TOCPMCOE|=((1<<TOCC0OE));
          
          unsigned int rand=0;
          for (j = 0; j < numCapChannels; j++) {
            rand+=capTouchCurrent[j]+capTouchCal[j];
          }
          while(rand>9)rand-=9;
          if(rand==0){
            play_RTTTL(song_0);
          }else if(rand==1){
            play_RTTTL(song_1);
          }else if(rand==2){
            play_RTTTL(song_2);
          }else if(rand==3){
            play_RTTTL(song_3);
          }else if(rand==4){
            play_RTTTL(song_4);
          }else if(rand==5){
            play_RTTTL(song_5);
          }else if(rand==6){
            play_RTTTL(song_6);
          }else if(rand==7){
            play_RTTTL(song_7);
          }else if(rand==8){
            play_RTTTL(song_8);
          }else if(rand==9){
            play_RTTTL(song_9);
          }
      }
    }
    if(tone){
      tone=8000000ul/(unsigned long)notesAll[tone];
      if(ICR2!=tone){
        ICR2=tone;
        OCR2B=tone/2;
        TOCPMCOE|=((1<<TOCC0OE));
        }
      if (!isTouch) {
        isTouch = 1;
        initialTouchPos = primary;
        finalTouchPos = primary;
        currentTouchedPositions=0;
      } else {
        finalTouchPos = primary;
        for (j = 0; j < 6; j++) {
          if(capTouchCurrent[j]>10)
            currentTouchedPositions|=(1<<j);
         }
      }
    } /*else {
      if (isTouch) {
        isTouch = 0;
        completedTouch = 1;
      }
    }*/else{
      if(ICR2!=0){
        TOCPMCOE&=~((1<<TOCC0OE));
        ICR2=0;
        PORTA=0;
        }
    }
    
  }
  return 0;
}

unsigned int getCapReading(uint8_t pin) {
  unsigned int i, avg=0;
  for (i = 0; i < 10; i++) {
    avg += capRead(pin);
  }
  avg /= 10;
  return avg;
}

unsigned int capRead(uint8_t pin) {
  while(PINA&(1<<1));
  _delay_us(1);
  if(pin>0)pin+=1;
  
  if(pin==8){
    pin=11;
    PUEB |= (1 << (0)); //charge external cap
  }else if(pin==9){
    pin=10;
    PUEB |= (1 << (1)); //charge external cap
  }else{
    PUEA |= (1 << pin); //charge external cap
  }

  ADMUXA = 0x0E; //connect ADC input cap to GND
  ADCSRA |= _BV(ADSC); //run ADC
  while (ADCSRA & _BV(ADSC));
  
  if(pin==10){
    PUEB &= ~(1 << (1)); //float external cap
  }else if(pin==11){
    PUEB &= ~(1 << (0)); //float external cap
  }else{
    PUEA &= ~(1 << pin); //float external cap
  }
  

  ADMUXA = (pin) & 0x0F; //connect ADC cap to external cap
  ADCSRA |= _BV(ADSC); //run ADC
  while (ADCSRA & _BV(ADSC));

  unsigned int result = ADCL; //result is proportional to external/internal
  return (ADCH << 8) | result;
}
#define byte uint8_t
#define OCTAVE_OFFSET 0


int tone_pin = 6;
volatile uint32_t count = 0;
int prev = 0;

void delay(int dcount){
  int c=0;
  for(;c<dcount;c++){
    _delay_us(999);
  }
}

void tone(int pin, int note)
{
  uint16_t val = 8000000ul/(unsigned long)note;
  ICR2 = val;
  OCR2B = val/2;

}
 
 void noTone(int pin){
 
  ICR2 = 0;
  OCR2B = 0;
 
 
 }
 

#define isdigit(n) (n >= '0' && n <= '9')


void play_RTTTL(const char* p){
 
  byte default_dur = 4;
  byte default_oct = 6;
  int bpm = 63;
  int num;
  long wholenote;
  long duration;
  byte note;
  byte scale;
 
   // Absolutely no error checking in here


  // format: d=N,o=N,b=NNN:
  // find the start (skip name, etc)

  while(pgm_read_byte(p) != ':')p++; // ignore name
  p++;                     // skip ':'

  // get default duration
  if(pgm_read_byte(p) == 'd')
  {
    p++; p++;              // skip "d="
    num = 0;
    while(isdigit(pgm_read_byte(p)))
    {
      num = (num * 10) + (pgm_read_byte(p) - '0');
      p++;
    }
    if(num > 0) default_dur = num;
    p++;                   // skip comma
  }

  // get default octave
  if(pgm_read_byte(p) == 'o')
  {
    p++; p++;              // skip "o="
    num = pgm_read_byte(p) - '0';
    p++;
    if(num >= 3 && num <=7) default_oct = num;
    p++;                   // skip comma
  }

  // get BPM
  if(pgm_read_byte(p) == 'b')
  {
    p++; p++;              // skip "b="
    num = 0;
    while(isdigit(pgm_read_byte(p)))
    {
      num = (num * 10) + (pgm_read_byte(p) - '0');
      p++;
    }
    bpm = num;
    p++;                   // skip colon
  }

  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole note (in milliseconds)


  // now begin note loop
  while(pgm_read_byte(p))
  {
    // first, get note duration, if available
    num = 0;
    while(isdigit(pgm_read_byte(p)))
    {
      num = (num * 10) + (pgm_read_byte(p) - '0');
      p++;
    }
    
    if(num) duration = wholenote / num;
    else duration = wholenote / default_dur;  // we will need to check if we are a dotted note after

    // now get the note
    note = 0;

    switch(pgm_read_byte(p))
    {
      case 'c':
        note = 1;
        break;
      case 'd':
        note = 3;
        break;
      case 'e':
        note = 5;
        break;
      case 'f':
        note = 6;
        break;
      case 'g':
        note = 8;
        break;
      case 'a':
        note = 10;
        break;
      case 'b':
        note = 12;
        break;
      case 'p':
      default:
        note = 0;
    }
    p++;

    // now, get optional '#' sharp
    if(pgm_read_byte(p) == '#')
    {
      note++;
      p++;
    }

    // now, get optional '.' dotted note
    if(pgm_read_byte(p) == '.')
    {
      duration += duration/2;
      p++;
    }
  
    // now, get scale
    if(isdigit(pgm_read_byte(p)))
    {
      scale = pgm_read_byte(p) - '0';
      p++;
    }
    else
    {
      scale = default_oct;
    }

    scale += OCTAVE_OFFSET;

    if(pgm_read_byte(p) == ',')
      p++;       // skip comma for next note (or we may be at the end)

    // now play the note

    if(note)
    {
      tone(tone_pin, notes[(scale - 4) * 12 + note]); // original - 4, not - 5 
      delay(duration);
      noTone(tone_pin);
    }
    else
    {
      delay(duration);
    }
  }
}
 
