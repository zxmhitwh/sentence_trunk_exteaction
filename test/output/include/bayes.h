#ifndef __BAYES_H__
#define __BAYES_H__

#include <map>
#include <iostream>
#include <string>
#include <vector>
//#include "ext_string.h"
//#include "MySeg.h"
#include "mt_seg.h"
#include "UtilsLib.h" 
using namespace std;

#define MAX_MODEL_NUM 5
#define SCALING 0
#define DEFAULT_LABEL "NULL"


class Bayes
{
public:
    static Bayes* GetInstance();
    bool loadModel(const int SERIAL,const char* File);
    
    string Predict(const int SERIAL,vector<string>& statement_seg);
    string Predict(const int SERIAL,vector<WORD_POS>& statement_seg);

private:

    Bayes();
    static Bayes* instance;
    map<string,map<string,float> > WordsProbability[MAX_MODEL_NUM];
    int class_num[MAX_MODEL_NUM];
    int scaling_threshlod[MAX_MODEL_NUM];
    int default_prob[MAX_MODEL_NUM];
    map<string,float> PriorProbability[MAX_MODEL_NUM];
};

#endif
