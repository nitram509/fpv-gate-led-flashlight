#include <FastLED.h>

#define TRIGGER_PIN 3 // Arduino Pin --> HC-SR04 Trig
#define ECHO_PIN 2    // Arduino Pin --> HC-SR04 Echo
#define PULSE_TO_CM_DIVIDER 58 // from HC-SR04 User's Manual v1.0

#define NUM_LEDS 24
#define LED_PIN 9
#define BUZZER_PIN 6

#define MEASURE_INTERVAL 20 // milliseconds a measurement should happen, max. HC-SR04 is 50 per second
#define MAX_DISTANCE_CM 34

#define FLY_THROUGH_GRACE_PERIOD 1000

#define DEBUG 0

//--------------------------------


CRGB leds[NUM_LEDS];

unsigned long last_measured = 0;
unsigned long flyThroughAt  = 0;
unsigned long time = 0;
bool flyThroughDetected = false;
int last_distance = 0;

void setup()
{
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(TRIGGER_PIN, HIGH); //Signal abschalten
  digitalWrite(TRIGGER_PIN, HIGH);

  Serial.begin(57600);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  time = millis();
}

int getDistanceInCentimeter()
{
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(3);
  noInterrupts();
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  interrupts();

  long distance = duration / PULSE_TO_CM_DIVIDER;
  return (distance);
}

void makeNoise()
{
  int delta = (time - flyThroughAt );
  if (delta > 500)
  {
    noTone(BUZZER_PIN);
  }
  else if (delta > 0)
  {
    tone(BUZZER_PIN, 2000 + abs(map(delta, 0, 500, -1000, 1000)));
  }
}

void animateLed()
{
  int delta = (time - flyThroughAt);
  if (delta > 500)
  {
    fill_solid(leds, NUM_LEDS, CHSV( 190, 255, 125));
    FastLED.show();
  }
  else if (delta > 400)
  {
    int value = abs(map(delta, 400, 500, 0, 125));
    fill_solid(leds, NUM_LEDS, CHSV( 190, 255, value));
    FastLED.show();
  }
  else if (delta > 100)
  {
    int value = 255 - abs(map(delta, 100, 400, -255, 255));
    fill_solid(leds, NUM_LEDS, CHSV( 0, 255, value));
    FastLED.show();
  }
  else if (delta > 0)
  {
    int value = abs(map(delta, 0, 100, 125, 0));
    fill_solid(leds, NUM_LEDS, CHSV( 190, 255, value));
    FastLED.show();
  }
}

void loop()
{
  time = millis();

  if ((time - last_measured) > MEASURE_INTERVAL)
  {
    last_measured = time;
    int distance = getDistanceInCentimeter();
    if (DEBUG)
    {
      Serial.write("distance:");
      Serial.print(distance, DEC);
      Serial.write(" cm\n");
    }

    if (distance != last_distance && distance < MAX_DISTANCE_CM)
    {
      if (!flyThroughDetected)
      {
        flyThroughAt  = time;
      }
      flyThroughDetected = true;
    }
  }

  if (flyThroughDetected)
  {
    int delta = (time - flyThroughAt);
    if (delta > FLY_THROUGH_GRACE_PERIOD)
    {
      flyThroughDetected = false;
    }
  }

  animateLed();
  makeNoise();
}




