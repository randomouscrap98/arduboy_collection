#pragma once

//"ETX": end of text. Fitting I think. Why not 0? You can't read a compressed string as 
//a string anyway, so that convention doesn't matter
constexpr uint8_t DEFAULTDELIMITER = 0x03; //You CAN'T change this, it's part of the super simple encoder!
constexpr uint8_t WINDOWLENGTH = 32;    //You CAN'T change this; it's part of the super simple encoder

//The encoding is meant for text, and works as such:
// - Initialize a sliding window of size 16
// 0 or 1 at highest bit to represent literal or not
// 7 bits for length, 8 bits for back fill? then window is 256
// 5 bits for back, 2 bits for length, length is 2 + length for up to 5

// Window must be significantly larger. Hm...
// hello, what are you doing here?
// i'm here to avenge my mother
// no you're not, go back to sleep
// no, you don't understand. i must avenge her

#define WINDOWINDEX(x) = ((x) & WINDOWLENGTH - 1)

// Encode text. You generally don't do this on device but figured might as well have it
uint32_t encode_text(uint8_t * text, uint32_t length, uint8_t * outbuf, uint32_t buflength)
{
    uint8_t window[WINDOWLENGTH] = {0};
    uint8_t curwin = 0;
    //uint8_t * curwin = window; //Circular buffer
    //uint8_t * curtext = text;
    uint8_t * curout = outbuf;
    uint32_t outlength = 0;


    //Main compression loop
    for(uint32_t tp = 0; tp < length; tp++)
    {
        uint8_t findlen = 0;
        uint8_t findpos = curwin;

        //Scan window
        for(uint8_t wp = 0; wp < WINDOWLENGTH; wp++)
        {
            uint8_t thisfindlen = 0;

            //Scan text chars for same 
            for(thisfindlen = 0; thisfindlen < 5; thisfindlen++)
            {
                if(text[tp + thisfindlen] != window[WINDOWINDEX(curwin + wp + thisfindlen)])
                {

                }
            }
        }
    }
}

