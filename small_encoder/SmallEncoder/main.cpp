//#include "SmallEncoder.h"
#include <stdio.h>

int main()
{
    //Try to encode some text.
    if(sizeof(char) != 1)
    {
        printf("Char is wrong size: %d\n", sizeof(char));
    }

    printf("Done\n");
}