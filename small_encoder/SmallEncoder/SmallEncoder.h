#pragma once

#include <stdint.h>

#ifdef SMALLENCODER_STANDARDENV
#include <algorithm>
#define SMALLENCODER_DIRECTREAD
#define max(a,b) std::max(a,b)
#else
#include <Arduboy2.h>
#endif

//"ETX": end of text. Fitting I think. Why not 0? You can't read a compressed string as 
//a string anyway, so that convention doesn't matter
constexpr uint8_t SMALLENCODER_DEFAULTDELIMITER = 0x03; //You CAN'T change this, it's part of the super simple encoder!

// ------------- LZ77 (SORT OF) ----------------

constexpr uint8_t LZ77WINDOWLENGTH = 64;    //You CAN'T change this; it's part of the super simple encoder
constexpr uint8_t LZ77WINDOWMASK = LZ77WINDOWLENGTH - 1;
constexpr uint8_t LZ77MATCHBITS = 1;
constexpr uint8_t LZ77MATCHMASK = (1 << LZ77MATCHBITS) - 1;
constexpr uint8_t LZ77MINMATCHLENGTH = 2;
constexpr uint8_t LZ77MAXMATCHLENGTH = LZ77MATCHMASK + LZ77MINMATCHLENGTH;

// Encode text. You generally don't do this on device but figured might as well have it
int32_t encode_text_lz77(uint8_t * text, int32_t length, uint8_t * outbuf, int32_t buflength)
{
    uint32_t outlength = 0;
    int32_t tp = 0;

    //Main compression loop
    while(tp < length)
    {
        uint8_t findlen = 0;
        int32_t findpos = 0;

        //Scan window, which just means scan backwards in text. wp is direct index into 'window' (text)
        for(int8_t wp = tp - 1; wp >= max(0, tp - LZ77WINDOWLENGTH); wp--)
        {
            uint8_t thisfindlen = 0;

            //Scan forward for the max match, break when nothing found. 
            for(thisfindlen; thisfindlen < LZ77MAXMATCHLENGTH; thisfindlen++)
                if(wp + thisfindlen >= length || text[wp + thisfindlen] != text[tp + thisfindlen] ||
                   text[wp + thisfindlen] == SMALLENCODER_DEFAULTDELIMITER) 
                    break;

            //If we found a greater match, store the parameters
            if(thisfindlen > findlen)
            {
                findlen = thisfindlen;
                findpos = (tp - wp - 1) & LZ77WINDOWMASK;
            }

            //No need to continue if we found the max match
            if(findlen == LZ77MAXMATCHLENGTH)
                break;
        }

        //We scanned the window, only store 'compressed value' if enough found
        if(findlen >= LZ77MINMATCHLENGTH)
        {
            //high bit for "encoded", 5 bits for scanback, 2 bits for len
            outbuf[outlength] = 0x80 | (findpos << LZ77MATCHBITS) | (findlen - LZ77MINMATCHLENGTH);
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

int32_t decode_text_lz77(uint8_t * compressed, int32_t length, uint8_t * outbuf, int32_t buflength)
{
    int32_t textlen = 0;

    for(uint32_t i = 0; i < length; i++)
    {
        uint8_t c;
        #ifdef SMALLENCODER_DIRECTREAD
        c = compressed[i];
        #else
        c = pgm_read_byte(compressed + i);
        #endif

        //This is an encoded byte.
        if(c & 0x80)
        {
            uint8_t len = LZ77MINMATCHLENGTH + (c & LZ77MATCHMASK);
            uint8_t back = ((c >> LZ77MATCHBITS) & LZ77WINDOWMASK) + 1;

            for(len; len > 0; len--)
            {
                outbuf[textlen] = outbuf[textlen - back];
                if(++textlen == buflength && len != 1) return -1;
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

int16_t decode_partial_text_lz77(uint8_t * compressed, uint16_t length, uint8_t * outbuf, int16_t buflength, int16_t pos)
{
    uint8_t window[LZ77WINDOWLENGTH];
    uint16_t wp = 0;
    int16_t textlen = 0;

    for(uint16_t i = 0; i < length; i++)
    {
        uint8_t c;
        #ifdef SMALLENCODER_DIRECTREAD
        c = compressed[i];
        #else
        c = pgm_read_byte(compressed + i);
        #endif

        uint16_t owp = wp;

        //This is an encoded byte.
        if(c & 0x80)
        {
            uint8_t len = LZ77MINMATCHLENGTH + (c & LZ77MATCHMASK);
            uint8_t back = ((c >> LZ77MATCHBITS) & LZ77WINDOWMASK) + 1;

            for(len; len > 0; len--)
            {
                window[wp & LZ77WINDOWMASK] = window[(wp - back) & LZ77WINDOWMASK];
                wp++;
            }
        }
        else
        {
            window[wp & LZ77WINDOWMASK] = c;
            wp++;
            if(c == SMALLENCODER_DEFAULTDELIMITER) pos--;
        }

        if(pos == 0)
        {
            for(owp; owp < wp; owp++)
            {
                if(textlen >= buflength) return -1;
                outbuf[textlen++] = window[owp & LZ77WINDOWMASK];
            }
        }
        else if (pos < 0)
        {
            break;
        }
    }

    return textlen;
}


// ------------- HUFFMAN (MOSTLY) ----------------



//int32_t encode_text_huffman(uint8_t * text, int32_t length, uint8_t * outbuf, int32_t buflength)
//{
//
//}
//
//int32_t decode_text_huffman(uint8_t * compressed, int32_t length, uint8_t * outbuf, int32_t buflength)
//{
//
//}