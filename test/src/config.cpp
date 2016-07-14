/**
 * @file senming_config.cpp
 * @brief 配置文件管理类
 */
#include "config.h"

config::config()
{
};

config::~config()
{
};
    
int config::load_config(const char *cfg_file)
{
	ifstream fin(cfg_file);
        if (!fin.good())
        {
            cout<<"Failed to open config file"<<endl;
            return -1;
        }

        string line;
        while(getline(fin, line))
        {
            line = xstr::strip(line);
            if (line == "" || line[0] == '#') 
                continue;
            vector<string> kv;
            xstr::split(line, '=',  kv);
            if (2 != kv.size())
            {
                cout<<"Format error of config line"<<endl;;
                continue;
            }
            string key = xstr::strip(kv[0]);
            string val = xstr::strip(kv[1]);
            if ("" == key || "" == val)
            {
                cout<<"Format error of config line"<<endl;
                continue;
            }
            if (_cfg_items.find(key) == _cfg_items.end())
            {
                _cfg_items[key] = val;    
            }
        }

        fin.close();
        return 0;
    };

    int config::set_config(const string &cfg_key, const string &cfg_val, bool overwrite)
    {
        if (cfg_key == "" || cfg_val == "")
        {
            cout<<"Set config failed: key or value is empty"<<endl;
            return -1;
        }
        if (!overwrite && _cfg_items.find(cfg_key) != _cfg_items.end())
        {
            cout<<"Set global config failed: overwrite not allowed"<<endl;
            return -1;
        }
        _cfg_items[cfg_key] = cfg_val;
        return 0;
    };

    void config::clear_all_config()
    {
        _cfg_items.clear();
    };
