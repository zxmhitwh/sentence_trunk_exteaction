
#include "UtilsLib.h"

#define MAX_LINE_LEN 1024000

//格式化成字符串
string FormatString(const char *lpszFormat, ...) 
{
    char szBuffer[MAX_LINE_LEN] = {0};
	va_list pArgs;
	va_start(pArgs, lpszFormat);
	vsnprintf(szBuffer, MAX_LINE_LEN, lpszFormat, pArgs);
	return((string)szBuffer);
}

//字符串+
string operator + (const string sString1, const string sString2) 
{
	string sResult = sString1;
	sResult += sString2;
	return sResult;
}

//布尔型转字符串
string BoolToString(bool bBool) 
{
    return bBool?"true":"false";
}

//整型转字符串
string IntToString(int iInt) 
{
    return FormatString("%d", iInt);
}

//浮点型转字符串
string FloatToString(float fFloat) 
{
    return FormatString("%f", fFloat);
}

//字符串是否为数字
bool IsANumber(string sStr) 
{
	size_t i = 0;
	if (sStr.size() == 0)
		return false;
	if ((sStr[0] == '-')||(sStr[0] == '+'))
		i++;
	if (i == sStr.size())
		return false;
	while ((i < sStr.size()) && isdigit(sStr[i]))
		i++;
	if (i == sStr.size())
		return true;
	else if (sStr[i] == '.') 
    {
		i++;
		while ((i < sStr.size()) && isdigit(sStr[i]))
			i++;
		if (i == sStr.size())
			return true;
		else 
			return false;
	} else
		return false;
}

//转大写
string ToUpperCase(string sString) 
{
    transform(sString.begin(),sString.end(),sString.begin(),::toupper);
    return sString;
}

//转小写
string ToLowerCase(string sString) 
{
    transform(sString.begin(),sString.end(),sString.begin(),::tolower);
    return sString;
}

//去除左端无用字符
string TrimLeft(string sString, char * pToTrim) 
{
	unsigned int i = 0;
	while(i < sString.length()) 
	{
		char * c = pToTrim;
		while ( *c != '\0' && *c != sString[i] )
			c++;
		if ( *c == '\0' )
			break;
		i++;
	}
	string sResult = sString.substr(i, sString.length() - i);
	return sResult;
}

//去除右端无用字符
string TrimRight(string sString, char * pToTrim) 
{
	int i = sString.length()-1;
	while((i >= 0) )
	{
		char * c = pToTrim;
		while ( *c != '\0' && *c != sString[i] )
			c++;
		if ( *c == '\0' )
			break;
		i--;
	}
    if((i >= 1) && (sString[i] == 13))
    {
        --i;
    }
	string sResult = sString.substr(0, i+1);
	return sResult;
}

//取出两端无用字符
string Trim(string sString, char * pToTrim) 
{
	return TrimLeft(TrimRight(sString, pToTrim), pToTrim);
}

//从字符串取出第一行
string ExtractFirstLine(string& rString) 
{
	string sResult;
	SplitOnFirst(rString, "\n", sResult, rString);
	return sResult;
}

//从字符串截取第一段，用pDividers中任意字符分割
bool SplitOnFirstWithAnyChar(string sOriginal, string sDividers, 
				  string& rsFirstPart, string& rsSecondPart,char& cDiv) 
{
    int iCharPos = -1;
    for(unsigned int i = 0;i < sDividers.size();++i)
    {
        int iTmp = sOriginal.find_first_of(sDividers[i]);
        if(iTmp == -1)
        {
            continue;
        }
        if(iCharPos == -1)
        {
            iCharPos = iTmp;
            continue;
        }
        if(iTmp < iCharPos)
        {
            iCharPos = iTmp;
        }
    }
    if(iCharPos == -1) 
    {
		rsFirstPart = sOriginal;
		rsSecondPart = "";
        cDiv = '-';
		return false;
	} 
    else 
    {
        cDiv = sOriginal[iCharPos];
		rsFirstPart = sOriginal.substr(0, iCharPos);
		rsSecondPart = sOriginal.substr(iCharPos + 1, 
										sOriginal.length() - iCharPos);
		return true;			
	}
}

//从字符串截取第一段
bool SplitOnFirst(string sOriginal, char* pDividers, 
				  string& rsFirstPart, string& rsSecondPart) 
{
    int iCharPos = sOriginal.find_first_of(pDividers);
    if(iCharPos == -1) 
    {
		rsFirstPart = sOriginal;
		rsSecondPart = "";
		return false;
	} 
    else 
    {
		rsFirstPart = sOriginal.substr(0, iCharPos);
		rsSecondPart = sOriginal.substr(iCharPos + 1, 
										sOriginal.length() - iCharPos);
		return true;			
	}
}

// 截取第一段
bool SplitOnFirstWithStr(string sOriginal, string sDividers, 
				  string& rsFirstPart, string& rsSecondPart)
{
    string::size_type pos = sOriginal.find(sDividers);
    if(pos != string::npos)
    {
        rsFirstPart = sOriginal.substr(0,pos);
        rsSecondPart = sOriginal.substr(pos+sDividers.size());
        return true;
    }
    else
    {
        rsFirstPart = sOriginal;
        rsSecondPart = "";
        return false;
    }
}

// 截取最后一段
bool SplitOnLastWithStr(string sOriginal, string sDividers, 
				  string& rsFirstPart, string& rsSecondPart)
{
    string::size_type pos = sOriginal.rfind(sDividers);
    if(pos != string::npos)
    {
        rsFirstPart = sOriginal.substr(0,pos);
        rsSecondPart = sOriginal.substr(pos+sDividers.size());
        return true;
    }
    else
    {
        rsFirstPart = "";
        rsSecondPart = sOriginal;
        return false;
    }
}

//从字符串截取第一段
bool SplitOnFirst(string sOriginal, char* pDividers, 
				  string& rsFirstPart, string& rsSecondPart, char cQuote) 
{
    int i = 0;
    bool bWithinQuotes = false;
    int l = sOriginal.length();
    while(i < l) 
    {
        if(bWithinQuotes) 
        {
            while((sOriginal[i] != cQuote) && (i < l)) 
                i++;
            if(i == l) 
            {
                rsFirstPart = sOriginal;
                rsSecondPart = "";
                return false;
            }
            i++;
            bWithinQuotes = false;
        } 
        else if(sOriginal[i] == cQuote) 
        {
            bWithinQuotes = true;
            i++;
        } 
        else if(strchr(pDividers, sOriginal[i]) != NULL) 
        {
            rsFirstPart = sOriginal.substr(0, i);
            rsSecondPart = sOriginal.substr(i + 1, sOriginal.length() - i);
            return true;
        } 
        else 
        {
            i++;
        }
    }
    rsFirstPart = sOriginal;
    rsSecondPart = "";
    return false;
}

//从字符串截取最后一段
bool SplitOnLast(string sOriginal, char* pDividers, 
				  string& rsFirstPart, string& rsSecondPart) 
{
    int iCharPos = sOriginal.find_last_of(pDividers);
	if(iCharPos == -1) 
    {
		rsFirstPart = "";
		rsSecondPart = sOriginal;
		return false;
	} 
    else 
    {
		rsFirstPart = sOriginal.substr(0, iCharPos);
		rsSecondPart = sOriginal.substr(iCharPos + 1, 
										sOriginal.length() - iCharPos);
		return true;			
	}
}

void PartitionStringWithStr(string sString,vector<string>& saResult, string sDividers)
{
	string sTemp;
	while(sString != "")
	{
        SplitOnFirstWithStr(sString,sDividers,sTemp,sString);
		if(sTemp != "")
        {
			saResult.push_back(sTemp);
        }
	}
}

//切分字符串
vector<string> PartitionString(string sString, char * pDividers) 
{
	vector<string> saResult;
	string sTemp;
	while (sString != "" )
	{
		SplitOnFirst(sString, pDividers, sTemp, sString);
		if(sTemp != "")
			saResult.push_back(sTemp);
	}
	return saResult;
}

//切分字符串
void PartitionStringWithAnyChar(string sString,vector<string>& saResult,vector<char>& vcDivPos,char* pDividers) 
{
	string sTemp;
	while (sString != "" )
	{
        char cDiv;
        SplitOnFirstWithAnyChar(sString,pDividers,sTemp,sString,cDiv);
		if(sTemp != "")
        {
			saResult.push_back(sTemp);
            vcDivPos.push_back(cDiv);
        }
	}
    if(vcDivPos.size() > 0)
    {
        vcDivPos.pop_back();
    }
} 

//切分字符串
void PartitionString(string sString,vector<string>& saResult, char * pDividers) 
{
	string sTemp;
	while (sString != "" )
	{
		SplitOnFirst(sString, pDividers, sTemp, sString);
		if ( sTemp != "" )
			saResult.push_back(sTemp);
	}
} 

//切分字符串
vector<string> PartitionString(string sString, char * pDividers, char cQuote) 
{
	vector<string> saResult;
	string sTemp;
	while (sString != "" )
	{
		SplitOnFirst(sString, pDividers, sTemp, sString, cQuote);
		if ( sTemp != "" )
			saResult.push_back(sTemp);
	}
	return saResult;
}

//切分字符串
void PartitionString(string sString,vector<string>& saResult,char * pDividers, char cQuote) 
{
	string sTemp;
	while (sString != "" )
	{
		SplitOnFirst(sString, pDividers, sTemp, sString, cQuote);
		if ( sTemp != "" )
			saResult.push_back(sTemp);
	}
} 

//子串替换
string ReplaceSubString(string sSource, string sToReplace, string sReplacement) 
{
    string sResult = sSource;
    int pos = 0;
    while((pos = sResult.find(sToReplace, pos)) >= 0) 
    {
        sResult.replace(pos, sToReplace.length(), sReplacement);
		pos += sReplacement.length();
    }
    return sResult;
}

//取出对称括号部分
unsigned int FindClosingQuoteChar(string sString, unsigned int iStartPos, 
								  char cOpenQuote, char cCloseQuote) 
{
	unsigned int iOpenBraces = 1;	
	unsigned int iPos = iStartPos;
	while((iOpenBraces > 0) && (iPos < sString.length())) 
    {
		if(sString[iPos] == cOpenQuote) 
			iOpenBraces++;
		else if(sString[iPos] == cCloseQuote)
			iOpenBraces--;
		iPos++;
	}
	return iPos;
}

//map<string,string>转字符串
string S2SHashToString(STRING2STRING s2sHash, string sSeparator, 
                       string sEquals) 
{
    string sResult = "";
	if (!s2sHash.empty()) 
    {
		STRING2STRING::iterator iPtr;
		for(iPtr = s2sHash.begin(); iPtr != s2sHash.end(); iPtr++) 
        {
			if (iPtr != s2sHash.begin()) 
                sResult += sSeparator;
			sResult += iPtr->first + sEquals + iPtr->second;
		}
	}
    return sResult;
}

//字符串转map
STRING2STRING StringToS2SHash(string sString, string sSeparator, 
                              string sEquals) 
{
    TStringVector vsPairs = 
        PartitionString(sString, (char *)sSeparator.c_str());
    STRING2STRING s2s;
    for(unsigned int i = 0; i < vsPairs.size(); i++) 
    {
        string sAttr, sValue;
        SplitOnFirst(vsPairs[i], (char *)sEquals.c_str(), sAttr, sValue);        
        s2s.insert(STRING2STRING::value_type(Trim(sAttr), Trim(sValue)));
    }
    return s2s;
}

//字符串转map
void StringToS2SHash(string sString,STRING2STRING& s2s, string sSeparator, 
                    string sEquals) 
{
    TStringVector vsPairs = 
        PartitionString(sString, (char *)sSeparator.c_str());
    for(unsigned int i = 0; i < vsPairs.size(); i++) 
    {
        string sAttr, sValue;
        SplitOnFirst(vsPairs[i], (char *)sEquals.c_str(), sAttr, sValue);        
        s2s.insert(STRING2STRING::value_type(Trim(sAttr), Trim(sValue)));
    }
}

//map追加
void AppendToS2S(STRING2STRING& rs2sInto, STRING2STRING& rs2sFrom) 
{
    STRING2STRING::iterator iPtr;
    for(iPtr = rs2sFrom.begin(); iPtr != rs2sFrom.end(); iPtr++) 
        rs2sInto.insert(STRING2STRING::value_type(iPtr->first, iPtr->second));
}

//删除字符
string RemoveChar(const string& str, char ch)
{
    string ss;
    size_t len = str.size();
    for(size_t i = 0; i < len;++i)
    {
       if (str[i] != ch )
       {
          ss.push_back(str[i]);
       }
    }
    return ss;
}

//删除字符
string RemoveChar(const string& str, string sCh)
{
    string ss;
    size_t len = str.size();
    for(size_t i = 0; i < len;++i)
    {
        if(sCh.find(str[i]) == string::npos)
        {
            ss.push_back(str[i]);
        }
    }
    return ss;
}

static int __ID = 0;

//获取唯一ID
string GetUniqueStringID() 
{
	__ID++;
	assert( __ID );
	return (FormatString("%d", __ID - 1));
}

//获取唯一ID
int GetUniqueIntID() 
{
	__ID++;
	assert( __ID );
	return ( __ID - 1 );
}

//获取随机整数ID
int GetRandomIntID() 
{
    return rand();
}


static long long iStartUpTimestamp;

//设置初始时间
void InitializeHighResolutionTimer() 
{
    struct timeb timebuffer;
    ftime(&timebuffer);
    iStartUpTimestamp = 1000*(long long)timebuffer.millitm + timebuffer.time;
}

//timeb格式返回时间
timeb GetTime() 
{
    struct timeb timebuffer;
    ftime(&timebuffer);
    return timebuffer;
}

//字符形式返回当前时间
string GetTimeAsString() 
{
    struct timeb timebuffer;
    ftime(&timebuffer);
	return TimeToString(timebuffer);
}

//timeb转字符串
string TimeToString(timeb time) 
{
	tm* tmTime;
	string sResult;
	tmTime = localtime(&time.time);
    return FormatString("%02d:%02d:%02d.%03d", 
                        tmTime->tm_hour, 
		                tmTime->tm_min, 
                        tmTime->tm_sec, 
                        time.millitm);
}

//获取决定时间戳
long long GetCurrentAbsoluteTimestamp() 
{
    struct timeb timebuffer;
    ftime(&timebuffer);
    return (long long)timebuffer.millitm + 1000*timebuffer.time;
}

//休眠ms
void Sleep( int iDelay )
{
   int iGoal;
   iGoal = iDelay + (int)clock();
   while (iGoal > clock());
}


//读取文件
bool ReadFromFile(const string& filename,vector<string>& lines)
{
    lines.clear();
    FILE* fp = fopen(filename.c_str(),"r");
    if (NULL == fp)
    {
        return false;
    }   
    char line[MAX_LINE_LEN] = {0};
    while(fgets(line,MAX_LINE_LEN-1,fp))
    {
        string str_line = Trim(line,"\r\n");
        if (!str_line.empty())
        {
            lines.push_back(str_line);
        }   
    }   
    fclose(fp);
    return !lines.empty();
}


///////////////////////////////////////////////////////////////////////////////
//默认日期获取
int __GetMonthDayNum(int year,int month)
{
    int max_day = 0;
    if(month == 2)
    {
        if(year%100==0 && year%400>0 || year%4 != 0)
        {
            max_day = 28;
        }
        else
        {
            max_day = 29;
        }
    }
    else if (month ==2 || month == 4 || month == 6 || month == 9 || month == 11)
    {
        max_day = 30;
    }
    else
    {
        max_day = 31;
    }
    return max_day;
}

int AddOffsetDays(int &year,int &month,int &day,int add_days)
{
    int max_days = __GetMonthDayNum(year,month);
    if(add_days > 28)
    {
        return -1;
    }
    day += add_days;
    if(day > max_days)
    {
        day -= max_days;
        month += 1;
        if(month > 12)
        {
            month -= 12;
            year += 1;
        }
    }
    return 0;
}

string GetDateStr(int iOffsetDays)
{
    time_t now;
    struct tm *fmt;
    time(&now);
    fmt = localtime(&now);
    int oyear = fmt->tm_year+1900;
    int omonth = fmt->tm_mon + 1;
    int oday = fmt->tm_mday;
    AddOffsetDays(oyear,omonth,oday,iOffsetDays);
    return FormatString("%04d-%02d-%02d,%04d-%02d-%02d,0",oyear,omonth,oday,oyear,omonth,oday);
}

void GetYMD(string sDate,int &year,int &month,int &day)
{
    string sFirst,sSecond;
    SplitOnFirst(sDate,",",sFirst,sSecond);
    if(sSecond.empty())
    {
        return;
    }
    sscanf(sFirst.c_str(),"%d-%d-%d",&year,&month,&day);
}

/*
ostream& operator<<(ostream& os, const vector<string>& m)
{
    string s = "[";
    for(uint i = 0;i < m.size();++i)
    {
        s += m[i] + "\t";
    }
    s = Trim(s,"\t") + "]";
    os << s.c_str();
    return os;
}
*/

ostream& operator<<(ostream& os, const vector<string>& m)
{
    string s = "[";
    for(uint i = 0;i < m.size();++i)
    {
        s += "\"" + m[i] + "\",";
    }
    s = Trim(s,",") + "]";
    os << s.c_str();
    return os;
}

ostream& operator<<(ostream& os, const vector<float>& m)
{
    string s = "[";
    for(uint i = 0;i < m.size();++i)
    {
        s += "\"" + FormatString("%f",m[i]) + "\",";
    }
    s = Trim(s,",") + "]";
    os << s.c_str();
    return os;
}
ostream& operator<<(ostream& os, map<string,string>& m)
{
    string s = "[";
    for(map<string,string>::iterator it = m.begin();it != m.end();++it)
    {
        s += it->first + ":" + it->second + "\t";
    }
    s = Trim(s,"\t") + "]";
    os << s.c_str();
    return os;
}

int code_convert(char *from_charset,char *to_charset,char *inbuf,char *outbuf)
{/*
    size_t inlen = strlen(inbuf);
    size_t outlen = strlen(outbuf);
   // iconv_t cd = iconv_open(to_charset,from_charset);
    if (cd==0) 
    {
        return -1;
    }
    memset(outbuf,0,outlen);
    if ((int)iconv(cd,&inbuf,&inlen,&outbuf,&outlen)==-1) 
    {
        iconv_close(cd);
        return -1;
    }
    iconv_close(cd);*/
    return 0;
}

namespace xstr
{
    std::string strip(const std::string &str, const char *strip_chars)
    {
        int n = (int)str.size();
        
        int s = 0;
        while (s < n - 1 && strchr(strip_chars, str[s]) != NULL)
        {
            s++;
        }
        int e = n - 1;
        while (e >= s && strchr(strip_chars, str[e]) != NULL)
        {
            e--;
        }
        return str.substr(s, e - s + 1); 
    }

    void split(const std::string &str, const std::string &sep, std::vector<std::string> &items)
    {
        int sep_size = (int)sep.size();
        size_t sep_pos = str.find(sep);
        size_t lst_pos = 0;
        while(sep_pos != std::string::npos)
        {
            items.push_back(str.substr(lst_pos, sep_pos - lst_pos));
            lst_pos = sep_pos + sep_size;
            sep_pos = str.find(sep, lst_pos);
        }
        if (lst_pos <= str.size())
        {
            items.push_back(str.substr(lst_pos));
        }
    }

    void split(const std::string &str, char sep, std::vector<std::string> &items)
    {
        split(str, std::string(1, sep), items);
    }

    std::string join(const std::vector<std::string> &items, const std::string &sep)
    {
        std::string res = "";
        if (0 == (int)items.size()) return res;
        res += items[0];
        for (int i = 1; i < (int)items.size(); i++)
        {
            res += sep + items[i];
        }
        return res;
    }

    std::string join(const std::vector<std::string> &items, char sep)
    {
        return join(items, std::string(1, sep));
    }

    std::string to_lower(const std::string &str)
    {
        std::string lower = str;
        for (int i = 0; i < (int)lower.size(); i++)
        {
            if ((lower[i] >= 'A') && (lower[i] <= 'Z'))
            {
                lower[i] += 'a' - 'A';
            }
        }
        return lower;
    }

    bool contains(const std::string &str, const std::string &pattern)
    {
        return str.find(pattern) != std::string::npos;
    }

    int contains_num(const std::string &str, const std::string &pattern)
    {
        int count = 0;
        size_t loc = 0;
        size_t len = pattern.length();
        while((loc = str.find(pattern, loc)) != std::string::npos)
        {
            count ++;
            loc += len;
        }
        return count;
    }

    void replace(std::string &str, const std::string from_str, const std::string to_str)
    {
        size_t start = str.find(from_str);
        while (std::string::npos != start)
        {
            str.replace(start, from_str.size(), to_str);
            start = str.find(from_str);
        }
    }

    char *new_c_str(const std::string str)
    {
        char *c_str = new char[str.length() + 1];
        strncpy(c_str, str.c_str(), str.length());
        c_str[str.length()] = '\0';
        return c_str;
    }
}


