#ifndef __ZHWORD_H__
#define __ZHWORD_H__

#include <string>
#include <vector>

using namespace std;

class zhWord;

class zhWord
{
public:
  zhWord(string w,vector<string> z): _word(w), _zhuYin(z) {}
  string getWord() { return _word; }
  vector<string> getZhuYin() { return _zhuYin; }
  vector<string> getZhuYinHead();

private:
  string _word;
  vector<string> _zhuYin;

};

vector<string> zhWord::getZhuYinHead()
{
  vector<string> list;
  for(int i = 0; i < _zhuYin.size(); ++i){
    string head = _zhuYin[i].substr(0,2);
    list.push_back(head);
  }
  return list;
}

#endif