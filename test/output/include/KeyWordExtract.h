#ifndef  __KEY_WORD_EXTRACT__H__
#define  __KEY_WORD_EXTRACT__H__

#include <iostream>
#include <string>
#include "UtilsLib.h"
#include "processor.h"
using namespace std;

class KeyWordExtract
{
public:
    
    static bool Init(const string Path);
    static void UnInit();
    //seg\pos\parse
    static bool Extract(senming::comment_t& comment);

private:
    //static
    static map<string,int> mWord2Cluster;
    //static map<string,int> mWord2ID;
    static map<string,double> mFeaWeight;
};
#endif


