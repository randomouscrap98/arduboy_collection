#define SMALLENCODER_STANDARDENV

#include "SmallEncoder.h"
#include "WriteHeaders.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

constexpr int16_t MAXBUFFERLENGTH = 1000;

// #define SHOWTEXT

#define ENCODEFUNC(t,tl,b,bl) encode_text_lz77(t,tl,b,bl)
#define DECODEFUNC(b,bl,t,tl) decode_partial_text_lz77(b,bl,t,tl,0)

int total_saved = 0;
int total_bytes = 0;

void runtest(const char * text)
{
    uint32_t textlen = strlen(text);

    char outtext[MAXBUFFERLENGTH] = {0};
    uint8_t buffer[MAXBUFFERLENGTH] = {0};

    uint32_t encodedlen = ENCODEFUNC((uint8_t *)text, textlen, buffer, MAXBUFFERLENGTH);
    uint32_t decodedlen = DECODEFUNC(buffer, encodedlen, (uint8_t *)outtext, MAXBUFFERLENGTH);
    printf("Original text length: %d, encoded: %d, decoded: %d\n", textlen, encodedlen, decodedlen);

    total_saved += (textlen - encodedlen);
    total_bytes += textlen;

    #ifdef SHOWTEXT
    printf(" > %s\n", text);
    printf(" < %s\n", outtext);
    #endif

    if(strncmp(text, outtext, textlen)) {
        printf("Decoded string does not match encoded\n");
        exit(1);
    }
}

char megatext[] = "And so it has come to pass, that the age of magic is gone"
    "For now it is the age of man. And man is stupid. Yes that's right, men are stupid."
    "Stupidity has ruined magic! Wait... let's think about this, that doesn't make sense."
    "Stupidity is often the lack of reason, but usually it's reason which makes magic disappear."
    "Well that and greed. OK perhaps magic wasn't killed by stupidity but by greed, yes by man's "
    "insatiable greed! And so, without magic, the creatures of the world wither and decay into "
    "baser imitations of once great beings, and man dominates a banal world.";

char megatext2[] = "I needed to do things for every bit in a byte, but iterate over a large number of bits. "
    "Some badness here comes from the if statement, which must be executed every bit, the calculation of yStart "
    "& 7 (and a variable to hold it, registers are limited ofc) and the fastlshift8, which is a lookup table. "
    "Lookup tables are very fast but they still take time, I have to index into the array and read from program memory, "
    "which is a bit slow, maybe 7 cycles. This has significant benefits. It removes the need to check if bidx is 0, "
    "because that's just the start of the loop. So I can do it directly within the new loop. The new loop iterates 8 "
    "TIMES fewer, so that's a huge savings from not doing compares and increments. And finally, I don't have to do a "
    "lookup table anymore, because I just pass in the constant that came from the table (8 times).";

char lessertext[] = "Here is some text. I'm really hoping it has some small amount of compression. I'm doubtful.";

int main()
{
    //Try to encode some text.
    if(sizeof(char) != 1)
    {
        printf("Char is wrong size: %d\n", sizeof(char));
    }

    runtest("This sure is some text, yessir. It would be a shame if it were to compress poorly.");
    runtest("bacon bacon bacon bacon eggs eggs eggs eggs bacon bacon bacon bacon yeah");
    runtest("You have come to the right place. I will teach you");
    runtest("OK so let's try this with a lot of text\nYou see, there will often be a lot of text next to each other\nI'm hoping the dang system will give SOME level of compression\nEven though I know more complex compression schemes would produce significantly better results\nOh well");
    runtest("GRAB THE SWORD\nYOU WIN!\nENTER NAME:\nTHIS GAME SUCKS\nYOU HAVE FOUND THE SECRET AREA\nHUFFMAN ENCODING IS BETTER\nBUFFMAN ENCODING");
    runtest("ok so here's the deal. i need you to go find some mushrooms. they're red and whatever. you know, the smelly kind, like in zelda. you know zelda, right?");
    runtest(megatext);
    runtest(megatext2);

    printf("Total savings: %d / %d = %f\n", total_saved, total_bytes, (float)total_saved / total_bytes * 100);

    uint32_t textlen = strlen(lessertext);
    uint8_t buffer[MAXBUFFERLENGTH] = {0};
    int32_t length = encode_text_lz77((uint8_t *)lessertext, textlen, buffer, MAXBUFFERLENGTH);
    write_blob("somevar.h", "somevar", buffer, length);

    printf("Done\n");
}
