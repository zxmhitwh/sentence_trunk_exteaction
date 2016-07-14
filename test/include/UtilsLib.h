#ifndef __UTILS_LIB_H__
#define __UTILS_LIB_H__

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <assert.h>
#include <math.h>

#include <map>
#include <vector>
#include <list>
#include <set>
#include <string>
#include <queue>
#include <stack>
#include <algorithm>
#include <limits>
#include <string.h>
#include <sstream>
#include <iostream>
#include <iconv.h>
#include "ul_log.h"

using namespace std;

typedef vector <int> TIntVector;
typedef vector <string> TStringVector;
typedef map <string,  string> STRING2STRING;
typedef map <string,  float> STRING2FLOAT;
typedef map <string,  STRING2FLOAT> STRING2STRING2FLOAT;

#define KEFU_LOG_DEBUG(message, ...) ul_writelog(UL_LOG_DEBUG, "[%s]: "message, __FUNCTION__, ##__VA_ARGS__)
#define KEFU_LOG_TRACE(message, ...) ul_writelog(UL_LOG_TRACE, "[%s]: "message, __FUNCTION__, ##__VA_ARGS__)
#define KEFU_LOG_WARNING(message, ...) ul_writelog(UL_LOG_WARNING, "[%s]: "message, __FUNCTION__, ##__VA_ARGS__)
#define KEFU_LOG_ERROR(message, ...) ul_writelog(UL_LOG_FATAL, "[%s]: "message, __FUNCTION__, ##__VA_ARGS__)

//-----------------------------------------------------------------------------
//字符串操作相关
//-----------------------------------------------------------------------------

//格式化成字符串
string FormatString(const char *lpszFormat,  ...);

//字符串+
string operator + (const string sString1,  const string sString2);

//布尔型转字符串
string BoolToString(bool bBool);

//整型转字符串
string IntToString(int iInt);

//浮点型转字符串
string FloatToString(float fFloat);

//字符串是否为数字
bool IsANumber(string sStr);

//转大写
string ToUpperCase(string sString);

//转小写
string ToLowerCase(string sString);

//去除左端无用字符
string TrimLeft(string sString,  char * pToTrim = " \n\r\t");

//去除右端无用字符
string TrimRight(string sString,  char * pToTrim = " \n\r\t");

//取出两端无用字符
string Trim(string sString,  char * pToTrim = " \n\r\t");

//从字符串取出第一行
string ExtractFirstLine(string& rString);

//从字符串截取第一段，用pDividers中任意字符分割
bool SplitOnFirstWithAnyChar(string sOriginal,  string sDividers,  
				  string& rsFirstPart,  string& rsSecondPart, char& cDiv) ;

//从字符串截取第一段
bool SplitOnFirst(string sOriginal,  char* pDividers,  
				  string& rsFirstPart,  string& rsSecondPart);

//从字符串截取第一段，分割符也为字符串
bool SplitOnFirstWithStr(string sOriginal,  string sDividers,  
				  string& rsFirstPart,  string& rsSecondPart);

//从字符串截取第一段
bool SplitOnFirst(string sOriginal,  char* pDividers,  
				  string& rsFirstPart,  string& rsSecondPart,  char cQuote);

//从字符串截取最后一段
bool SplitOnLast(string sOriginal,  char* pDividers,  
				 string& rsFirstPart,  string& rsSecondPart);

// 截取最后一段
bool SplitOnLastWithStr(string sOriginal,  string sDividers,  
				  string& rsFirstPart,  string& rsSecondPart);

void PartitionStringWithAnyChar(string sString, vector<string>& saResult, vector<char>& vcDivPos, char* pDividers);

//切分字符串
vector<string> PartitionString(string sString,  char * pDividers);
void PartitionString(string sString, vector<string>& saResult,  char * pDividers);
void PartitionStringWithStr(string sString, vector<string>& saResult,  string sDividers);

//切分字符串
vector<string> PartitionString(string sString,  char * pDividers,  char cQuote);

//切分字符串
void PartitionString(string sString, vector<string>& saResult, char * pDividers,  char cQuote);

//子串替换
string ReplaceSubString(string sSource,  string sToReplace,  string sReplacement=string());

//取出对称括号部分
unsigned int FindClosingQuoteChar(string sString,  unsigned int iStartPos,  
								  char cOpenQuote,  char cCloseQuote);
								  
//map<string, string>转字符串
string S2SHashToString(STRING2STRING s2sHash,  
                       string sSeparator = ",  ",  string sEquals = " = ");

//字符串转map
STRING2STRING StringToS2SHash(string sString,  string sSeparator = ", ",  
                              string sEquals = "=");

//字符串转map
void StringToS2SHash(string sString, STRING2STRING& s2s,  
                     string sSeparator = ", ", 
                     string sEquals = "=");
                              
//map追加
void AppendToS2S(STRING2STRING& rs2sInto,  STRING2STRING& rs2sFrom);


//删除字符
string RemoveChar(const string& str,  char ch);
string RemoveChar(const string& str, string sCh);

//-----------------------------------------------------------------------------
//唯一ID相关函数
//-----------------------------------------------------------------------------

//获取唯一ID
string GetUniqueStringID();

//获取唯一ID
int GetUniqueIntID();

//获取随机整数ID
int GetRandomIntID();

//-----------------------------------------------------------------------------
//时间相关函数
//-----------------------------------------------------------------------------

//设置初始时间
void InitializeHighResolutionTimer();

//timeb格式返回时间
timeb GetTime();

//字符形式返回当前时间
string GetTimeAsString();

//获取决定时间戳
long long GetCurrentAbsoluteTimestamp();

//timeb转字符串
string TimeToString(timeb time);

//休眠ms
void Sleep(int iDelay);


//-----------------------------------------------------------------------------
//文件操作
//-----------------------------------------------------------------------------

//读取文件
bool ReadFromFile(const string& filename,  vector<string>& lines);

//-----------------------------------------------------------------------------
string GetDateStr(int iOffsetDays);

void GetYMD(string sDate, int &year, int &month, int &day);

int AddOffsetDays(int &year, int &month, int &day, int add_days);

ostream& operator<<(ostream& os,  const vector<string>& m);

ostream& operator<<(ostream& os,  map<string, string>& m);

ostream& operator<<(ostream& os, const vector<float>& m);

int code_convert(char *from_charset, char *to_charset, char *inbuf, char *outbuf);



namespace xstr
{

    /**
     * 字符串去除首尾空格等
     * @param[in]   str             待处理字符串
     * @param[in]   strip_chars     要strip的字符
     * @param[out]  无
     * @return                      返回处理后字符串
     */
    std::string strip(const std::string &str, const char *strip_chars = " \t\r\n");

    /**
     * 字符串切分
     * @param[in]   str             待处理字符串
     * @param[in]   sep             分隔的字符或字符串
     * @param[out]  items           切割后的字符串数组 
     * @return      无
     */
    void split(const std::string &str, const std::string &sep, std::vector<std::string> &items);
    void split(const std::string &str, char sep, std::vector<std::string> &items);
    
    /**
     * 字符串连接
     * @param[in]   items           待连接的字符串数组
     * @param[in]   sep             连接的字符或字符串
     * @param[out]  无 
     * @return                      返回连接后字符串
     */
    std::string join(const std::vector<std::string> &items, const std::string &sep);
    std::string join(const std::vector<std::string> &items, char sep);

    /**
     * 字符串小写转换
     * @param[in]   str             输入的字符串
     * @param[out]  无 
     * @return                      返回小写字符串
     */
    std::string to_lower(const std::string &str);

    /**
     * 判断字符串是否包含子串
     * @param[in]   str             待判断字符串
     * @param[in]   pattern         子串
     * @param[out]  无 
     * @return                      返回是否包含子串
     */
    bool contains(const std::string &str, const std::string &pattern);
    
    /**
     * 判断字符串包含次数
     * @param[in]   str             待判断字符串
     * @param[in]   pattern         子串
     * @param[out]  无 
     * @return                      包含子串的个数
     */
    int contains_num(const std::string &str, const std::string &pattern);

    /**
     * 替换原字符串中的子字符串
     * @param[in]   str             待替换的字符串
     * @param[in]   from_str        要替换的子串
     * @param[in]   to_str          替换成的子串
     * @param[out]  str             替换后的字符串通过原字符串返回
     * @return      无
     */
    void replace(std::string &str, const std::string from_str, const std::string to_str);

    /**
     * 字符串转换其他类型模版函数
     * @param[in]   str             待转换的字符串
     * @param[out]  res             转换结果
     * @return                      返回转换是否成功
     */
    template <class T>
    bool parse(const std::string &str, T &res)
    {
        T trans;
        std::stringstream ss;
        ss << str;
        ss >> trans;
        if (!ss.fail()) res = trans;
        return !ss.fail();
    }

    /**
     * 其他类型转字符串
     * @param[in]   d               待转换的数据
     * @param[out]  无
     * @return                      返回转换后的字符串
     */
    template <class T>
    std::string to_string(const T &d)
    {       
        std::stringstream ss;
        ss.precision(std::numeric_limits<T>::digits10);
        ss << d;
        std::string str;
        ss >> str;
        return str;
    }

    /**
     * 生成新的C字符串
     * @param[in]   str             输入字符串
     * @param[out]  无
     * @return                      返回新的C字符串指针
     */
    char *new_c_str(const std::string str);
}

#endif // __UTILS_LIB_H__
