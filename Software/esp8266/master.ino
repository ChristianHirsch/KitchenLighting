#include<Wire.h>

#define STATUS_LED 5

void setup() {
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, HIGH);
  
  Wire.begin();

  digitalWrite(STATUS_LED, LOW);
}

byte ledValues[] = {0, 0, 0};
byte W_LED = 0;
const byte RED[]   = {255, 0, 0};
const byte GREEN[] = {0, 255, 0};
const byte BLUE[]  = {0, 0, 255};
const byte *FADE[] =
{
  RED,
  GREEN,
  BLUE
};

int color = 0;
int intep = 0;

void loop()
{
  computeValues();

  Wire.beginTransmission(69);
  Wire.write('s');
  Wire.write(ledValues[0]);
  Wire.write(ledValues[1]);
  Wire.write(ledValues[2]);
  Wire.write(W_LED);
  Wire.endTransmission();

  delay(25);
}

void computeValues(void)
{
  interpolateColors(ledValues, FADE[color % 3], FADE[(color + 1) % 3], intep);
  
  W_LED = 0;

  intep += 1;
  if(intep > 100)
  {
    intep = 0;
    color++;
  }
}

void interpolateColors(byte *result, const byte *color1, const byte *color2, int value)
{
  result[0] = ((100 - value) * color1[0] + value * color2[0]) / 100;
  result[1] = ((100 - value) * color1[1] + value * color2[1]) / 100;
  result[2] = ((100 - value) * color1[2] + value * color2[2]) / 100;
}

