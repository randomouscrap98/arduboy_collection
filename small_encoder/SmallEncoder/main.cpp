#define SMALLENCODER_STANDARDENV

#include "SmallEncoder.h"
#include "WriteHeaders.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

constexpr int16_t MAXBUFFERLENGTH = 1000;

//#define SHOWTEXT

#define ENCODEFUNC(t,tl,b,bl) encode_text_lz77(t,tl,b,bl)
#define DECODEFUNC(b,bl,t,tl) decode_partial_text_lz77(b,bl,t,tl,0)

void runtest(const char * text)
{
    uint32_t textlen = strlen(text);

    char outtext[MAXBUFFERLENGTH] = {0};
    uint8_t buffer[MAXBUFFERLENGTH] = {0};

    uint32_t encodedlen = ENCODEFUNC((uint8_t *)text, textlen, buffer, MAXBUFFERLENGTH);
    uint32_t decodedlen = DECODEFUNC(buffer, encodedlen, (uint8_t *)outtext, MAXBUFFERLENGTH);
    printf("Original text length: %d, encoded: %d, decoded: %d\n", textlen, encodedlen, decodedlen);

    if(strncmp(text, outtext, textlen)) {
        printf("Decoded string does not match encoded\n");
        exit(1);
    }

    #ifdef SHOWTEXT
    printf(" > %s\n", text);
    printf(" < %s\n", outtext);
    #endif
}

char megatext[] = "And so it has come to pass, that the age of magic is gone"
    "For now it is the age of man. And man is stupid. Yes that's right, men are stupid."
    "Stupidity has ruined magic! Wait... let's think about this, that doesn't make sense."
    "Stupidity is often the lack of reason, but usually it's reason which makes magic disappear."
    "Well that and greed. OK perhaps magic wasn't killed by stupidity but by greed, yes by man's "
    "insatiable greed! And so, without magic, the creatures of the world wither and decay into "
    "baser imitations of once great beings, and man dominates a banal world.";

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
    runtest(megatext);

    uint32_t textlen = strlen(lessertext);
    uint8_t buffer[MAXBUFFERLENGTH] = {0};
    int32_t length = encode_text_lz77((uint8_t *)lessertext, textlen, buffer, MAXBUFFERLENGTH);
    write_blob("somevar.h", "somevar", buffer, length);

    printf("Done\n");
}
