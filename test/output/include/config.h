#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include "UtilsLib.h"


using namespace std;

class config
{
 	map<string, string> _cfg_items;
public:
	config();
        ~config();
        /**
         * 读取配置文件
         */
        int load_config(const char* cfg_file);
	/**
         * 配置项setter
         */
        int set_config(const string &cfg_key, const string &cfg_val, bool overwirte = true);

        /**
         * 配置项getter模版
         */
        template <class T>
            int get_config(const string &cfg_key, T &cfg_val)
            {
                if (_cfg_items.end() == _cfg_items.find(cfg_key))
                {
                    return -1;
                }
                else
                {
                    string &val = _cfg_items[cfg_key];
                    if (!xstr::parse(val, cfg_val))
                    {
                        return -1;
                    }
                    else
                    {
                        return 0;
                    }
                }
            }
        /**
         * 清除全部配置
         */
        void clear_all_config();
};
#endif
