#include "sound.hpp"

#include "mymath.hpp"
#include <ArduboyTones.h>

ArduboyTones sound(arduboy.audio.enabled);

void menu_move_beep() { sound.tone(4000, 10); }
void menu_select_beep() { sound.tone(2000, 15); }
void confirm_beep() { sound.tone(2000, 30, 3000, 30, 4000, 40); }
void descend_beep() { sound.tone(60, 50, 57, 50, 54, 130); }
void item_open_beep() { sound.tone(1000, 30, 1500, 30); }
void item_close_beep() { sound.tone(1500, 30, 1000, 30); }
void item_pickup_beep() { sound.tone(500, 30, 1000, 30); }
void step_beep() { sound.tone(40 + RANDB(10), 30); }
