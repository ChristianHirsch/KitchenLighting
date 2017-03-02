/*
 Fading

 This example shows how to fade an LED using the analogWrite() function.

 The circuit:
 * LED attached from digital pin 9 to ground.

 Created 1 Nov 2008
 By David A. Mellis
 modified 30 Aug 2011
 By Tom Igoe

 http://www.arduino.cc/en/Tutorial/Fading

 This example code is in the public domain.

 */
#define R_LED 3
#define G_LED 6
#define B_LED 9
#define W_LED 10

int ledValues[] =
{
  0, 0, 0, 0
};

int i = 0;

void setup() {
  // nothing happens in setup
}

void loop() {
  if(i%10 == 0)
  {
    ledValues[0] = (ledValues[0] + 5) % 255;
  }
  if(i%15 == 0)
  {
    ledValues[1] = (ledValues[1] + 5) % 255;
  }
  if(i%20 == 0)
  {
    ledValues[2] = (ledValues[2] + 5) % 255;
  }
  if(i%25 == 0)
  {
    ledValues[3] = (ledValues[3] + 5) % 255;
  }
  
  analogWrite(R_LED, ledValues[0]);
  analogWrite(G_LED, ledValues[1]);
  analogWrite(B_LED, ledValues[2]);
  analogWrite(W_LED, ledValues[3]);
  
  i++;
  delay(1);
}


