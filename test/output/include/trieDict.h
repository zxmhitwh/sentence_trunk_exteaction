//======================================================================================
// Name         : TrieDict.h
// Author       : niuruiyao@meituan.com
// Organization : tne.nlpml@meituan.com
// Version      : v1.0
// Description  : class TrieDict gives an interface to build(add), write and load a tire.
                    //When a trie is built, it allows the user to judge if a key is in
                    //this trie and get value of the key.
//======================================================================================

#ifndef __TRIE_DICT_H__
#define __TRIE_DICT_H__

#include <stdio.h>
#include <sys/types.h>

#include "Types.h"

namespace NLP
{
    struct Header;
    class TrieBase;
    class MMap;
    class TrieDict
    {
        public:
            TrieDict();
            ~TrieDict();

            //load the tire from a file or mem
            int loadDict(const char* fileName);
            int loadDict(FILE* fp);
            int loadDict(const char* mem, uint32_t len);

            /**
             * Match a key in the trie
             * @param szKey: input characters
             * @param keyLen: characters length
             * @return 0 if the key is not exist, else return wordId
             */  
            int32_t match(const char* szKey, uint32_t keyLen);
            
            /**
             * Match all words in the trie
             * @param str: input characters
             * @param len: characters length
             * @param matches: information about all matched words
             * @param matchMaxLen: the maximum number of matched words
             * @return the number of matched words
             */ 
            int32_t matchAll(const char* str, uint32_t len, MatchInfo * matches, uint32_t matchMaxLen);
            int32_t matchAll(const char* str, uint32_t len, MatchRes * matches, uint32_t matchMaxLen);

            /**
             * Match longest words in the trie
             * @param szKey: input characters
             * @param keyLen: characters length
             * @param matchedLen: length of the longest matched word
             * @return 0 if no matched words exist in the trie, else return wordId
             */
            int32_t matchLongest(const uint8_t* szKey, int keyLen, int& matchedLen);

            bool ifLoaded() const { return _bIfLoaded; };
            int32_t getValueType() const;
            const char *getTrieErr() const;

            /**
             * get value of szKey from a trie
             * @param szKey: input characters
             * @param keyLen: characters length
             * @param pKeyVal: the value pointer. User should convent the type to what they wanted.
             * @return wordId, wordId = 0 means the key is not in this trie
             */
            int32_t getValue(int32_t wordId, uint8_t **pKeyVal);

            /**
             * build a trie
             * @param szKey: input characters
             * @param keyLen: characters length
             * @param val: the value of szKey
             * @return 0 if there is an error
             */
            int32_t add(const char* szKey, uint32_t keyLen, const char* val = NULL);

            //init the trie depends on trie_type and value_type
            int initTrie(unsigned int trie_type = 1, unsigned int value_type = 0);

            //write the tire into a file
            int writeDict(const char* fileName);
            int writeDict(FILE* fp);
            void clearData();

        private:
            Header * _header;// the head to record some information about the trie
            uint8_t *_pData;// pointer to the value. If the value_type is 3(char*), it pointer to the value position in _pString.
            uint8_t *_pString;// pointer to the value string
            TrieBase *_ptrie;// base pointer to a trie. If trie_type is 0, it points to DoubleArrayTrie;
            MMap *_dictMap;
            //if trie_tyoe is 1, it points to DoubleTrie.
            bool _bDataNeedFree;
            bool _bDataStringNeedFree;
            bool _bReadOnly;
            bool _bIfLoaded;
    };
}//namespace

#endif // __TRIE_DICT_H__
