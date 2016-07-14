#ifndef __KEFU_H__
#define __KEFU_H__
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <hash_map>
#include <stdlib.h>
#include <math.h>
//#include "MySeg.h"
#include "Word2Vec.h"
#include "mt_seg.h"
#include "StopWord.h"
#include "bayes.h"
#include "pcre.h"
#include "Clucene.h"
#include "ul_dictmatch.h"
#include "config.h" 
#include "senming_splitter.h"
#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"
//#include "boost/log/core.hpp"
//#include "boost/log/trivial.hpp"  
//#include "boost/log/expressions.hpp"
//#include "boost/log/utility/setup/file.hpp"



using namespace std;

#define MAX_COMMENT_LENGTH 200
#define MIN_COMMENT_LENGTH 15

#define PRODUCT_CLS_ID "2"
#define SERVER_CLS_ID "1"
#define OTHER_CLS_ID "3"

typedef struct KefuResult {
    string type;
    string question_cate;
    string sim_question;
    string answer;
} KefuResult;

class Kefu
{
public:

    static bool Init(const char* Filename);
    static void UnInit();
    Kefu();
    ~Kefu();
    void  Consult(string& query,string& type, string& answer, vector<string> &result);
    //返回值， 0 请求内容有错或者格式有误
    //1 系统错误
    //2 有敏感词
    //3 无法回答
    //4 需要补问
    //5 答案多选
    //6 能够回答
    //7 没有答案
    //8 模式匹配
    //10 访问攻击
    //11 聊天过频
    //12 导航知识/模式
    int Consult(string &query,float& score,string &answer,string& standarAnswer,string&,string&,string &,vector<pair<string,float> >&recommand); 
    void Consult(string& query,vector<KefuResult>& results);

private:
    int getFocusSentence(senming::comment_t&,string &);
    static bool initBinaryClassfiyDict(string&,string&,string&);
    static bool init_keyword_dict(string&);
    static bool init_sensitive_dict(string &path);
    static bool init_synword_dict(string &syn_word_path);
    //存在敏感词，返回true，并用****repace
    //否则返回false
    bool replace_sensitive_word(string& query,string replace_str ="***");
    bool replace_synword(string&);
    int getBinaryClassify(string &,vector<WORD_POS>&);
    static bool bKefuHasInit;   
    static set<string> punctuationSet;
    static pcre* pDoubtPattern;
    static set<string> endSet;
    static map<string,float> keyword_dict;
    //过滤敏感词字典
    static dm_dict_t* sensitive_dict;
    static vector<string> synword_vec;
    //同义词词典
    static dm_dict_t* synword_dict;
    static map<string,string> synword_dict_map;
    dm_pack_t* m_resPack;
    CodeConversionTool codeTool;

    myseg seg;

    splitter sentence_splitter;
};

#endif

