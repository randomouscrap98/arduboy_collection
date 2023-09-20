#pragma once

#include <stdint.h>
#include <algorithm>

//"ETX": end of text. Fitting I think. Why not 0? You can't read a compressed string as 
//a string anyway, so that convention doesn't matter
constexpr uint8_t DEFAULTDELIMITER = 0x03; //You CAN'T change this, it's part of the super simple encoder!
constexpr uint8_t WINDOWLENGTH = 32;    //You CAN'T change this; it's part of the super simple encoder
constexpr uint8_t MINMATCHLENGTH = 2;
constexpr uint8_t MAXMATCHLENGTH = 5;

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

//// Find the real index into a circular window buffer
//uint8_t windex(uint8_t x)
//{
//    return x & (WINDOWLENGTH - 1);
//}

// Encode text. You generally don't do this on device but figured might as well have it
int32_t encode_text(uint8_t * text, int32_t length, uint8_t * outbuf, int32_t buflength)
{
    uint32_t outlength = 0;
    int32_t tp = 0;

    //Main compression loop
    while(tp < length)
    {
        uint8_t findlen = 0;
        int32_t findpos = 0;

        //Scan window, which just means scan backwards in text
        for(int8_t wp = tp - 1; wp >= std::max<int32_t>(0, tp - WINDOWLENGTH); wp--)
        {
            uint8_t thisfindlen = 0;

            //Scan forward for the max match, break when nothing found
            for(uint8_t sp = wp; sp < tp; sp++)
            {
                if(text[tp + thisfindlen] != text[sp] || thisfindlen > MAXMATCHLENGTH) break;
                thisfindlen++;
            }

            //If we found a greater match, store the parameters
            if(thisfindlen > findlen)
            {
                findlen = thisfindlen;
                findpos = (tp - wp - 1) & 0b11111;
            }

            //No need to continue if we found the max match
            if(findlen == MAXMATCHLENGTH)
                break;
        }

        int32_t otp = tp;

        //We scanned the window, only store 'compressed value' if enough found
        if(findlen >= MINMATCHLENGTH)
        {
            //high bit for "encoded", 5 bits for scanback, 2 bits for len
            outbuf[outlength] = 0x80 | (findpos << 2) | (findlen - MINMATCHLENGTH);
            tp += findlen;
        }
        else
        {
            outbuf[outlength] = text[tp];
            tp++;
        }

        outlength++;

        if(outlength > buflength)
            return -1;
    }

    return outlength;
}

int32_t decode_text(uint8_t * compressed, int32_t length, uint8_t * outbuf, int32_t buflength)
{
    int32_t textlen = 0;

    for(uint32_t i = 0; i < length; i++)
    {
        uint8_t c = compressed[i];

        //This is an encoded byte.
        if(c & 0x80)
        {
            uint8_t len = MINMATCHLENGTH + (c & 0b11);
            uint8_t back = ((c >> 2) & 0b11111) + 1;

            for(uint8_t wp = 0; wp < len; wp++)
            {
                outbuf[textlen] = outbuf[textlen - back];
                if(++textlen == buflength && wp != len - 1) return -1;
            }
        }
        else
        {
            outbuf[textlen] = c;
            if(++textlen == buflength && i != length - 1) return -1;
        }
    }

    return textlen;
}