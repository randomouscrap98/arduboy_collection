#include <FixedPoints.h>
#include <Arduboy2.h>

Arduboy2 arduboy;

typedef UFixed<8,8> uflot;
typedef SFixed<7,8> flot;

void setup()
{
    arduboy.boot();
    arduboy.flashlight();
    arduboy.setFrameRate(30);
}

uint8_t current = 1;
//char buff[20];

void printBits(uint16_t v)
{
    for(uint8_t i = 0; i < 16; i++)
        arduboy.print(v & (1 << (15 - i))?1:0);
}

void loop()
{
    if(!arduboy.nextFrame()) return;

    arduboy.pollButtons();

    int mod = 0;
    if(arduboy.justPressed(A_BUTTON) || arduboy.pressed(UP_BUTTON))
        mod = 1;
    if(arduboy.pressed(DOWN_BUTTON))
        mod = -1;
        
    current = (current + 256 + mod) % 256;

    arduboy.clear();
    flot truef = flot::fromInternal(current);
    flot rtruef = 1.0 / truef;
    arduboy.print((float)truef, 4);
    arduboy.print(" -> ");
    arduboy.print((float)rtruef, 4);
    arduboy.println();
    arduboy.print(current);
    arduboy.print(" -> ");
    arduboy.print((uint16_t)rtruef.getInternal());
    arduboy.println();
    arduboy.println();
    printBits(current);
    arduboy.println();
    arduboy.println();
    printBits(rtruef.getInternal());

    //sprintf(buff, "%d: %f -> %f", current, (float)truef, (float)1 / truef);
    //arduboy.print(buff);
    arduboy.display();
}
