#ifndef  __PATTERN_PARSER_CORE_H_
#define  __PATTERN_PARSER_CORE_H_

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <iostream>
#include "trieDict.h"
#include "pattern_parser.h"

namespace NLP {


#ifndef PAT_FWRITE
#define PAT_FWRITE( DATA_PTR, ITERM_SIZE, ITERM_NUM, FSTREAM ) \
    if ( fwrite( DATA_PTR, ITERM_SIZE, ITERM_NUM, FSTREAM ) == 0 ) { \
        FATAL_LOG( "fwrite error\n" ); \
        if ( FSTREAM != NULL ) { \
            fclose( FSTREAM ); \
        } \
        return -1; \
    };
#endif

#ifndef PAT_FREAD
#define PAT_FREAD( BUFF_PTR, ITERM_SIZE, ITERM_NUM, FSTREAM ) \
    if ( fread( BUFF_PTR, ITERM_SIZE, ITERM_NUM, FSTREAM ) == 0 ) { \
        FATAL_LOG( "fread error\n" ); \
        if ( FSTREAM != NULL ) { \
            fclose( FSTREAM ); \
        } \
        return -1; \
    };
#endif

//最多可注册函数个数
const int PAT_MAX_FUN_NUM = 20;
//最多可以使用的通配符个数
const int PAT_MAX_WILDCARD_NUM = 20;

//连续匹配忽略词的最大次数，即停留在同一个状态的最大次数
const int MAX_SAMESTATE_NUM = 8;
//默认的最大的搜索状态数
const int MAX_SEARCH_STATES = 2000;

//可忽略词的属性 formid 不可作为下标
const int IGN_WORD_PROP = -3;
//当前节点上存在函数
const int NODE_HAVE_FUNC = -2;
//当前节点上有通配符
const int NODE_HAVE_WILDCARD = -3;
//当前节点上既有通配符，又有函数
const int NODE_HAVE_TWO = -4;

//递归搜索时的当前状态
typedef struct search_statue{
    const char* query;          // 搜索当前query
    int query_len;              // query长度
    int strpos;                 // 递归当前位置
    int current;                // 当前节点位置
    int res_size;               // 结果大小
    int res_num;                // 实际结果数
    int term_num;               // 匹配词个数:函数、模板词、固定词、停用词、通配符
    int formid_res[RES_TERMS_SIZE];// 模板匹配的顺序
    const char* word_extra[RES_TERMS_SIZE]; // 模板词典的属性字段指针
    int pos[RES_TERMS_SIZE];      // 匹配词位置
    int len[RES_TERMS_SIZE];      // 匹配词长度
    int beforestate;              // 记录当前状态的之前状态
    int cntsamestate;             // 记录连续状态相同的数目
    int cntstate;                 // 记录搜索的状态数
    bool isbeyond;                // 记录是否超过状态阈值
}pat_statue;


typedef struct tmp_type{
    int type;
    char proper[PAT_MAX_WORDS_LEN];
}type_t;

class pattern_parser_core
{
public:

    /**
     * @brief : 保存词典到文件
     * @param file [in] : 文件名
     * @return : 成功返回当前0，失败返回-1
     **/
    int savefile(const char* file);

    /**
     * @brief : 从二进制文件读取词典
     * @param file [in] : 文件名
     * @return : 成功返回当前0，失败返回-1
     **/
    int loadfile(const char* file);

    /**
     * @brief : 从明文文件读取词典
     * @param pat_fname [in] : 模版文件名
     * @param term_fname [in] : term文件名
     * @param igno_fname [in] : 忽略词文件名
     * @return : 成功返回当前0，失败返回-1
     **/
    int loadfile(const char* pat_fname,const char* term_fname,const char* igno_fname);

    /**
     * @brief : 在模板树中查找当前query，并返回结果
     * @param [in/out] now_statue : 递归搜索时当前状态
     * @param [out] result : 记录结果的数据结构
     **/
    void search(pat_statue* now_statue,pat_res_t* result);

    /**
     * @brief : 注册函数，要求格式为:类似[F:name]
     * @param name [in] : 待注册函数名,模板文件中出现的格式 如 [F:num]
     * @param fptr [in] : 函数指针，参数char*，返回值int
     * @return : 成功返回当前0，失败返回-1
     **/
    int register_func(const char *name,int(*fptr)(const char*));        //注册函数

    /**
     * @brief : 注册通配符，要求格式为类似[W:0-5]
     * @param name [in] : 待注册通配符名
     * @return : 成功返回当前0，失败返回-1
     **/
    int register_wildcard(const char *name);

    /**
     * @brief 构造函数
     **/
    pattern_parser_core();

    /**
     * @brief 析构函数
     **/
    ~pattern_parser_core();

private:

    /**
     * @brief  : 对象数据结构初始化
     * @return : 成功返回0，失败返回-1
     **/
    int init();

    /**
     * @brief 销毁所有数据结构
     **/
    void destroy();

    /**
     * @brief 输入term词并保存进数据结构
     * @param [in] filename : 输入term文件
     * @param [out] word_record : 用来记录出现的词
     * @return 正常返回0，异常返回-1
     **/
    int inputWord(const char* filename, std::multimap<std::string,type_t>& word_record);

    /**
     * @brief 输入停用词并保存进数据结构
     * @param [in] filename : 输入停用文件
     * @param [out] word_record : 用来记录出现的词
     * @return 正常返回0，异常返回-1
     **/
    int inputIgn(const char* filename, std::multimap<std::string,type_t>& word_record);

    /**
     * @brief 输入模板并保存进数据结构
     * @param [in] filename : 输入模板文件
     * @param [out] word_record : 用来记录出现的词
     * @return 正常返回0，异常返回-1
     **/
    int inputForm(const char* filename, std::multimap<std::string,type_t>& word_record);

    /**
     * @brief  : 追加分配_nodelist内存，两倍
     * @return : 成功返回当前_nodelist长度，失败返回-1
     **/
    int reallocNodeList();

    /**
     * @brief : 追加分配_hashlist内存，两倍
     * @return : 成功返回当前_hashlist长度，失败返回-1
     **/
    int reallocHashList();

    /**
     * @brief : 追加分配_wordlist内存，两倍
     * @return : 成功返回当前_wordlist长度，失败返回-1
     **/
    int reallocFormList();

    /**
     * @brief : 获得formid在hash中当前段的位置，即下标
     * @param [in] segpos : hash当前段首位置
     * @param [in] seglen : hash当前段长度
     * @param [in] formid : formid，即_formlist下标
     * @return : 返回_hashlist下标
     **/
    int getpos(int segpos,int seglen,int formid);

    /**
     * @brief 移动一个hash段，并在newseg这个段中插入新字
     * @param [in] oldseg : 旧段在hashlist中的起始位置
     * @param [in] oldlen : 旧段长度
     * @param [in] newseg : 新段起始位置
     * @param [in] newlen : 新段长度
     * @param [in] newword : 新字
     * @return : 成功返回0
     *           失败返回1:移动时发生冲突
     **/
    int movesegment(int oldseg, int oldlen, int newseg, int newlen, int newid);

    /**
     * @brief : 用于压缩hash表，去掉各段之间空白的部分
     * @return : 成功返回0
     **/
    int checkhash();

    /**
     * @brief : 截取模板中的模板词类型，例：从“[D:地方][D:地图]”这个模板中截取出“[D:地方]”这个模板词类型
     * @param [out] nowstr : 截取结果
     * @param [in] form : 输入模板字符串
     * @param [in] nowpos : 输入模板字符串开始截取的位置
     * @return : 返回截取的字符串长度
     **/
    int cutform(std::string& nowstr, char* form, int nowpos);

    /**
     * @brief : 获得模板词类型、固定词、函数、通配符在_formlist[]中的下标,若不存在则做插入操作
     * @param [in] form : 模板类型、固定词、函数、通配符类型的字符串，如：[D:name] 在哪里 [F:num] [W:0-8]
     * @param [in] tag : 标记 tag[0]: r 只读操作
     *                                w 写操作
     *                        tag[1]: W  通配符
     *                                F  函数
     *                                O  其它两种类型
     * @return : 成功返回formid号，即_formlist下标
     *           失败返回-1
     **/
    int getformid(const char* form, char* proper, const char* tag);

    /**
     * @brief : 向树中新加入一个新模版
     * @param [in] input : 模板
     * @param [in] proper : 模板属性
     * @return 成功返回0 失败返回-1
     **/
    int insert(char* input, const char* proper, std::multimap<std::string,type_t>& word_record);

    /**
     * @brief : 向树中新加入所有term
     * @param [in] word_record : 所有term
     * @return 成功返回0 失败返回-1
     **/
    int insert(std::multimap<std::string,type_t>& word_record);

    /**
     * @brief : 在模板树中查找当前query，匹配模板词、停用词、固定词,在search()中调用
     * @param [in/out] now_statue : 递归搜索时当前状态
     * @param [out] result_t : 记录结果的数据结构
     **/
    void matchPatIgn(pat_statue* now_statue,pat_res_t* result);

    /**
     * @brief : 在模板树中查找当前query，匹配通配符,在search()中调用
     * @param [in/out] now_statue : 递归搜索时当前状态
     * @param [out] result_t : 记录结果的数据结构
     **/
    void matchWildCard(pat_statue* now_statue,pat_res_t* result);

    /**
     * @brief : 在模板树中查找当前query，匹配函数,在search()中调用
     * @param [in/out] now_statue : 递归搜索时当前状态
     * @param [out] result_t : 记录结果的数据结构
     **/
    void matchFun(pat_statue* now_statue,pat_res_t* result);

private:
 
    // 记录路径的单元
    typedef struct form_trans_node {
        int len;                // 记录当前匹配词的长度
        int formid;             // 记录当前前缀所对应的类型id(模板词和固定词时为_formlist[]中的下标,停用词为-3)
        const char* proper;     // 匹配词的属性
    }ftran_n;
    // 单词所属模板类型
    typedef struct word_type {
        int type;               // 单词所属的类型   对应formid
        char* proper;           // 单词属性,每个词条后面的属性
        word_type* next;        // 链表指针
    } type_w;
    // 单词列表中的节点：单词，属性，类型链表
    typedef struct word_node {
        char* word;             // 记录单词
        type_w* type;           // 记录单词类型的链表
    } node_w;
    //data
    node_w*     _wordlist;      // 单词表
    int         _wmaxlen;       // 单词表最大长度
    int         _wnowlen;       // 单词表当前长度
    TrieDict* _word_triedict;
    ///////////////////////////////////////////////////////////////////////////
    //存放外部函数接口结构
    typedef struct func_node{
        char name[PAT_MAX_WORDS_LEN];    // 函数名
        int (*fptr)(const char *);      // 模板匹配的函数指针
    }func_t;
    // 通配符数据结构
    typedef struct wildcard_node{
        char name[PAT_MAX_WORDS_LEN];    // 通配符名
        int down;                       // 通配符匹配的字符串长度下界
        int up;                         // 通配符匹配的字符串长度上界
    }wildc_t;
    // hash表单元
    typedef struct form_hash_node
    {
        int formid;                 // 模板id，如果为头节点则记录当前节点是否有函数或者通配符
        int next;                   // 指向的下一个节点编号，如果为头节点则为当前段长(包含头节点)
        int father;                 // 指向nodelist里面的下标
    }fnode_h;
    // 模板树的节点
    typedef struct form_tree_node
    {
        char* resultproper;        // 记录匹配出口所对应的属性,非出口为NULL
        int segpos;                 // 该节点所对应的hash表段的首地址下标
    }fnode_t;

    // 模板词类型列表单元
    typedef struct form_node
    {
        char *form;                 // 类型 : 模板词、固定词、函数、通配符,例如[D:地方],在哪里,[F:num],[W:1-5]
        char *proper;               // 属性
    }fnode_f;

    //data
    static const int FORM_HASH_SEG_LEN = 2; // hashlist中每个段表的初始长度
    static const int FORM_NO_DATA = -1;     // 不存在数据

    fnode_f*    _formlist;          // 模板词、固定词、函数、通配符->id
    fnode_t*    _nodelist;          // 树节点列表
    fnode_h*    _hashlist;          // hash表

    func_t      _funclist[PAT_MAX_FUN_NUM];         //函数
    wildc_t     _wildclist[PAT_MAX_WILDCARD_NUM];   //通配符

    int         _funcNum;       // 函数个数
    int         _wildcNum;      // 通配符个数
    int         _otherNum;      // 固定词和模板词个数
    int         _fmaxlen;       // formlist的最大值
    int         _nmaxlen;       // nodelist的最大值
    int         _nnowlen;       // nodelist的当前长度
    int         _hmaxlen;       // hashlist的最大长度
    int         _hseglen;       // hashlist中每个段表的初始长度
    int         _hnowlen;       // hashlist的当前长度
};
}//namespace
#endif //__PATTERN_PARSER_CORE_H_

