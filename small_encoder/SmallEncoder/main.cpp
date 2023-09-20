#include "SmallEncoder.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

constexpr int MAXBUFFERLENGTH = 1000;

#define SHOWTEXT

void runtest(const char * text)
{
    uint32_t textlen = strlen(text);

    char outtext[MAXBUFFERLENGTH] = {0};
    uint8_t buffer[MAXBUFFERLENGTH] = {0};

    uint32_t encodedlen = encode_text((uint8_t *)text, textlen, buffer, MAXBUFFERLENGTH);
    uint32_t decodedlen = decode_text(buffer, encodedlen, (uint8_t *)outtext, MAXBUFFERLENGTH);
    printf("Original text length: %d, encoded: %d, decoded: %d\n", textlen, encodedlen, decodedlen);
    //printf("Original text length: %d, encoded: %d\n", textlen, encodedlen);

    #ifdef SHOWTEXT
    printf(" > %s\n", text);
    printf(" < %s\n", outtext);
    #endif
}

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

    printf("Done\n");
}
