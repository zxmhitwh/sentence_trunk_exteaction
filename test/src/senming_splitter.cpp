/**
 * @file senming_splitter.cpp
 * @brief 使用标点分句类
 */
#include <senming_splitter.h>


const int SENTENCE_MAX_LEN = 10000;
dm_dict_t * splitter::_dm_dict = NULL;


    bool is_decim(char c)
    {
        return (c >= '0' && c <= '9');
    }
    int append_dm_dict(const char * path, dm_dict_t *dm_dict, int set_prop);
using namespace senming;
splitter::splitter(): _dm_pack(NULL)
{

    _dm_pack = dm_pack_create(DEF_DM_PACK_SIZE);
    if (NULL == _dm_pack)
    {
        cout<<"fail  split _dm_pack"<<endl;
    }
}

dm_dict_t* load_dm_dict(const char * file, int size, int set_prop)
{
    ifstream is(file);
    if (!is.good())
    {
        return NULL;
    }

    dm_dict_t *dm_dict = dm_dict_create(size);
    if (NULL == dm_dict)
    {
        return NULL;
    }

    string line;
    while (getline(is, line))
    {
        line = xstr::strip(line, "\r\n");
        if (line == "" || line[0] == '#')
        {
            continue;
        }
        string text;
        int prop = 0;
        if (set_prop < 0)
        {
            vector<string> items;
            xstr::split(line, '\t', items);
            if (items.size() != 2 || items[0].empty()
                    || !xstr::parse(items[1], prop))
            {
                continue;
            }
            text = items[0];
        }
        else
        {
            text = line;
            prop = set_prop;
        }
        dm_lemma_t lemma;
        lemma.pstr = const_cast<char *>(text.c_str());
        lemma.len = text.size();
        lemma.prop = prop;
        dm_add_lemma(dm_dict, &lemma);
    }

    return dm_dict;

}



splitter::~splitter()
{
    if (NULL != _dm_pack)
    {
        dm_pack_del(_dm_pack);
        _dm_pack = NULL;
    }
}

void splitter::destroy()
{
    if (NULL != _dm_dict)
    {
        dm_dict_del(_dm_dict);
        _dm_dict = NULL;
    }
}

int splitter::init(config &cfg)
{
    string split_word_file;
    cfg.get_config("split-words", split_word_file);
    _dm_dict = load_dm_dict(split_word_file.c_str(), DEF_DM_DICT_SIZE, SPLIT_SEG_WORD);
    if (NULL == _dm_dict)
    {
        return -1;
    }
    string filter_word_file;
    if (0 == cfg.get_config("filter-words", filter_word_file))
    {
        if (0 != append_dm_dict(filter_word_file.c_str(), _dm_dict, SPLIT_FILT_WORD))
        {
            return -1;
        }
    }        
    return 0;
}

    int append_dm_dict(const char * path, dm_dict_t *dm_dict, int set_prop)
    {

        ifstream is(path);
        if (!is.good())
        {
            return -1;
        }

        if (NULL == dm_dict)
        {
            return -1;
        }

        string line;
        while (getline(is, line))
        {
            line = xstr::strip(line, "\r\n");
            if (line == "" || line[0] == '#')
            {
                continue;
            }
            string text;
            int prop = 0;
            if (set_prop < 0)
            {
                vector<string> items; 
                xstr::split(line, '\t', items);
                if (items.size() != 2 || !items[0].empty()
                        || !xstr::parse(items[1], prop))
                {
                    continue;
                }
                text = items[0];
            }
            else
            {
                text = line;
                prop = set_prop;
            }
            dm_lemma_t lemma;
            lemma.pstr = const_cast<char *>(text.c_str());
            lemma.len = text.size();
            lemma.prop = prop;
            dm_add_lemma(dm_dict, &lemma);
        }

        return 0;
    }
int splitter::split(comment_t &comment)
{
    char sent_buff[SENTENCE_MAX_LEN];
    string &line = comment.text;
    codetool.ConvertUtf8ToGB(line);
    vector<sentence_t> &sentence_list = comment.sentence;
    if (0 != dm_search(_dm_dict, _dm_pack, line.c_str(), line.size(), DM_OUT_ALL))
    {   
        return -1;
    }
    int spos = 0, epos = 0;
    bool discard = false;
    for (int i = 0; i < (int)_dm_pack->ppseg_cnt; i++)
    {
        int prop = _dm_pack->ppseg[i]->prop;
        // 区分标点和过滤词
        if (prop == SPLIT_FILT_WORD)
        {
            discard = true;
            continue;
        }
        else if (prop == SPLIT_SEG_WORD)
        {
            epos = _dm_pack->poff[i];
        }
        // 标点为.时，特殊处理
        if (strcmp(_dm_pack->ppseg[i]->pstr, ".") == 0)
        {
            if (epos > 0 && epos < (int)line.size()
                    && is_decim(line[epos-1]) 
                    && is_decim(line[epos+1]))
            {
                continue;
            }
        }
        if (spos < epos && !discard)
        {
            string sent_str(line.begin() + spos, line.begin() + epos);
            sent_str =  sent_str + _dm_pack->ppseg[i]->pstr;
            //cout<<"src"<<sent_str<<endl;
            if ((int)sent_str.size() >= SENTENCE_MAX_LEN)
            {
                sent_str = sent_str.substr(0, SENTENCE_MAX_LEN);
            }
            ul_trans2bj(sent_str.c_str(), sent_buff);
            ul_trans2bj_ext(sent_buff, sent_buff);
            ul_gb18030_gbi_str((unsigned char*)sent_buff, (unsigned char*)sent_buff, 0);
            sentence_t sentence;
            sentence.text = sent_buff;
        codetool.ConvertGBToUtf8(sentence.text);
            sentence_list.push_back(sentence);
        }
        spos = epos + _dm_pack->ppseg[i]->len;
        discard = false;
    }
    if (spos < (int)line.size() && !discard)
    {
        string sent_str(line.begin() + spos, line.end());
        if ((int)sent_str.size() >= SENTENCE_MAX_LEN)
        {
            sent_str = sent_str.substr(0, SENTENCE_MAX_LEN);
        }
        ul_trans2bj(sent_str.c_str(), sent_buff);
        ul_trans2bj_ext(sent_buff, sent_buff);
        sentence_t sentence;
        sentence.text = sent_buff;
        codetool.ConvertGBToUtf8(sentence.text);
        sentence_list.push_back(sentence);
    }

    return 0;
}

