#include <Wire.h>
#include <SimbleeForMobile.h>

uint8_t m_led[5];
uint8_t v_led[5];

uint8_t s_230_1, s_230_2;

char *modes[] = { "Off", "On", "Disco", "Flash", NULL };

void setup()
{
  Wire.begin();
  
  SimbleeForMobile.deviceName = "ledC";
  SimbleeForMobile.advertisementData = "LED control";
  SimbleeForMobile.begin();
}

void loop()
{
  SimbleeForMobile.process();
}

void ui()
{
  SimbleeForMobile.beginScreen(rgb(85,85,85));

  const int y_inc = 100;
  
  int y_offset = 10;
  for(uint8_t i=0; i<5; i++)
  {
    char buf[10];
    sprintf(buf, "LED %u:", i+1);
    SimbleeForMobile.drawText(10, y_offset, buf, BLACK, 25);
    m_led[i] = SimbleeForMobile.drawSegment(80, y_offset, SimbleeForMobile.screenWidth - (10+80), modes, 4);
    v_led[i] = SimbleeForMobile.drawSlider(0, y_offset + 50, SimbleeForMobile.screenWidth, 0, 255);
    y_offset += y_inc;
  }

  int x_offset = 25;
  SimbleeForMobile.drawText(x_offset, y_offset, "Strahler:", BLACK, 25);
  s_230_1 = SimbleeForMobile.drawSwitch(x_offset + 90, y_offset);

  x_offset += 150;
  SimbleeForMobile.drawText(x_offset, y_offset, "Motor:", BLACK, 25);
  s_230_2 = SimbleeForMobile.drawSwitch(x_offset + 70, y_offset);
  
  SimbleeForMobile.endScreen();
}

void ui_event(event_t &event)
{
  Wire.beginTransmission(69);

  // modes
  for(uint8_t i=0; i<5; i++)
  {
    if(event.id != m_led[i])
      continue;
    Wire.write(i);Wire.write('m');Wire.write((uint8_t) event.value);
    Wire.endTransmission();
    return;
  }

  // values
  for(uint8_t i=0; i<5; i++)
  {
    if(event.id != v_led[i])
      continue;
    Wire.write(i);Wire.write('v');Wire.write((uint8_t) event.value);
    Wire.endTransmission();
    return;
  }

  // switch 1
  if(event.id == s_230_1)
  {
    Wire.write(5); Wire.write((uint8_t) event.value);
  }
  // switch 2
  else if(event.id == s_230_2)
  {
    Wire.write(6); Wire.write((uint8_t) event.value);
  }
  Wire.endTransmission();
}
