#ifndef __CODE_CONVERSION_H__
#define __CODE_CONVERSION_H__

#include <string>
#ifdef linux
#include <locale>
#include <cstdlib>
#else
#include <Windows.h>
#endif

#include "ul_ccode.h"
#include "UtilsLib.h"
#include "ul_log.h"

using std::string;
using std::wstring;

class CCodeConversion
{
public:
	static wstring UTF8ToUnicode(const char* line)
	{
#ifdef linux
		setlocale(LC_ALL, "zh_CN.UTF-8");
		size_t size = mbstowcs(NULL, line, 0);
		wchar_t* wcstr = new wchar_t[size + 1];
		if (!wcstr)
			return L"";
		mbstowcs(wcstr, line, size+1);
#else
		size_t size = MultiByteToWideChar(CP_UTF8, 0, line, -1, NULL, 0);
		wchar_t* wcstr = new wchar_t[size];
		if (!wcstr)
			return L"";
		MultiByteToWideChar(CP_UTF8, 0, line, -1, wcstr, size);	
#endif
		wstring final(wcstr);
		delete[] wcstr;

		return final;
	}

	static wstring UTF8ToUnicode(string& line)
	{
#ifdef linux
		setlocale(LC_ALL, "zh_CN.UTF-8");
		size_t size = mbstowcs(NULL, line.c_str(), 0);
		wchar_t* wcstr = new wchar_t[size + 1];
		if (!wcstr)
			return L"";
		mbstowcs(wcstr, line.c_str(), size+1);
#else
		size_t size = MultiByteToWideChar(CP_UTF8, 0, line.c_str(), -1, NULL, 0);
		wchar_t* wcstr = new wchar_t[size];
		if (!wcstr)
			return L"";
		MultiByteToWideChar(CP_UTF8, 0, line.c_str(), -1, wcstr, size);	
#endif
		wstring final(wcstr);
		delete[] wcstr;

		return final;
	}

	static string UnicodeToUTF8(wstring& line)
	{
#ifdef linux
		setlocale(LC_ALL, "zh_CN.UTF-8");
		size_t size = wcstombs(NULL, line.c_str(), 0);
		char* mbstr = new char[size + 1];
		if (!mbstr)
			return "";
		wcstombs(mbstr, line.c_str(), size+1);
#else
		size_t size = WideCharToMultiByte(CP_UTF8, 0, line.c_str(), -1, NULL, 0, NULL, NULL);
		char* mbstr = new char[size];
		if (!mbstr)
			return "";
		WideCharToMultiByte(CP_UTF8, 0, line.c_str(), -1, mbstr, size, NULL, NULL);
#endif
		string final(mbstr);
		delete[] mbstr;

		return final;
	}

	static wstring GBToUnicode(string& line)
	{
#ifdef linux
		setlocale(LC_ALL, "zh_CN.GB2312");
		size_t size = mbstowcs(NULL, line.c_str(), 0);
		wchar_t* wcstr = new wchar_t[size + 1];
		if (!wcstr)
			return L"";
		mbstowcs(wcstr, line.c_str(), size+1);
#else
		size_t size = MultiByteToWideChar(CP_ACP, 0, line.c_str(), -1, NULL, 0);
		wchar_t* wcstr = new wchar_t[size];
		if (!wcstr)
			return L"";
		MultiByteToWideChar(CP_ACP, 0, line.c_str(), -1, wcstr, size);	
#endif
		wstring final(wcstr);
		delete[] wcstr;

		return final;
	}

	static string UnicodeToGB(wstring& line)
	{
#ifdef linux
		setlocale(LC_ALL, "zh_CN.GB2312");
		size_t size = wcstombs(NULL, line.c_str(), 0);
		char* mbstr = new char[size + 1];
		if (!mbstr)
			return "";
		wcstombs(mbstr, line.c_str(), size+1);
#else
		size_t size = WideCharToMultiByte(CP_ACP, 0, line.c_str(), -1, NULL, 0, NULL, NULL);
		char* mbstr = new char[size];
		if (!mbstr)
			return "";
		WideCharToMultiByte(CP_ACP, 0, line.c_str(), -1, mbstr, size, NULL, NULL);
#endif
		string final(mbstr);
		delete[] mbstr;

		return final;
	}

	static unsigned int GetUnicodeLen(wchar_t c)
	{
		wstring line;
		line += c;
#ifdef linux
		setlocale(LC_ALL, "zh_CN.GB2312");
		return wcstombs(NULL, line.c_str(), 0);
#else
		return WideCharToMultiByte(CP_ACP, 0, line.c_str(), -1, NULL, 0, NULL, NULL) - 1;
#endif
	}
};


//ÆÀÂÛ´îÅäÔª×é´Êµä
class CodeConversionTool 
{
public:
    
    
    CodeConversionTool();
    ~CodeConversionTool();
       
    bool ConvertUtf8ToGB(string& str);
    bool ConvertGBToUtf8(string& str);
private:    
    
};

#endif //CODE_CONVERSION_H_


