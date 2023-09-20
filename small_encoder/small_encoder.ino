#include <Arduboy2.h>

#include "SmallEncoder/SmallEncoder.h"
#include "SmallEncoder/somevar.h"

Arduboy2 arduboy;
uint8_t temp;

void setup()
{
    arduboy.begin();
    arduboy.setFrameRate(30);
    temp = pgm_read_byte(somevar);
}

void loop()
{
    if(!arduboy.nextFrame()) return;

    arduboy.clear();

    char text[150];
    decode_text_lz77((uint8_t *)somevar, somevar_length, (uint8_t *)text, 150);
    arduboy.print(strlen(text));

    arduboy.display();
}
