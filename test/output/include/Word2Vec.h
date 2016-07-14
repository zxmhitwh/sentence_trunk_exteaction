#ifndef __WORD_TO_VEC_H__
#define __WORD_TO_VEC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
//#include "MySeg.h"
#include "mt_seg.h"
using namespace std;

class word2vec
{
public:
	~word2vec();
	
    bool LoadModel(const char* model_path);
    void DestroyModel();
	
    long long  getwords();    //返回总词数
	long long  getsize();     //返回向量维度
	
    static word2vec* GetInstance();
	
    bool getWordVector(string word, float wordvec[]);
    bool getWordVector(vector<WORD_POS>& words, vector<float>& wordvec, int type = 0);
    bool getAllWordVectors(vector<WORD_POS>& words, vector<float*>& wordvec);

private:
	
    word2vec();
    
    float* M;
    Sdict_build* term_dict;
    long long words;
    long long size;
	
    static word2vec* Instance;	
};

#endif
