#include <iostream>
#include "KeyWordExtract.h"
#include "time.h"
#include <sys/time.h>
using namespace std;
int MD5 =1;
int main(int argc,char **argv)
{
    config cfg;
    if(argc != 2)
        return 1;
    cfg.load_config(argv[1]);
    if(!(senming::processor::Init(cfg)))
    {
        return -1;
    }
    if(!KeyWordExtract::Init("./chunk/"))
    {
        return -1;
    }
    senming::processor proc;
    KeyWordExtract ke;
    string line;
    map<string,int> words;
    while(getline(cin,line))
    {
        struct timeval start, end;
        gettimeofday( &start, NULL );
        senming::comment_t com;
        com.text = xstr::strip(line);
        proc.process(com,false);
        ke.Extract(com);
        for(size_t i = 0;i < com.sentence.size();++i)
        {
            vector<int>& heads = com.sentence[i].heads;
            vector<string>& deprels = com.sentence[i].deprels;;
            if(heads.size() == 0)
            {
                continue;
            }
            for(size_t j = 0;j < com.sentence[i].term_pre.size();++j)
            {
                cout << com.sentence[i].term_pre[j].text << "/" << com.sentence[i].term_pre[j].is_chunk << "\t";
            }
            cout << endl << com.text << endl;;
        }
        //for(int i=0;i<10000000;i++);
        gettimeofday( &end, NULL );
        int timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec -start.tv_usec;
        //for(int i=0;i<10000;i++)
        //    cout<<1<<endl;
        cout<<"time:"<<timeuse<<"us"<<endl;
    }
    return 0;
}



