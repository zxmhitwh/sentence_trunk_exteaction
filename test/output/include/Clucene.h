#ifndef __LUCENE_H__
#define __LUCENE_H__
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <hash_map>
#include <stdlib.h>
#include <math.h>
#include "UtilsLib.h"  
//#include "MySeg.h"
#include "mt_seg.h"
#include "Word2Vec.h"
#include "MinEditDistance.h"
#include "ul_dictmatch.h"
#include "StopWord.h"
#include "PinyinSim.h"
#include "boost/algorithm/string.hpp"

using namespace std;

#define MAX_LINE_LEN 1024000 
#define TOTAL_HITS 5

const int CONTENT_ERROR = 0;  //输入内容有错
const int SYSTEM_ERROR = 1;   //系统错误
const int SENSITIVE_WORD = 2;  //有敏感词
const int COMPLETY_QUES = 3 ;  //补问
const int ANSWER_MORE = 4 ; //答案多选
const int CAN_ANSWER = 5;    //可以回答利用模板
const int RECOMMAND_ANSWER = 6;    //可以回答并且有推荐
const int NO_ANSWER = 7 ;   //没有答案
const int VISITE_MORE = 8 ;  //访问过多
const int TREE_PATTERN = 9;  //导航或者树形

struct KefuResult;

class Clucene
{
public:
    static bool Init(const char* Filename,const char* SimWordFile,const char *,const char *,map<string,float>&,dm_dict_t*,vector<string>&,map<string,string>&);
    static void UnInit();
    static string get_standard_ques(string &q);
    static string get_question_map_answer(string &q);
    static int  FAQMatchWordSense(string& questionType,vector<WORD_POS>& query_seg, string& answer, float &score,vector<pair<string,float> >& result,string &,string &,string &,map<string,float> &);
    static int  FAQMatch(string& questionType,vector<WORD_POS>& query_seg, string& answer, float &score,vector<pair<string,float> >& result,string &,string &,string &,map<string,float> &);
    static bool replace_synword(string& str,dm_pack_t*);
    static bool replace_synword(vector<WORD_POS>& ent_seg);
    static void FAQMatch(string& sClass,string& sClassCls,
            vector<WORD_POS>& vsTokens, string& answer, vector<string> &result);
    static void FAQMatch(string& sClass,string& sClassCls,
            vector<WORD_POS>& vsTokens, string& answer, vector<KefuResult> &result);
    static int  CheckIsInCorpus(string& query,string& type,string& answer,string &,bool isTreeInput = false);
    
    static int RunKNN(int iClsNum);

    static bool UnEqualNum(int num);
    static int get_corpus_lines();
    
    static bool UpdateCorpus(vector<vector<string> >& vsCorpus,vector<vector<string> >& vsTreeCorpus);
    static bool find_standar_question(pair<string,string> & p,string & standarques,string & source);
    static bool update_local_corpus_file(vector<vector<string> >& vsCorpus,vector<vector<string> >& vsTreeCorpus);
};

#endif

