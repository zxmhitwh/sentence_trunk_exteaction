#ifndef __MIN_EDIT_DISTANCE_H__
#define __MIN_EDIT_DISTANCE_H__

#include "code_conversion.h"
#include <string>
#include <sstream>
#include <string.h>
using std::string;
using std::wstring;
using std::stringstream;
using std::wstringstream;
#include <algorithm>

using std::reverse;

const unsigned int INSERT_COST = 1;
const unsigned int DELETE_COST = 1;
const unsigned int REPLACE_COST = 1;

const unsigned int INSERT_OPE = 1;
const unsigned int DELETE_OPE = 2;
const unsigned int REPLACE_OPE = 4;

class MinEditDistance
{
public:
	MinEditDistance();
	~MinEditDistance();

	void Init(string str1, string str2);
	void Init(wstring str1, wstring str2);
	bool CalaMED(unsigned int& med);
	//获取编辑距离的不同比例，不同字符除以最大的字符个数
	float getMED(string str1,string str2);
	bool BackTrace(string& strAlignment);
	bool BackTrace(wstring& strAlignment);

	string PrintMatrix();

private:
	void ClearMatrix();
	unsigned int CalaMin(unsigned int& x, unsigned int& y, unsigned int& z);
	string getInterval(unsigned int cost);
	wstring getwInterval(wchar_t c, unsigned int b = 0);

private:
	wstring m_str1;
	wstring m_str2;
	unsigned int m_xLength;
	unsigned int m_yLength;
	unsigned int** m_matrix;
	unsigned int** m_opMatrix;
};
#endif
