#ifndef _DIRTY_FILTER_H_
#define _DIRTY_FILTER_H_

#include <iostream>
#include <string>
#include <vector>
#include "UtilsLib.h"
#include "pattern_parser.h"
#include "ul_log.h"
#include "config.h"
using namespace std;


class DirtyFilter 
{
public:
    
    static bool Init(string& conf);
    static void Uninit();

    static void filter_dirty(string& str);

private:
    static NLP::pattern_parser parser;
};

#endif


