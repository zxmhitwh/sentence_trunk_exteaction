#ifndef __STOP_WORD_H__
#define __STOP_WORD_H__

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <fstream>
#include <vector>
//#include <MySeg.h>
#include "mt_seg.h"
#include "UtilsLib.h"

using namespace std;
#define MAX_WORDS_LEN 1024

class StopWord
{
private:
	StopWord();
	static StopWord* instance;
	map<string,int> stopword;
public:
	static StopWord* GetInstance();
	/*加载停用词词典
        [in]  Filename  词典路径
	 词典加载为成员 stopword中
 	*/
	bool LoadStopWord(const char* Filename);
	/*停用词过滤
  	[in] seg_line  代过滤的句子
	将含有停用词的项去除
	*/
	void FilterStopWord(vector<WORD_POS> &seg_line);
};

#endif
