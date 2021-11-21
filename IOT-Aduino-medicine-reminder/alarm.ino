#include <Adafruit_NeoPixel.h>

#define NOTE_C5 523
#define NOTE_D5 587
#define NOTE_E5 659
//Defining the notes' frequencies for the melody

#define interval 10000
//Time in millisecconds between first and second alarm

#define SIGNAL_PIN 3
#define STRIP_PIN 9
#define LED_COUNT 4
#define PIEZZO_PIN 10

#define MELODY_LENGTH 4

int melodies[2][MELODY_LENGTH] = 
{
  {NOTE_C5, NOTE_E5, NOTE_D5, 0},
  {NOTE_C5, NOTE_C5, NOTE_C5, NOTE_C5}
};

int noteDurations[] = 
{
  1000 / 8, 1000 / 8, 1000 / 8, 1000 /  2
};

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, STRIP_PIN, NEO_GRB + NEO_KHZ800);
unsigned long color = strip.Color(255, 255, 255);

void ledToggle()
{
  if(strip.getPixelColor(0) == strip.Color(0,0,0))
      {
        strip.fill(color);
      }
      else
      {
		strip.clear();
      }
    strip.show();
}

void playNote(int noteDuration, int noteToPlay)
{
      tone(PIEZZO_PIN, noteToPlay, noteDuration);
      delay(noteDuration * 1.30);

      noTone(PIEZZO_PIN);
}

void playMelodyAndLEDs(int position)
{
    for(int i = 0; i < MELODY_LENGTH; i++)
  {
      playNote(noteDurations[i], melodies[position][i]);
      ledToggle();
      delay(100);
      
  }  
}

void setup() 
{ 
  pinMode(SIGNAL_PIN, INPUT);
  pinMode(STRIP_PIN, OUTPUT);
  pinMode(PIEZZO_PIN, OUTPUT); 
  strip.begin();
}

void alarm(unsigned long timeSinceActivation)
{
   while(digitalRead(SIGNAL_PIN))
   {
       if(millis() - timeSinceActivation > interval)
      {
         color = strip.Color(255, 0, 0);      
         playMelodyAndLEDs(1);
      }
      else
      {
         color = strip.Color(255, 255, 0);
         playMelodyAndLEDs(0);
      } 
   }
}



void loop() 
{ 
  if(digitalRead(SIGNAL_PIN))
  {  
    alarm(millis());
  }
  delay(100);
}


