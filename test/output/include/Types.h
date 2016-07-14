#ifndef __WORDS_INT_TYPES_H__
#define __WORDS_INT_TYPES_H__

#include <sys/types.h>
#include <string>
typedef unsigned char uint8_t;
typedef signed char int8_t;

typedef unsigned short uint16_t;
typedef signed short int16_t;

typedef unsigned int uint32_t;
typedef signed int int32_t;

//typedef unsigned int size_t;

namespace NLP
{
    struct MatchInfo
    {
        int        pos; // the end position of the matched word
        int32_t state; // wordId of the matched word
    };

    union ValueType
    {
        int vInt;
        float vFloat;
        char* vChar;
    };

    struct MatchRes
    {
        int pos;
        std::string key;
        ValueType value;
    };
}//namespace

#endif
