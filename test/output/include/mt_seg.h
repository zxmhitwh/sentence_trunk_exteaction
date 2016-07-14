
#ifndef __MY_SEG_H__
#define __MY_SEG_H__

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "config.h"
#include "processor.h"

using namespace std;
        
const int MAX_LENGTH = 8192;
const int WINDOW_WIDTH = 3;

//切分返回结果结构体
struct WORD_POS
{
	string word;       //切分单词
	float weight;	   //单词权重
	//int weight;	   //单词权重
	bool operator == ( WORD_POS &w)     //运算符重载	
	{
		return w.word == word && w.weight == weight;	
	}
    WORD_POS(string s,float w)
    {
        word = s;
        weight = w;
    }
};
//单例实现分词类，词典加载一次
class myseg
{
private:
    static bool bSegHasInit;
    senming::processor proc;
    static set<string> imp_words;

public:
    
    static bool Init(config &cfg);
    static void UnInit();
    static float GetTermWeight(string& term,string& pos);

    int Segmentation(string& text, vector<WORD_POS> &seg_res);
	int Segmentation(string& text, vector<string> &seg_res);
	int SegmentationAll(string& text, vector<string> &seg_res);
    int SegmentationAll(string& text, vector<WORD_POS> &seg_res);
    bool IsPhoneNum(string& text);
};
ostream& operator<<(ostream& os, const vector<WORD_POS>& m);
#endif

