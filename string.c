#include "string.h"

#include <stdint.h>

void number_to_string(uint16_t number, char * string)
{
    if (number == 0)
    {
        string[0] = '0';
        string[1] = '\0';
    }
    else
    {
        char tmp[6] = {0};
        uint16_t i = 0, j = 0;

        while (number > 0)
        {
            tmp[i] = number % 10 + '0';
            number /= 10;
            i += 1;
        }

        while (i > 0)
        {
            string[j] = tmp[i - 1];
            j += 1;
            i -= 1;
        }

        string[j] = '\0';
    }
}

void byte_to_hex(uint8_t byte, char * string)
{
    const char hex[] = "0123456789ABCDEF";

    string[0] = hex[(byte >> 4) & 0x0F];
    string[1] = hex[byte & 0x0F];
    string[2] = '\0';
}
