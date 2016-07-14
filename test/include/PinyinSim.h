
#ifndef _PINYIN_SIM_H
#define _PINYIN_SIM_H

#include <string>

class PinyinSimDict
{
public:
    static bool  Load(const char* dictPath);
    static void  Release();

    float QuerySim(std::string& query1, std::string& query2);
    float QueryPinyinMED(std::string& query1, std::string& query2);
    bool Pinyin(std::string& query,std::string& py);
    float PinyinSim(std::string& py1, std::string& py2);   
};

#endif

