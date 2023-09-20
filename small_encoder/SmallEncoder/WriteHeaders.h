#pragma once

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string.h>

#ifndef BYTESPERLINE
#define BYTESPERLINE 16
#endif

void write_blob(const char * filename, const char * varname, uint8_t * blob, int32_t length)
{
    std::ofstream myfile;
    myfile.open (filename);
    myfile << "#pragma once\n\n";
    myfile << "constexpr uint32_t " << varname << "_length = " << length << ";\n\n";
    myfile << "constexpr uint8_t " << varname << "[] PROGMEM = {\n";
    myfile << "  ";
    for(int32_t i = 0; i < length; i++)
    {
        myfile << "0x" << std::hex << (int)blob[i] << std::dec;
        if(i != length - 1)
        {
            myfile << ", ";
            if((i % BYTESPERLINE) == BYTESPERLINE - 1)
                myfile << "\n  ";
        }
        else
        {
            myfile << "\n";
        }
    }
    myfile << "};\n";
    myfile.close();
}
