#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include "segment_dll.h"
#include "parser_dll.h"
#include "postag_dll.h"
#include <vector>
#include <map>
#include <sstream>
#include "UtilsLib.h"
#include "KeyWordExtract.h"
using namespace std;

#define WINDOW_WITH 3

map<string,int> KeyWordExtract::mWord2Cluster = map<string,int>();
Sdict_build* KeyWordExtract::weight_dict = NULL;

static set<string> g_vWordSet ;//= etistring>();
//static set<string> g_vDepSet = vector<string>();

bool KeyWordExtract::Init(const string Path)
{
    //聚类词典
    string line;
    ifstream infile((Path+"/sample_label").c_str());
    if(!infile.is_open())
    {
        cout << "open sample_label error" << endl;
        return false;
    }
    while(getline(infile,line))
    {
        vector<string> item;
        PartitionString(line,item,(char*)" ");
        if(item.size() < 2)
            continue;
        mWord2Cluster[item[0]] = atoi(item[1].c_str());
    }
    infile.close();
    //权重词典
    ifstream infile2((Path+"/weight.txt").c_str());
    if(!infile2.is_open())
    {
        cout << "open weight error" << endl;
        return false;
    }
    weight_dict = db_creat(2000000,1);
    if(NULL == weight_dict)
    {
        return false;
    }
    Sdict_snode snode;
    while(getline(infile2,line))
    {
        vector<string> item;
        PartitionString(line,item,(char*)"\t");
        if(item.size() < 2)
            continue;
        //退款_word_0,v_pos_0,
        vector<string> tok;
        PartitionString(item[0],tok,(char*)"_");
        if(tok.size() <= 3)
        {
            if(tok[1] == "word")
                g_vWordSet.insert(tok[0]);
            //if(tok[1] == "father" || tok[1] == "son")
            //    g_vDepSet.insert(tok[0]);
        }
        //
        char c[20] = {0};
        strcpy(c,item[1].c_str());
        double value = strtod(c,NULL);
        //权重浮点值转成整形，少许精度丢失，负数也转换为无符号型
        unsigned int ivalue = (unsigned int)(1000000*value);
        if(ivalue != 0)
        {
            creat_sign_f64((char*)(item[0].c_str()), item[0].size(), &(snode.sign1), &(snode.sign2));
            snode.code = -1;
            snode.other = 0;
            db_op1(weight_dict, &snode, ADD);
            snode.other = ivalue;
            db_op1(weight_dict, &snode, MOD);
        }
    }
    return true;
}

void KeyWordExtract::UnInit()
{
    if(NULL != weight_dict)
    {
        db_del(weight_dict);
        weight_dict = NULL;
    }
}

float GetKeyValue(Sdict_build* dict,string& key)
{
    if(NULL == dict || key == "")
    {
        return 0.0f;
    }
    Sdict_snode snode;
    creat_sign_f64((char*)(key.c_str()),key.size(),&(snode.sign1),&(snode.sign2));
    db_op1(dict, &snode, SEEK);
    if(-1 == snode.code)
    {
        return 0.0f;
    }
    int ivalue = (int)(snode.other);
    return (float)(ivalue)/1000000;
}

bool KeyWordExtract::Extract(senming::comment_t& com)
{
    if(com.sentence.size() == 0)
        return false;
    //dep parse结果
    vector<int>& heads = com.sentence[0].heads;
    vector<string>& deprels = com.sentence[0].deprels;;
    if(heads.size() != com.sentence[0].term_pre.size() 
            || deprels.size() != com.sentence[0].term_pre.size())
        return false;
    vector<int> is_word;
    vector<int> is_cluster;
    vector<string> parent_edge;
    //vector<int> parent_node;
    /*
    map<string,int>::const_iterator it;
    for(it=mWord2Cluster.begin();it!=mWord2Cluster.end();it++)
        cout<<it->first<<it->second<<endl;
    */
    int len = com.sentence[0].term_pre.size();
    vector<vector<string> > child_edge(len);
    //cout<<g_vWordSet.size()<<endl;
    for(size_t i=0;i<com.sentence[0].term_pre.size();i++)
    {
        set<string>::iterator it;
        map<string,int>::iterator it1;
        it = g_vWordSet.find(com.sentence[0].term_pre[i].text);
        if(it!=g_vWordSet.end())
            is_word.push_back(1);
        else
            is_word.push_back(0);
        it1 = mWord2Cluster.find(com.sentence[0].term_pre[i].text);
        if(it1!=mWord2Cluster.end())
            is_cluster.push_back(it1->second);
        else
            is_cluster.push_back(9999);
        parent_edge.push_back(com.sentence[0].deprels[i]);
        if(com.sentence[0].heads[i]!=0)
        {
            child_edge[com.sentence[0].heads[i]-1].push_back(com.sentence[0].deprels[i]);
        }
    }
    /*
    for(int i=0;i<is_word.size();i++)
        cout<<is_word[i];
    cout<<endl;
    */
    string cluster;
    for(size_t j = 0;j < com.sentence[0].term_pre.size();++j)
    {
        vector<string> vsKeys;
        string& word = com.sentence[0].term_pre[j].text;
        if(is_word[j]!=0)
        {
            vsKeys.push_back(word+"_word_0");
        }
        string& pos = com.sentence[0].term_pre[j].postag;
        vsKeys.push_back(pos+"_pos_0");
        //cout<<is_cluster[j]<<endl;
        if(is_cluster[j]!=9999)
        {
            stringstream sstr;
            sstr<<is_cluster[j];
            sstr>>cluster;
            vsKeys.push_back(cluster+"_cluster_0");
        }
        if(j==1)
        {
            if(is_word[0]!=0)
                vsKeys.push_back(com.sentence[0].term_pre[0].text+"_word_-1");
            vsKeys.push_back(com.sentence[0].term_pre[0].postag+"_pos_-1");
            if(is_cluster[0]!=9999)
            {
                stringstream sstr;
                sstr<<is_cluster[0];
                sstr>>cluster;
                vsKeys.push_back(cluster+"_cluster_-1");
            }
        }
        if(j>=2)
        {
            if(is_word[j-2]!=0)
                vsKeys.push_back(com.sentence[0].term_pre[j-2].text+"_word_-2");
            vsKeys.push_back(com.sentence[0].term_pre[j-2].postag+"_pos_-2");
            if(is_cluster[j-2]!=9999)
            {
                stringstream sstr;
                sstr<<is_cluster[j-2];
                sstr>>cluster;
                vsKeys.push_back(cluster+"_cluster_-2");
            }
            if(is_word[j-1]!=0)
                vsKeys.push_back(com.sentence[0].term_pre[j-1].text+"_word_-1");
            vsKeys.push_back(com.sentence[0].term_pre[j-1].postag+"_pos_-1");
            if(is_cluster[0]!=9999)
            {
                stringstream sstr;
                sstr<<is_cluster[j-1];
                sstr>>cluster;
                vsKeys.push_back(cluster+"_cluster_-1");
            }
        }
        if(j==com.sentence[0].term_pre.size()-2)
        {
            if(is_word[j+1]!=0)
                vsKeys.push_back(com.sentence[0].term_pre[j+1].text+"_word_1");
            vsKeys.push_back(com.sentence[0].term_pre[j+1].postag+"_pos_1");
            if(is_cluster[j+1]!=9999)
            {
                stringstream sstr;
                sstr<<is_cluster[j+1];
                sstr>>cluster;
                vsKeys.push_back(cluster+"_cluster_1");
            }
        }
        if(j<=com.sentence[0].term_pre.size()-3&&com.sentence[0].term_pre.size()>=3)
        {
            if(is_word[j+1]!=0)
                vsKeys.push_back(com.sentence[0].term_pre[j+1].text+"_word_1");
            vsKeys.push_back(com.sentence[0].term_pre[j+1].postag+"_pos_1");
            if(is_cluster[j+1]!=9999)
            {
                stringstream sstr;
                sstr<<is_cluster[j+1];
                sstr>>cluster;
                vsKeys.push_back(cluster+"_cluster_1");
            }
            if(is_word[j+2]!=0)
                vsKeys.push_back(com.sentence[0].term_pre[j+2].text+"_word_2");
            vsKeys.push_back(com.sentence[0].term_pre[j+2].postag+"_pos_2");
            if(is_cluster[j+2]!=9999)
            {
                stringstream sstr;
                sstr<<is_cluster[j+2];
                sstr>>cluster;
                vsKeys.push_back(cluster+"_cluster_2");
            }
        }
        vsKeys.push_back(parent_edge[j]+"_parent_edge_0");
        if(!child_edge[j].empty())
            for(int k=0;k<child_edge[j].size();k++)
                vsKeys.push_back(child_edge[j][k]+"_child_edge_0");
        if(com.sentence[0].heads[j]!=0)
        {
            int head = com.sentence[0].heads[j]-1;
            if(is_word[head]!=0)
                vsKeys.push_back(com.sentence[0].term_pre[head].text+"_word_parent_node_0");
            vsKeys.push_back(com.sentence[0].term_pre[head].postag+"_pos_parent_node_0");
            if(is_cluster[head]!=9999)
            {
                stringstream sstr;
                sstr<<is_cluster[head];
                sstr>>cluster;
                vsKeys.push_back(cluster+"_cluster_parent_node_0");
            }
        }
        if(j>=1)
        {
            vsKeys.push_back(parent_edge[j-1]+"_parent_edge_-1");
            if(!child_edge[j-1].empty())
            for(int k=0;k<child_edge[j-1].size();k++)
                vsKeys.push_back(child_edge[j-1][k]+"_child_edge_-1");
            if(com.sentence[0].heads[j-1]!=0)
            {
                int head = com.sentence[0].heads[j-1]-1;
                if(is_word[head]!=0)
                    vsKeys.push_back(com.sentence[0].term_pre[head].text+"_word_parent_node_-1");
                vsKeys.push_back(com.sentence[0].term_pre[head].postag+"_pos_parent_node_-1");
                if(is_cluster[head]!=9999)
                {
                    stringstream sstr;
                    sstr<<is_cluster[head];
                    sstr>>cluster;
                    vsKeys.push_back(cluster+"_cluster_parent_node_-1");
                }
            }
        }
        if(j<=com.sentence[0].term_pre.size()-2&&com.sentence[0].term_pre.size()>=2)
        {
            vsKeys.push_back(parent_edge[j+1]+"_parent_edge_1");
            if(!child_edge[j+1].empty())
                for(int k=0;k<child_edge[j+1].size();k++)
                    vsKeys.push_back(child_edge[j+1][k]+"_child_edge_1");
            if(com.sentence[0].heads[j+1]!=0)
            {
                int head = com.sentence[0].heads[j+1]-1;
                if(is_word[head]!=0)
                    vsKeys.push_back(com.sentence[0].term_pre[head].text+"_word_parent_node_1");
                vsKeys.push_back(com.sentence[0].term_pre[head].postag+"_pos_parent_node_1");
                if(is_cluster[head]!=9999)
                {
                    stringstream sstr;
                    sstr<<is_cluster[head];
                    sstr>>cluster;
                    vsKeys.push_back(cluster+"_cluster_parent_node_1");
                }
            }
        }
        /*
        cout<<word<<" ";
        for(int i=0;i<vsKeys.size();i++)
            cout<<vsKeys[i]<<" ";
        cout<<endl;
        */
        ////////////////////////////////////////////////////////
        //获取模型权重各维度key
        //vsKeys.push_back("退款_word_0");
        
        ///////////////////////////////////////////////////////
        //基于key查找权重
        float sum = 0.0f;
        for(size_t k = 0;k < vsKeys.size();++k)
        {
            sum += GetKeyValue(weight_dict,vsKeys[k]);
        }
        if(sum < 0.0f)
            com.sentence[0].term_pre[j].is_chunk = false;
    }

    return true;
}


