/**
 * @file processor.cpp
 * @brief 句子处理类，封装切词、词性标注、句法分析
 */

#include "processor.h"

namespace senming
{

int processor::use_parser = 1;
int processor::use_wordseg = 1;
bool processor::bProcessHasInit = false;
void* processor::engine = NULL;

dm_dict_t* processor::_dm_dict = NULL;

vector<pair<string, int> > processor::_comb_rule_vec = vector<pair<string, int> >();
map<string, string> processor::_pos_rule_map = map<string, string>();
map<string, string> processor::_postag_map = map<string, string>();

processor::processor()
{
    if(1 == use_wordseg)
    {
        // 细粒度：TC_GU
        int seg_model = TC_U2L|TC_S2D|TC_T2S|TC_LGU|TC_SGU|TC_CONV|TC_WGU|TC_POS|TC_USR;
        seghandle = old_seg::TCCreateSegHandle(seg_model);
    }
    _dm_pack = NULL;
    _dm_pack = dm_pack_create(DEF_DM_PACK_SIZE);
    if(_dm_pack == NULL)
        SENMING_LOG_ERROR("failed to create dm_pack");
}
processor::~processor()
{
    old_seg::TCCloseSegHandle(seghandle); //释放handle
    if (NULL != _dm_pack)
    {
        dm_pack_del(_dm_pack);
        _dm_pack = NULL;
    }
}

void processor::UnInit()
{
    if(1 == use_wordseg)
    {
        old_seg::TCUnInitSeg();               //释放分词资源
    }
    if(1 == use_parser)
    {
        if (NULL != engine) {
            parser_release_parser(engine);
        }
    }
    bProcessHasInit = false;
    if (NULL != _dm_dict)
    {
        dm_dict_del(_dm_dict);
        _dm_dict = NULL;
    }
}

bool processor::Init(config &cfg)
{
    if(bProcessHasInit)
    {
        return true;
    }
    string rule_path, dict_path;
    int ret = 0;
    cfg.get_config("rule-path", rule_path);
    cfg.get_config("dict-path", dict_path);
    cfg.get_config("use-wordseg",use_wordseg);
    cfg.get_config("use-parser",use_parser);
    if(1 == use_wordseg)
    {
        string wordseg_dict_path_mt;
        cfg.get_config("wordseg-dict-mt", wordseg_dict_path_mt);

        if (!old_seg::TCInitSeg((dict_path + wordseg_dict_path_mt).c_str())) {
            printf("failed to load mt worddict:%s\n", wordseg_dict_path_mt.c_str());
            UnInit();
            return false;        
        }
    }
    if(1 == use_parser)
    {
        //load ltp parser
        string parser_conf_path_ltp;
        cfg.get_config("parser-model-ltp", parser_conf_path_ltp);
        engine = parser_create_parser((dict_path + parser_conf_path_ltp).c_str());
        if (!engine) 
        {
            printf("failed to create parser thread model");
            UnInit();
            return false;
        }

    }
    //切词词典
    string split_word_file;
    cfg.get_config("split-words", split_word_file);
    _dm_dict = load_dm_dict(split_word_file.c_str(), DEF_DM_DICT_SIZE, SPLIT_SEG_WORD);
    if (NULL == _dm_dict)
    {
        return false;
    }
    string postag_rule_file;
    cfg.get_config("mt_postag-rules", postag_rule_file);
    ret = read_postag_rules(rule_path+postag_rule_file);
    if (ret != 0)
    {
        UnInit();
        return false;
    }
    string combine_rule_file;
    cfg.get_config("mt_combine-rules", combine_rule_file);
    ret = read_combine_rules(rule_path+combine_rule_file);
    if (ret != 0)
    {
        UnInit();
        return false;
    }

    string postag_mapping_file;
    cfg.get_config("postag-mapping", postag_mapping_file);
    ret = read_postag_mapping(rule_path + postag_mapping_file);
    if (ret != 0) {
        UnInit();
        return false;
    } 

    bProcessHasInit = true;
    return true;
}

dm_dict_t* processor::load_dm_dict(const char *p, int size, int set_prop)
{
    ifstream is(p);
    if (!is.good())
    {
        SENMING_LOG_ERROR("bad input stream");
        return NULL;
    }

    dm_dict_t *dm_dict = dm_dict_create(size);
    if (NULL == dm_dict)
    {
        SENMING_LOG_ERROR("failed to create dm_dict of size %d", size);
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
                SENMING_LOG_ERROR("format error of line: %s", line.c_str());
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
int processor::read_postag_rules(const string file)
{
    ifstream fin(file.c_str());
    if (!fin.good())
    {
        printf("failed to open pos rule file: %s", file.c_str());
        return -1;
    }
    string line;
    while(getline(fin, line))
    {
        line = xstr::strip(line, "\r\n");
        if (line == "" || line[0] == '#')
        {
            continue;
        }
        vector<string> items;
        xstr::split(line, ':', items);
        if (items.size() != 2)
        {
            //KEFU_LOG_WARNING("format error of line: %s", line.c_str());
            continue;
        }
        _pos_rule_map[items[0]] = items[1];
    }
    fin.close();
    return 0;
}

int processor::read_postag_mapping(const string file)
{
    ifstream fin(file.c_str());
    if (!fin.good())
    {
        printf("failed to open postag_mapping file: %s", file.c_str());
        return -1;
    }
    string line;
    while(getline(fin, line))
    {
        line = xstr::strip(line);
        if (line == "" || line[0] == '#')
        {
            continue;
        }
        vector<string> items;
        xstr::split(line, '=', items);
        if (items[0] == "")
        {
            continue;
        }
        _postag_map[items[0]] = items[1];
    }
    fin.close();
    return 0;
}

int processor::read_combine_rules(const string file)
{
    ifstream fin(file.c_str());
    if (!fin.good())
    {
        printf("failed to open combine rule file: %s", file.c_str());
        return -1;
    }
    string line;
    while(getline(fin, line))
    {
        line = xstr::strip(line);
        if (line == "" || line[0] == '#')
        {
            continue;
        }
        vector<string> items;
        xstr::split(line, '\t', items);
        if (items[0] == "")
        {
            continue;
        }
        int chkphr = 0;
        if (items.size() >= 2)
        {
            xstr::parse(items[1], chkphr);
        }
        _comb_rule_vec.push_back(make_pair(items[0], chkphr));
    }
    fin.close();
    return 0;
}


int processor::postag(sentence_t &sentence)
{
    string& text = sentence.text;
    vector<term_t> &term_pre = sentence.term_pre;
    set<string> &phrase = sentence.phrase;


    size_t encoding = TC_UTF8;

    old_seg::TCSegment(seghandle, text.c_str(), text.length(), encoding);
    //得到分词的个数
    int rescount = old_seg::TCGetResultCnt(seghandle);
    old_seg::pWP wordpos;      //单个分词和词性
    char pos[16];
    int offset = 0;
    for (int i = 0; i < rescount; i++) {
        string word = old_seg::TCGetWordAt(seghandle, i);
        wordpos = old_seg::TCGetAt(seghandle, i);
        old_seg::TCPosId2Str(wordpos->pos, pos);
        offset += word.length();
        term_t aterm;
        aterm.text = word;
        aterm.offset = offset;
        aterm.postag = pos;
        correctPostag(aterm);
        //string correctPos = getPostagMappingResult(string(pos));
        //aterm.postag = correctPos;
        term_pre.push_back(aterm);
    }
    return 0;
}

int processor::correctPostag(term_t &aterm) {
    aterm.postag = getPostagMappingResult(aterm.postag);
    map<string, string>::iterator iter;
    iter = _pos_rule_map.find(aterm.text + "@" + aterm.postag);
    if (iter != _pos_rule_map.end()) {
        aterm.postag = iter->second;
    }
    return 0;
}

/*
   int processor::correct(sentence_t &sentence)
   {
   vector<term_t>& term_pre = sentence.term_pre;

   set<string>& phrase = sentence.phrase;
   string line_ptg;
   termvec_2_str(term_pre, line_ptg);
// 词性修正规则
line_ptg = string(" ") + line_ptg + string(" ");
for (int i = 0; i < (int)_pos_rule_vec.size(); i++)
{
xstr::replace(line_ptg, _pos_rule_vec[i].first, _pos_rule_vec[i].second);
}
vector<term_t>& term_vec = sentence.term_pro;
str_2_termvec(xstr::strip(line_ptg), term_vec);

// 名词合并规则
for (int i = 0; i < (int)_comb_rule_vec.size(); i++)
{
vector<term_t> rule_vec;
str_2_termvec(_comb_rule_vec[i].first, rule_vec);

for (int j = 0; j < (int)term_vec.size()-(int)rule_vec.size()+1; j++)
{
if (match_rule(term_vec, rule_vec, j))
{
term_t aterm;
aterm.postag = "n";
for (int k = 0; k < (int)rule_vec.size(); k++)
{
aterm.text += term_vec[j+k].text;
}
if (_comb_rule_vec[i].second <= 0 || phrase.find(aterm.text) != phrase.end())
{
term_vec[j] = aterm;
term_vec.erase(term_vec.begin()+j+1, term_vec.begin()+j+rule_vec.size());
j--;
}
}
}
}
return 0;
}

*/
int processor::parse(sentence_t &sentence)
{
    vector<term_t>& term_pre = sentence.term_pre;
    vector<string> words;
    vector<string> postags;

    for (int i = 0; i < (int)term_pre.size(); i++)
    {
        words.push_back(term_pre[i].text);
        postags.push_back(term_pre[i].postag);
        //cout << utf8_term << " ";
    }
    //cout << endl;

    vector<int>& heads = sentence.heads;
    vector<string>& deprels = sentence.deprels;;
    heads.clear();
    deprels.clear();
    parser_parse(engine, words, postags, heads, deprels);
    sentence.parse_rel.clear(); 
    for (size_t i = 0; i < heads.size(); i++) {
        sentence.parse_rel.insert(make_pair(make_pair(heads[i] - 1, i), deprels[i]));
    }
    return 0;
}


string processor::getPostagMappingResult(const string postag) {
    map<string, string>::iterator iter;
    iter = _postag_map.find(postag);
    if (iter == _postag_map.end()) {
        return postag;
    } else {
        string rs = iter->second;
        return rs;
    }
}

bool processor::match_rule(const vector<term_t> &term_vec, const vector<term_t> &rule_vec, int spos)
{
    int rule_size = rule_vec.size();
    int term_size = term_vec.size();
    if (spos + rule_size > term_size)
    {
        return false;
    }
    for (int i = 0; i < rule_size; i++)
    {
        if (rule_vec[i].postag != "?") {
            if (term_vec[i+spos].postag != rule_vec[i].postag)
            {
                return false;
            }
        }
        if (rule_vec[i].text != "?")
        {
            size_t str_size;
            if (xstr::parse(rule_vec[i].text, str_size))
            {
                //转换为数字成功，说明表示的是字符长度
                if (term_vec[i+spos].text.size() != str_size)
                {
                    return false;
                }
            }
            else
            {
                //转换数字失败，比较文本内容
                if (rule_vec[i].text != term_vec[i+spos].text)
                {   
                    return false;
                }
            }
        }
    }
    return true;
}



int str_2_termvec(const string &str, vector<term_t> &term_vec, char split_char)
{
    term_vec.clear();
    vector<string> term_str_vec;
    xstr::split(str, split_char, term_str_vec);
    int cur_pos = 0;
    for (int i = 0; i < (int)term_str_vec.size(); i++)
    {
        size_t pos = term_str_vec[i].find_last_of('@');
        if (pos == string::npos || pos == 0 || pos == term_str_vec[i].size()-1)
        {
            continue;
        }
        term_t aterm;
        aterm.text = term_str_vec[i].substr(0, pos);
        aterm.postag = term_str_vec[i].substr(pos+1);
        aterm.offset = cur_pos;
        term_vec.push_back(aterm);
        cur_pos += aterm.text.size();
    }
    return 0;
}


int termvec_2_str(const vector<term_t> &term_vec, string &str, char split_char)
{
    if (term_vec.size() == 0)
    {
        return -1;
    }
    str = term_vec[0].text + "@" + xstr::strip(term_vec[0].postag);
    for (int i = 1; i < (int)term_vec.size(); i++)
    {
        //str += string(1, split_char) + term_vec[i].text + "@" + term_vec[i].postag;
        str += string(1, split_char) + xstr::strip(term_vec[i].text) + "@" + xstr::strip(term_vec[i].postag);
        // str +=  term_vec[i].text ;//+ "@" + term_vec[i].postag;
    }
    // cout<<" termvec_2_str(const "<< str<<endl;

    return 0;
}

int processor::correctParse(sentence_t &sentence) {
    int combinCount = 0;
    // 句法合并规则 CMP ， ATT
    for (map<pair<int, int>, string>::iterator it = sentence.parse_rel.begin(); it != sentence.parse_rel.end(); ++it) {
        string parseTag = it->second;
        string combinPostag;

        int pos1 = it->first.first - combinCount;
        int pos2 = it->first.second - combinCount;
        if (pos1 > pos2) {
            pos1 = it->first.second - combinCount;
            pos2 = it->first.first - combinCount;
        }
        if ((pos2 - pos1) != 1) {
            continue;
        }

        vector<term_t>& term_vec = sentence.term_pre;
        if (it->second == "CMP") {
            combinPostag = "v";
        } else if (it->second == "ATT") {
            combinPostag = "n";
        } else {
            continue;
        }

        bool isCombine = false;
        for (int i = 0; i < (int)_comb_rule_vec.size(); i++) {
            vector<term_t> rule_vec;
            str_2_termvec(_comb_rule_vec[i].first, rule_vec);
            if (match_rule(term_vec, rule_vec, pos1)) {
                isCombine = true;
                break;
            }
        }
        if (!isCombine) {
            continue;
        }

        // 合并两个词
        term_t aterm;
        aterm.postag = combinPostag;
        aterm.text = term_vec[pos1].text + term_vec[pos2].text;

        term_vec[pos1] = aterm;
        term_vec.erase(term_vec.begin() + pos2);
        combinCount++;
        //        cout << "combine text\t" << aterm.text << "/" << it->second << endl;
    }
    sentence.parse_rel.clear();
}

bool processor::Utf8_2_GB(string& str)
{
    codetool.ConvertUtf8ToGB(str);
    return true;
}
bool is_decim(char c)
{
    return (c >= '0' && c <= '9');
}
void findPath(int index ,vector<vector<int> >&v, vector<vector<int> >&paths,vector<int>&p)
{
    if(v[index].size() == 0)
    {
        paths.push_back(p);
        return ;
    }
    for(int i = 0 ; i < v[index].size() ;i ++)
    {
        int j = v[index][i];
        p.push_back(j - 1);
        findPath(j,v,paths,p);
        p.pop_back();
    }
}

void createParsePath(sentence_t &sentence)
{
    map<pair<int,int>,string> & parse_rel = sentence.parse_rel;
    vector<vector<int> > v(sentence.term_pre.size() +1);
    for(map<pair<int,int>,string>::iterator itr = parse_rel.begin(); itr != parse_rel.end(); itr++)
    {
        int  last_id = itr->first.first + 1;
        int id = itr->first.second + 1;
        v[last_id].push_back(id);
    }
    vector<int> p;
    p.push_back(-1);
    findPath(0,v,sentence.parse_path,p);
    /* for(int i = 0 ; i < sentence.parse_path.size() ; i++)
       {
       cout<<"path"<< i<<":";
       for(int j = 0 ; j < sentence.parse_path[i].size() ; j++)
       {
       cout<<sentence.parse_path[i][j]<<" ";
       }
       cout<<endl;
       }*/
}

int processor::process(comment_t &comment,  bool isSplit) {

    string line = comment.text;
    if(line .size() >= 4096)
        line = line.substr(0,4095);
    char src[4096] = {0};
    if(!Utf8_2_GB(line))
        return -1;

    if (isSplit) {
        if( 0 != dm_search(_dm_dict,_dm_pack,line.c_str(),line.size(),DM_OUT_ALL))
            return -1;
        vector<sentence_t>& sentence_list = comment.sentence;
        sentence_list.clear();
        int spos = 0,epos = 0;
        bool discard = false;
        for(int i = 0 ; i < (int)_dm_pack->ppseg_cnt; i++)
        {
            int prop = _dm_pack->ppseg[i]->prop;
            //区分表达和过滤词
            if(prop == SPLIT_FILT_WORD)
            {
                discard = true;
                continue;
            }
            else if(prop == SPLIT_SEG_WORD)
                epos = _dm_pack->poff[i];

            if( strcmp(_dm_pack->ppseg[i]-> pstr,".") == 0)
            {
                if(epos > 0 && epos < (int)line.size()&&  is_decim(line[epos-1]) 
                        && is_decim(line[epos]))
                {
                    continue;
                }
            }
            if(spos < epos && !discard)
            {
                string sent_str(line.begin() + spos, line.begin() + epos);
                if ((int)sent_str.size() >= MAX_SENT_LEN)
                {
                    sent_str = sent_str.substr(0, MAX_SENT_LEN);
                }
                if(sent_str.size() < 4)
                {
                    continue;
                }
                //     memset(src,0,MAX_COMM_LEN);
                //      ul_trans2bj(sent_str.c_str(), src);
                //    ul_trans2bj_ext(src, src);
                //    ul_gb18030_gbi_str((unsigned char*)src, (unsigned char*)src, 0);
                sentence_t sentence;
                sentence.text = sent_str;
                codetool.ConvertGBToUtf8(sentence.text);
                sentence_list.push_back(sentence);
            }
            spos = epos + _dm_pack->ppseg[i]->len;
            discard = false;
        }
        if (spos < (int)line.size() && !discard)
        {
            string sent_str(line.begin() + spos, line.end());
            if ((int)sent_str.size() >= MAX_SENT_LEN)
            {
                sent_str = sent_str.substr(0, MAX_SENT_LEN);
            }
            if(sent_str.size() < 4)
            {
                return 0;
            }
            sentence_t sentence;
            codetool.ConvertGBToUtf8(sent_str);
            sentence.text = sent_str;
            sentence_list.push_back(sentence);
            //TODO: split sentence
            /*  sentence_t sent;
                sent.text = comment.text;
                comment.sentence.push_back(sent);*/
        }
    } else {
        sentence_t sent;
        sent.text = comment.text;
        comment.sentence.push_back(sent);
    }
    int ret = 0;
    //    cout<<" comment.sentence.size() "<< comment.sentence.size()<<endl;
    for (int i = 0; i < (int)comment.sentence.size(); i++) {
        ret += postag(comment.sentence[i]);
        //ret += correct(comment.sentence[i]);
        if (1 == use_parser) {
            ret += parse(comment.sentence[i]);
            // 句法合并规则加到句法分析之后处理
            //ret += correctParse(comment.sentence[i]);
            // 合并后再进行过一次句法分析
            //ret += parse(comment.sentence[i]);
        }
        createParsePath(comment.sentence[i]);
    }
    return ret;
}

};

