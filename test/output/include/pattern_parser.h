#ifndef  __PATTERN_PARSER_H_
#define  __PATTERN_PARSER_H_
#include <stdio.h>
#include <stdlib.h>
#include "ul_log.h"
namespace NLP {

#ifndef FATAL_LOG
#define FATAL_LOG(fmt,arg...) ul_writelog(UL_LOG_FATAL,"[%s() %s:%d] "fmt,__FUNCTION__,__FILE__,__LINE__,##arg);    ///< fatal日志
#endif
#ifndef WARNING_LOG
#define WARNING_LOG(fmt,arg...) ul_writelog(UL_LOG_WARNING,"[%s() %s:%d] "fmt,__FUNCTION__,__FILE__,__LINE__,##arg);///< warning日志
#endif
#ifndef TRACE_LOG
#define TRACE_LOG(fmt,arg...) ul_writelog(UL_LOG_TRACE,"[%s() %s:%d] "fmt,__FUNCTION__,__FILE__,__LINE__,##arg);    ///< trace日志
#endif
#ifndef DEBUG_LOG
#define DEBUG_LOG(fmt,arg...) ul_writelog(UL_LOG_DEBUG,"[%s() %s:%d] "fmt,__FUNCTION__,__FILE__,__LINE__,##arg);    ///< debug日志
#endif
#ifndef NOTICE_LOG
#define NOTICE_LOG(fmt,arg...) ul_writelog(UL_LOG_NOTICE,"[%s() %s:%d] "fmt,__FUNCTION__,__FILE__,__LINE__,##arg);  ///< notice日志
#endif
/*
#ifndef FATAL_LOG
#define FATAL_LOG(fmt,arg...)\
    fprintf(stderr,"[%s() %s:%d] \n"fmt,__FUNCTION__,__FILE__,__LINE__,##arg);   
#endif
#ifndef WARNING_LOG
#define WARNING_LOG(fmt,arg...)\
    fprintf(stderr,"[%s() %s:%d] \n"fmt,__FUNCTION__,__FILE__,__LINE__,##arg); 
#endif
#ifndef TRACE_LOG
#define TRACE_LOG(fmt,arg...)\
    fprintf(stderr,"[%s() %s:%d] \n"fmt,__FUNCTION__,__FILE__,__LINE__,##arg); 
#endif
#ifndef DEBUG_LOG
#define DEBUG_LOG(fmt,arg...)\
    fprintf(stderr,"[%s() %s:%d] \n"fmt,__FUNCTION__,__FILE__,__LINE__,##arg); 
#endif
#ifndef NOTICE_LOG
#define NOTICE_LOG(fmt,arg...)\
    fprintf(stderr,"[%s() %s:%d] \n"fmt,__FUNCTION__,__FILE__,__LINE__,##arg);
#endif
*/

//结果term个数超出这个范围的将被截断
const int RES_TERMS_SIZE = 50;
//词条的最大长度
const int PAT_MAX_WORDS_LEN = 100;
//模版最大长度
const int PAT_MAX_PATTS_LEN = 10240;

// 槽位的类型
typedef enum slot_type {
    T_FIX,  //固定词，模版中的明文词
    T_PAT,  //模版中槽位词表中的词
    T_FUNC, //函数型槽位
    T_IGN,  //停用词槽位
    T_WILDC //通配符
} SLOT_TYPE;

//模版中槽位匹配结果
typedef struct slot_res {
    SLOT_TYPE tfType;   		    //槽位类型
    int begin;			            //在原query中的起始位置
    int len;			            //这个词的长度
    char term[PAT_MAX_WORDS_LEN];   //匹配到的文本
    char str[PAT_MAX_WORDS_LEN];    //槽位的类型，PAT:"[D:area_place]",FUNC:"[F:num]",WILDC:"[W:1-10]",IGN:空串T_FIX:空串
    char extra[PAT_MAX_WORDS_LEN];  //匹配词在词表中附加信息，非词表的为空
    slot_res()
    {
        tfType = T_FIX;
        begin = 0;
        len = 0;
        str[0] = 0;
        extra[0] = 0;
    }
}slot_t;

//匹配结果
typedef struct pat_result {
    char pat_str[PAT_MAX_PATTS_LEN];//
    char pat_extra[PAT_MAX_PATTS_LEN];//
    int terms_num;				    //匹配词的个数
    int match_len;				    //实际匹配的长度
    slot_t termsRes[RES_TERMS_SIZE];//每一个词对应的槽位信息    
    float score;                    //模版分值
    pat_result()
    {
        pat_str[0] = 0;
        pat_extra[0] = 0;
        terms_num = 0;
    }
}pat_res_t;

class pattern_parser_core;
class pattern_parser
{
public:

    /**
     * @bried 通过读入二进制文件初始化数据结构
     * @param [in] file_pre : 二进制文件前缀名
     * @return : 成功返回true，失败返回false
     **/
    bool pat_load_bin(const char* file_pre);
    /**
     * @brief 通过读入模板文件，模板词文件，停用词文件初始化数据结构
     * @param [in] pat_fname : 模板文件名
     * @param [in] dict_fname : 模板词文件名
     * @param [in] igno_fname : 停用词文件名
     * @return : 成功返回true，失败返回false
     **/
    bool pat_create(const char* pat_fname,const char* term_fname,const char* igno_fname = NULL);
    /**
     * @brief 对输入数据进行解析
     * @param [in] input : 输入数据结构
     * @param [out] result : 结果数据结构
     * @param [in] res_size : 匹配模板最大个数
     * @return : 匹配模板个数，失败返回-1
     **/
    int pat_parser(const char* query, pat_res_t* result, const int res_size);
    /**
     * @brief 对输入数据进行解析
     * @param [in] input : 输入数据结构
     * @param [out] result : 结果数据结构
     * @return : 匹配模板个数，失败返回-1
     **/
    int pat_parser(const char* query, pat_res_t& result);
    /**
     * @brief 提供外部注册函数
     * @param [in] name : 函数名 如[F:num]
     * @param [in] fptr : 函数指针
     * @return : 正确返回0，错误返回-1
     **/
    bool pat_reg_func(const char *name, int(*fptr)(const char *));
    /**
     *@brief dump数结构到文件
     *@param [in] name_pre: 存储二进制的文件名
     *@return : 成功返回0,失败返回-1
     **/
    int pat_dump_bin(const char* name_pre);
    /**
     * @brief 构造函数
     **/
    pattern_parser();
    /**
     * @brief 析构函数
     **/
    ~pattern_parser();
private:
    //功能对象
    pattern_parser_core* p_core;
};
}//namespace
#endif
