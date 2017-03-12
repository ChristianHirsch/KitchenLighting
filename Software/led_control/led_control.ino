/*
 */

#include <Wire.h>

#define R_LED 3
#define G_LED 6
#define B_LED 9
#define W_LED 10

#define T_LED 12

void (*mainLoop)(void);

void noLoop(void);
void fadeLoop(void);
void musicLoop(void);

int i = 0;
uint8_t led = true;

typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} color_t;

uint8_t numFadeColors = 3;

const color_t RED   = {255, 0, 0};
const color_t GREEN = {0, 255, 0};
const color_t BLUE  = {0, 0, 255};

typedef struct colorNode {
  color_t color;
  struct colorNode *next;
} colorNode_t;

colorNode_t *fadingColors = NULL;

void setupFadingColors(void)
{
  colorNode_t *color1 = (colorNode_t *)malloc(sizeof(colorNode_t));
  color1->color = RED;

  colorNode_t *color2 = (colorNode_t *)malloc(sizeof(colorNode_t));
  color2->color = GREEN;

  colorNode_t *color3 = (colorNode_t *)malloc(sizeof(colorNode_t));
  color3->color = BLUE;

  color1->next = color2;
  color2->next = color3;
  color3->next = color1;

  fadingColors = color1;
}

void setup()
{ 
  Wire.begin(69);
  Wire.onReceive(receiveEvent);
  
  pinMode(T_LED, INPUT);
  //digitalWrite(T_LED, HIGH);

  pinMode(13, OUTPUT);
  digitalWrite(13, led);

  analogWrite(W_LED, 0);

  setupFadingColors();
  mainLoop = fadeLoop;
}

enum {
  NORMAL, FADE
} state;

void loop()
{
  mainLoop();
}

void receiveEvent(int howMany)
{  
  char c = Wire.read();
  int white = 0;
  switch(c)
  {
    case 's':
      analogWrite(R_LED, (byte) Wire.read());
      analogWrite(G_LED, (byte) Wire.read());
      analogWrite(B_LED, (byte) Wire.read());
      analogWrite(W_LED, (byte) Wire.read());
      break;
    case 'm':
      mainLoop = musicLoop;
      break;
    case 'f':
      numFadeColors = Wire.read();
      for(uint8_t i=0; i < numFadeColors; i++)
      {
        Wire.read(); // red
        Wire.read(); // green
        Wire.read(); // blue
      }
      mainLoop = fadeLoop;
      break;
    case 'w':
      analogWrite(W_LED, (byte) Wire.read());
      break;
    default:
      mainLoop = noLoop;
      analogWrite(R_LED, 0);
      analogWrite(G_LED, 0);
      analogWrite(B_LED, 0);
      analogWrite(W_LED, 255);
      break;
  }
}

void noLoop(void)
{
  delay(100);
}

void musicLoop()
{
  delay(25);
}

void fadeLoop(void)
{
  static int intep = 0;
  static colorNode_t *color = fadingColors;

  color_t ledValues;
  interpolateColors(&ledValues,
    &(color->color),
    &(color->next->color),
    intep);

  analogWrite(R_LED, ledValues.red);
  analogWrite(G_LED, ledValues.green);
  analogWrite(B_LED, ledValues.blue);

  intep += 1;
  if(intep > 100)
  {
    intep = 0;
    color = color->next;
  }

  digitalWrite(13, led = !led);
  
  delay(25);
}

void interpolateColors(color_t *const result, color_t const *const color1, color_t const *const color2, int const value)
{
  result->red   = ((100 - value) * color1->red   + value * color2->red)   / 100;
  result->green = ((100 - value) * color1->green + value * color2->green) / 100;
  result->blue  = ((100 - value) * color1->blue  + value * color2->blue)  / 100;
}
