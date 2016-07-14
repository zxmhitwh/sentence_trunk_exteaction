#ifndef __HEADER_H__
#define __HEADER_H__

#include "Types.h"
namespace NLP
{
    struct Header
    {
        Header() : data_max(0), data_used(1),
        longest_word_len(0), trie_type(0), value_type(0), string_len(0), string_len_max(0){};
        void clean()
        {
            data_max = 0;
            data_used = 1;
            longest_word_len = 0;
            trie_type = 0;
            value_type = 0;
            string_len = 0;
            string_len_max = 0;
        };
        Header& operator=(const Header &head)
        {
            data_max = head.data_max;
            data_used = head.data_used;
            longest_word_len = head.longest_word_len;
            trie_type = head.trie_type;
            value_type = head.value_type;
            string_len = head.string_len;
            string_len_max = head.string_len_max;
            return *this;
        };
        int32_t    data_max;  // max number of data it can hold
        int32_t    data_used; // number of data_used
        int32_t    longest_word_len; // length of the longest word
        int32_t trie_type;// 0 means a DoubleArrayTrie, 1 means a DoubleTrie
        int32_t value_type;// 0 means no value used, 1 means the value is int32_t, 2 means float, 3 means char*
        int32_t string_len;// if the value_type is 3(char*), it means the sum of string_len for every key (include '\0').
        int32_t string_len_max;// length of _pString
    };
}//namespace
#endif //__HEADER_H__
