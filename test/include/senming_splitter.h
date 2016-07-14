/**
 * @file senming_splitter.h
 * @brief 使用标点分句类
 */
#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include "ul_dictmatch.h"
#include "ul_ccode.h"
//#include "senming_def.h"
//#include "senming_tool.h"
//include "senming_config.h"

#include"processor.h"
#include "config.h"

using namespace std;
class splitter
{
    static dm_dict_t *_dm_dict;
    dm_pack_t *_dm_pack;
    CodeConversionTool codetool;

public:

    splitter();

    ~splitter();

    static int init(config &cfg);

    int split(senming::comment_t &comment);

    static void destroy();

};

