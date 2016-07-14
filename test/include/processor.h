#ifndef _PROCESSOR_H__
#define _PROCESSOR_H__
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include "config.h"
#include "UtilsLib.h"
#include "TCSegFunc.h"
#include "parser_dll.h"
#include "ul_log.h"
#include "code_conversion.h"
#include "ul_ccode.h"
#include "pcre.h"
#include "ul_dictmatch.h"
#include <property.h>

namespace senming
{
using namespace std;

#define SENMING_LOG_DEBUG(message, ...) ul_writelog(UL_LOG_DEBUG, "[%s]: "message, __FUNCTION__, ##__VA_ARGS__)
#define SENMING_LOG_TRACE(message, ...) ul_writelog(UL_LOG_TRACE, "[%s]: "message, __FUNCTION__, ##__VA_ARGS__)
#define SENMING_LOG_WARNING(message, ...) ul_writelog(UL_LOG_WARNING, "[%s]: "message, __FUNCTION__, ##__VA_ARGS__)
#define SENMING_LOG_ERROR(message, ...) ul_writelog(UL_LOG_FATAL, "[%s]: "message, __FUNCTION__, ##__VA_ARGS__)

const int MAX_TERM_LEN = 1000;
const int MAX_SENT_LEN = 1024;
const int MAX_COMM_LEN = 4096;
const int SEG_MAX_TERM = 1000;
const int SPLIT_SEG_WORD = 0;
const int SPLIT_FILT_WORD = 1;
const int DEF_DM_DICT_SIZE = 10000;
const int DEF_DM_PACK_SIZE = 1000;
const int CLS_TYPE_OFF = 0;             // ²»·ÖÀà
const int CLS_TYPE_UNSUPERVISE = 1;     // ÎÞ¼à¶½
const int CLS_TYPE_SUPERVISE = 2;       // ÓÐ¼à¶½

// PAIRÇé¸ÐÇãÏò
const int POSITIVE_PAIR = 2;
const int NEUTRAL_PAIR = 1;
const int NEGATIVE_PAIR = 0;


const int ADV_NEG_WORD = 1;
const int ADV_DEG_WORD = 2;

const int TEND_POS_WORD = 1;
const int TEND_NEG_WORD = 0;



// 词项结构体
typedef struct term_t
{
    std::string text;       // 词项字符串
    size_t offset;          // 偏移量
    std::string postag;     // 词性标注
    bool is_chunk;          // 是否主干词
    term_t()
    {
        is_chunk = true;
    }
}term_t;

// 分句结构体
typedef struct
{
    std::string text;
    // 分句字符串
    std::vector<term_t> term_basic;   // basic粒度切次结果
    std::vector<term_t> term_pre;   // 切词、词性标注结果(修正前)
    std::vector<term_t> term_pro;   // 切词、词性标注结果(修正后)
    std::set<std::string> phrase;   // 切词大粒度词组

    // 句法分析深度优先路径及节点关系
    std::map<std::pair<int, int>, std::string> parse_rel;
    std::set<std::string> pair; 
    std::vector<std::vector<int> > parse_path;

    //LTP dep parser结果接口
    vector<int> heads;
    vector<string> deprels;

}sentence_t;

// 评论结构体
typedef struct
{
    std::string text;                   // 一个未分句的评论字符串
    int label;                          // 评论的正负倾向(有监督数据)
    std::vector<sentence_t> sentence;   // 每个分句
}comment_t;

typedef struct
{
    std::string prop;       // ÊôÐÔ´Ê
    std::string comm;       // ÆÀ¼Û´Ê
    int tend;               // Çé¸ÐÇãÏò (0:¸ºÏò/1:ÎÞ·¨ÅÐ¶Ï/2:ÕýÏò)
    int freq;               // ÆÀÂÛÖÐ³öÏÖÆµ´Î
    int p_freq;             // ÕýÏòÆÀÂÛÖÐ³öÏÖÆµ´Î
    int n_freq;             // ¸ºÏòÆÀÂÛÖÐ³öÏÖÆµ´Î
    double p_pmi;           // ÕýÏòpmi
    double n_pmi;           // ¸ºÏòpmi
    double score;
}pair_t;

int str_2_termvec(const string &str, vector<term_t> &term_vec, char split_char = ' ');
int termvec_2_str(const vector<term_t> &term_vec, string &str, char split_char = ' ');

class processor
{
private:
    static bool bProcessHasInit;
    static int use_wordseg;
    static int use_parser;
    static void* engine;
    static vector<pair<string, int> > _comb_rule_vec;
    static map<string, string> _pos_rule_map;
    static map<string, string> _postag_map;
    old_seg::HANDLE seghandle;
    bool match_rule(const vector<term_t> &term_vec, const vector<term_t> &rule_vec, int spos);

    string getPostagMappingResult(const string postag);
    static int read_postag_rules(const string file);
    static dm_dict_t* load_dm_dict(const char *, int size, int set_prop);
    bool Utf8_2_GB(string &);
    static int read_combine_rules(const string file);
    static int read_postag_mapping(const string file);
    static dm_dict_t* _dm_dict ;
    dm_pack_t *_dm_pack;
    CodeConversionTool codetool;

    int postag(sentence_t &sentence);
    int correctPostag(term_t &aterm);
    int correctParse(sentence_t &sentence);
    int parse(sentence_t &sentence);

public:

    static bool Init(config& cfg);
    static void UnInit();
    processor();
    ~processor();

    int process(comment_t &comment,  bool isSplit = true); 

};

}

#endif
