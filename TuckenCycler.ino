#include <FastLED.h>
#include "pitches.h"

// LED stuff
#define LED_PIN     6
#define COLOR_ORDER RGB
#define CHIPSET     WS2811
#define NUM_LEDS    4

#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 200

CRGB leds[NUM_LEDS];

// Gun 

int buzzer = 8; 
int LEDpin = 13;

boolean working = 0;
boolean paused = 0;
boolean loaded = 0;
boolean loading = 0;
boolean wanttofire = 0 ;
boolean wanttoload = 0 ;
boolean marching = 0 ;

const int buttonFirePin = 2;
const int buttonChargePin = 3;

unsigned long previousMillis = 0;
const long pause_interval = 30000;

int fadeAmount = 5; 
int brightness = 0;

// Imperial
// notes in the melody:
int melody[] = {
  NOTE_G2,  NOTE_G2, NOTE_G2,  NOTE_G2,  NOTE_G2,  NOTE_G2,  NOTE_G2,  NOTE_G2,  NOTE_G2,  NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2,
  NOTE_G2,  NOTE_G2, NOTE_G2,  NOTE_G2,  NOTE_G2,  NOTE_G2,  NOTE_G2,  NOTE_G2,  NOTE_G2,  NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2,
  NOTE_G3,  NOTE_G3, NOTE_G3,  NOTE_DS3, NOTE_D3, NOTE_G3,   NOTE_DS3, NOTE_D3,  NOTE_G3,  0,        NOTE_AS3,  NOTE_AS3,  NOTE_AS3,  NOTE_DS4, NOTE_AS3, NOTE_FS3, NOTE_DS3, NOTE_D3, NOTE_G3,
  NOTE_G4,  NOTE_G3, NOTE_G3,  NOTE_D4,  NOTE_FS4, NOTE_F4,  NOTE_E4,  NOTE_B3, NOTE_E4,  0,        NOTE_GS3, NOTE_CS4, NOTE_C4,  NOTE_B3,
  NOTE_AS3, NOTE_A3, NOTE_AS3, 0,        NOTE_DS3, NOTE_FS3, NOTE_DS3, NOTE_FS3, NOTE_AS3, NOTE_G3,  NOTE_G3, NOTE_AS3,
  NOTE_G4,  NOTE_G3, NOTE_G3,  NOTE_D4,  NOTE_FS4, NOTE_F4,  NOTE_E4,  NOTE_B3, NOTE_E4,  0,        NOTE_GS3, NOTE_CS4, NOTE_C4,  NOTE_B3,
  NOTE_AS3, NOTE_A3, NOTE_AS3, 0,        NOTE_DS3, NOTE_FS3, NOTE_DS3, NOTE_D3,  NOTE_G3,  NOTE_DS3, NOTE_D3, NOTE_G3,
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  2, 4, 16, 16,16,4,16,16,16, 16,16,8,16,16,8,
  2, 4, 16, 16,16,4,16,16,16, 16,16,8,16,16,8,
  2, 2, 2, 3, 7, 2, 3, 7, 1, 16, 2, 2, 2, 3, 7, 2, 3, 7, 1,
  2, 3, 8, 2, 3, 8, 8, 8, 4, 8, 4, 2, 3, 8,
  8, 8, 4, 6, 4, 2, 3, 8, 2, 3, 8, 1,  
  2, 3, 8, 2, 3, 8, 8, 8, 4, 8, 4, 2, 3, 8,
  8, 8, 4, 6, 4, 2, 3, 8, 2, 3, 8, 1  
};

void setup() {
    delay(3000); // sanity delay
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );

    pinMode(LEDpin, OUTPUT);
    pinMode(buzzer, OUTPUT);
    pinMode(buttonFirePin, INPUT_PULLUP );
    pinMode(buttonChargePin, INPUT_PULLUP );   
      
    Serial.begin(9600);
    Serial.println("Starting loop");
    
    attachInterrupt(digitalPinToInterrupt(buttonFirePin), fire_handler, FALLING  );
    attachInterrupt(digitalPinToInterrupt(buttonChargePin), load_handler, FALLING  );   
}

void loop()
{
  unsigned long currentMillis = millis();

  if (!working && loaded && !paused && wanttofire) 
  {
    previousMillis = currentMillis;
    wanttofire = 0;
    fire();
  }

  if (!working && paused && wanttofire && !marching ) 
  {
    previousMillis = currentMillis;
    wanttofire = 0;
    march();
    
  }
  
  if (!working && !loaded  && wanttoload ) 
  {
    previousMillis = currentMillis;
    wanttoload = 0;
    loadgun();
  }
       
  if ( !working && paused && !loading)
  {
    delay(50);
    leds[0].fadeToBlackBy( 1 );
    leds[1].fadeToBlackBy( 1 );
    leds[2].fadeToBlackBy( 1 );
  }

  if ( !working && !paused && !loaded && !loading && leds[0])
  {
    leds[0].fadeToBlackBy( 1 );
    leds[1].fadeToBlackBy( 1 );
    leds[2].fadeToBlackBy( 1 );
    leds[3].fadeToBlackBy( 16 );

  }

  FastLED.show(); // display this frame
  FastLED.delay(20 / FRAMES_PER_SECOND);

  
  digitalWrite(LEDpin, paused); 
    
    if (currentMillis - previousMillis >= pause_interval && !working && !paused  && !loading) {
      previousMillis = currentMillis;
      pausegun();
    }

}

void load_handler()
{
  if (!wanttofire && !loading && !loaded) 
  {
    Serial.println("Want to Load");
    wanttoload = 1 ;
  }
} 

void fire_handler()
{
  if (loaded && !wanttoload ) 
  {
    Serial.println("Want to Fire");
    wanttofire = 1 ;
  }

  if (paused && !marching) 
  {
    Serial.println("Pause Fire");
    wanttofire = 1 ;
  }

  
} 

static void pausegun()
{ 
    Serial.println("Pause Gun");
    loaded = 0;
    paused = 1 ;
}

static void fire()
{
    Serial.println("Fire Start");
    working = 1 ;  
    leds[0] = CRGB::Purple;
    leds[1] = CRGB::Purple;
    leds[2] = CRGB::Purple;
    leds[3] = CRGB::Red;

    for (int i=880;i>200;i--) {
      tone(buzzer,i);
    }
    noTone(buzzer);
    loaded = 0;
    working = 0 ;
    Serial.println("Fire Finished");
}


static void loadgun()
{
    Serial.println("Load Start");
    working = 1 ;
    paused = 0 ;
    loading = 1 ;

    int t = 200 ;
    for(int x = 0; x < 3; x++){
    //start from blue
    for( int colorStep=0; colorStep <= 255; colorStep++ ) {
        int r = 0;
        int g = colorStep;
        int b = 255; 
        leds[x] = CRGB(r,g,b);
        FastLED.show();
        t++ ;
        tone(buzzer,t);
      }
    
    //into green
    for( int colorStep=255; colorStep >= 0; colorStep-- ) {
        int r = 0;
        int g = 255;
        int b = colorStep; 
        leds[x] = CRGB(r,g,b);
        FastLED.show();
        t++ ;
        tone(buzzer,t);
      }

      t = t -200 ;
    leds[x].fadeToBlackBy( 230 );
    delay(25);
    }

    noTone(buzzer);

    loading = 0 ;
    loaded = 1;
    working = 0 ;
    Serial.println("Load Finished");

}

void march() {

  Serial.println("Playing March");
  working =1 ;
  marching = 1 ;
 
  for (int thisNote = 0; thisNote < 101; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(buzzer, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(buzzer);
  }
  marching = 0 ;
  working = 0 ;
  wanttofire = 0 ;
  Serial.println("March Done");
}

