
#include "code_conversion.h"



CodeConversionTool::CodeConversionTool()
{
	;
}

CodeConversionTool::~CodeConversionTool()
{
	;
}

bool CodeConversionTool::ConvertGBToUtf8(string& str)
{
    wstring ws = CCodeConversion::GBToUnicode(str);
    str = CCodeConversion::UnicodeToUTF8(ws);
    return true;
}
bool CodeConversionTool::ConvertUtf8ToGB(string& str)
{
    wstring ws = CCodeConversion::UTF8ToUnicode(str);
    str = CCodeConversion::UnicodeToGB(ws);
    return true;
}




