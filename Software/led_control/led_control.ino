/*
 */

#include <Wire.h>

// PWM pins for led stripe
#define R_LED_PIN 3
#define G_LED_PIN 5
#define B_LED_PIN 6
#define W_LED_PIN 9
#define U_LED_PIN 10

#define S_230_PIN1 12
#define S_230_PIN2 13

#define M7_STROBE 2
#define M7_RESET 4
#define M7_DC A0

#define U_C 0
#define W_C 1
#define R_C 2
#define G_C 3
#define B_C 4

void (*mainLoop)(void);

void noLoop(void);
void fadeLoop(void);
void musicLoop(void);

int i = 0;
uint8_t led = true;

int equalizer[7] = { 0 };

bool music_hit = false;

typedef struct {
  enum {OFF = 0, ON, DISCO, FLASH} mode;
  uint8_t pin;
  uint8_t value;
  unsigned long last_flash;
} led_channel_t;

led_channel_t led_channel[5];

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
  uint8_t intep;
  color_t color;
  struct  colorNode *next;
} colorNode_t;

colorNode_t *fadingColors = NULL;
colorNode_t *discoColors = NULL;

color_t fadeValues;
color_t discoValues;

void setupFadingColors(void)
{
  // fading
  colorNode_t *color1 = (colorNode_t *)malloc(sizeof(colorNode_t));
  color1->intep = 0;
  color1->color = RED;

  colorNode_t *color2 = (colorNode_t *)malloc(sizeof(colorNode_t));
  color2->intep = 0;
  color2->color = GREEN;

  colorNode_t *color3 = (colorNode_t *)malloc(sizeof(colorNode_t));
  color3->intep = 0;
  color3->color = BLUE;

  color1->next = color2;
  color2->next = color3;
  color3->next = color1;

  fadingColors = color1;

  // disco
  color1 = (colorNode_t *)malloc(sizeof(colorNode_t));
  color1->intep = 0;
  color1->color = RED;

  color2 = (colorNode_t *)malloc(sizeof(colorNode_t));
  color2->intep = 0;
  color2->color = GREEN;

  color3 = (colorNode_t *)malloc(sizeof(colorNode_t));
  color3->intep = 0;
  color3->color = BLUE;

  color1->next = color2;
  color2->next = color3;
  color3->next = color1;

  discoColors = color1;
}

void readMSGEQ7()
{
  digitalWrite(M7_RESET, HIGH);
  digitalWrite(M7_RESET, LOW);
  for(int band=0; band < 7; band++)
  {
    digitalWrite(M7_STROBE, LOW);
    delayMicroseconds(50);
    
    equalizer[band] = analogRead(M7_DC);
    equalizer[band] = constrain(equalizer[band], 250, 1023);
    equalizer[band] = map(equalizer[band], 250, 1023, 0, 250);
    
    digitalWrite(M7_STROBE, HIGH);
  }
}

void setup()
{
  Serial.begin(9600);
  
  Wire.begin(69);
  Wire.onReceive(receiveEvent);
  
  pinMode(S_230_PIN1, OUTPUT);
  digitalWrite(S_230_PIN1, LOW);
  pinMode(S_230_PIN2, OUTPUT);
  digitalWrite(S_230_PIN2, LOW);

  setupFadingColors();

  // MSGEQ 7
  pinMode(M7_RESET, OUTPUT);
  pinMode(M7_STROBE, OUTPUT);

  led_channel[U_C].pin = U_LED_PIN;
  led_channel[W_C].pin = W_LED_PIN;
  led_channel[R_C].pin = R_LED_PIN;
  led_channel[G_C].pin = G_LED_PIN;
  led_channel[B_C].pin = B_LED_PIN;

  for(int i=0; i < 5; i++)
  {
    led_channel[i].mode = led_channel[i].OFF;
    led_channel[i].value = 0;
    led_channel[i].last_flash = 0;
    pinMode(led_channel[i].pin, OUTPUT);
    analogWrite(led_channel[i].pin, led_channel[i].value);
  }
  
  led_channel[2].mode = led_channel[2].FLASH;
  led_channel[2].value = 255;
  led_channel[3].mode = led_channel[3].FLASH;
  led_channel[3].value = 255;
  led_channel[4].mode = led_channel[4].FLASH;
  led_channel[4].value = 255;
}

enum {
  NORMAL, FADE
} state;

uint8_t getColorValue(color_t color, uint8_t idx)
{
  if(idx == 0)
    return color.red;
  if(idx == 1)
    return color.green;
  return color.blue;
}

void loop()
{
  static unsigned long past = 0;
  static unsigned long last_fade = 0;
  
  unsigned long now = millis();
  unsigned long dt = now - past;
  
  readMSGEQ7();
  musicLoop();

  if((now - last_fade) > 25)
  {
    last_fade = now;
    fadeMainLoop();
  }

  for(int i=0; i < 2; i++)
  {
    switch(led_channel[i].mode)
    {
      case led_channel[i].ON:
        analogWrite(led_channel[i].pin, led_channel[i].value);
        break;
      case led_channel[i].DISCO:
        unsigned long flash_dt; flash_dt = now - led_channel[i].last_flash;
        if(music_hit)
        {
          led_channel[i].last_flash = now;
          analogWrite(led_channel[i].pin, 255);
        }
        else if(flash_dt > 10)
          analogWrite(led_channel[i].pin, 0);
        break;
      case led_channel[i].FLASH:
        flash_dt = now - led_channel[i].last_flash;
        if(flash_dt > map(led_channel[i].value, 0, 255, 10, 255) * 3)
        {
          led_channel[i].last_flash = now;
          analogWrite(led_channel[i].pin, 255);
        }
        else if(flash_dt > 10)
        {
          analogWrite(led_channel[i].pin, 0);
        }
        break;
      default:
        analogWrite(led_channel[i].pin, 0);
    }
  }
  for(int i=0; i < 3; i++)
  {
    uint8_t id = i + 2;
    switch(led_channel[id].mode)
    {
      case led_channel[id].ON:
        analogWrite(led_channel[id].pin, led_channel[id].value);
        break;
      case led_channel[id].DISCO:
        analogWrite(led_channel[id].pin, map(getColorValue(discoValues, i), 0, 255, 0, led_channel[id].value));
        break;
      case led_channel[id].FLASH:
        analogWrite(led_channel[id].pin, map(getColorValue( fadeValues, i), 0, 255, 0, led_channel[id].value));
        break;
      default:
        analogWrite(led_channel[id].pin, 0);
    }
  }
  past = now;
}

void receiveEvent(int howMany)
{
  uint8_t channel = Wire.read();
  Serial.println(channel);
  
  if(channel >= 0 && channel < 5)
  {
    char type = Wire.read();
    if(type == 'm')
      led_channel[channel].mode = Wire.read();
    else if(type == 'v')
      led_channel[channel].value = Wire.read();

    Serial.print(": mode: ");
    Serial.println(led_channel[channel].mode);
    Serial.print(" / value: ");
    Serial.println(led_channel[channel].value);
  }
  else if(channel == 5)
  {
    if(Wire.read() == 1)
      digitalWrite(S_230_PIN1, HIGH);
    else
      digitalWrite(S_230_PIN1, LOW);
  }
  else if(channel == 6)
  {
    if(Wire.read() == 1)
      digitalWrite(S_230_PIN2, HIGH);
    else
      digitalWrite(S_230_PIN2, LOW);
  }
}

void noLoop(void)
{
  delay(100);
}

colorNode_t *fadeLoop(colorNode_t *color_node, color_t *color, uint8_t intep_inc = 1)
{
  interpolateColors(color,
    &(color_node->color),
    &(color_node->next->color),
    min(color_node->intep, 100));
    
  if(color_node->intep > 100)
  {
    color_node->intep = 0;
    return color_node->next;
  }

  color_node->intep += intep_inc;
  return color_node;
}

#define AVG_SAMPLES 512

void musicLoop()
{
  static unsigned long past = 0;
  unsigned long now = millis();
  unsigned long dt = now - past;
  
  music_hit = false;
  
  static int avg = 0;
  static int last_kick = 0;

  static uint8_t bass_avg[3] = {0};

  int b_value = equalizer[0];
  
  static uint8_t ib=0;
  ib++;
  ib = ib % 3;
  bass_avg[ib] = equalizer[0];

  static bool got_low = true;
  static bool got_high = false;
  bool hit = false;

  static uint8_t last_high = 255;
  static uint8_t last_low = 0;

  // max detection
  if(  bass_avg[(ib + 2) % 3] > bass_avg[(ib + 0) % 3]
    && bass_avg[(ib + 2) % 3] > bass_avg[(ib + 1) % 3])
  {
    if(bass_avg[(ib + 2) % 3] > last_high)
      last_high = bass_avg[(ib + 2) % 3];
    
    if(!got_low || got_high)
      return;
    
    if((dt > 500)
      && (bass_avg[(ib + 2) % 3] > (last_low * 10)))
      {
        Serial.println("High");
        
        last_high = bass_avg[(ib + 2) % 3];
        got_low = false;
        got_high = true;
        hit = true;
      }
  }
  // min detection
  else if(   bass_avg[(ib + 2) % 3] < bass_avg[(ib + 0) % 3]
          && bass_avg[(ib + 2) % 3] < bass_avg[(ib + 1) % 3])
  {
    if(bass_avg[(ib + 2) % 3] < last_low)
      last_low = bass_avg[(ib + 2) % 3];
      
    if(!got_high || got_low)
      return;
    
    if(bass_avg[(ib + 2) % 3] < (last_high / 10))
      {
        Serial.println("Low");
        last_low = bass_avg[(ib + 2) % 3];
        got_low = true;
        got_high = false;
        //hit = true;
      }
  }
  // going down
  else if((bass_avg[(ib + 0) % 3] < bass_avg[(ib + 2) % 3]
    && bass_avg[(ib + 2) % 3] < bass_avg[(ib + 1) % 3]))
  {
    
  }
  // going up
  else
  {
    
  }
  /* */
  static unsigned long last_fade = 0;
  if(hit)
  {
    music_hit = true;
    past = now;
    discoColors = fadeLoop(discoColors, &discoValues, 10);
  } else if((dt > 10000) && (dt / 250) > last_fade)
  {
    last_fade = (dt / 250);
    discoColors = fadeLoop(discoColors, &discoValues, 1);
  }
}

void fadeMainLoop(void)
{
  fadingColors = fadeLoop(fadingColors, &fadeValues, 1);
}

void interpolateColors(color_t *const result, color_t const *const color1, color_t const *const color2, int const value)
{
  result->red   = ((100 - value) * color1->red   + value * color2->red)   / 100;
  result->green = ((100 - value) * color1->green + value * color2->green) / 100;
  result->blue  = ((100 - value) * color1->blue  + value * color2->blue)  / 100;
}
