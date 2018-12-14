#ifndef __WORDMGR_H__
#define __WORDMGR_H__

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "zhWord.h"
#include "util.h"

using namespace std;

class wordMgr;

class wordMgr
{
public:
  wordMgr() { init(); }
  ~wordMgr() { reset(); }

  bool readFile(string);
  bool printMap(string);

private:
  void init() {}
  void reset();

  map<string,vector<zhWord>*> _zhuYinMap;
};

bool wordMgr::readFile(string fileName)
{
  fstream file;
  file.open(fileName.c_str(),ios::in);
  if(!file.is_open())
    return false;

  string str = "";
  while(getline(file,str)){
    if(str.empty())
      continue;
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end()); //remove all spaces
    string wordInfo = str.substr(0,2);
    str.erase(0,2);
    vector<string> zhuYins;
    lexOptions(str,zhuYins);

    zhWord word(wordInfo,zhuYins);
    vector<string> heads = word.getZhuYinHead();
    for(int i = 0; i < heads.size(); ++i){
      map<string,vector<zhWord>*>::iterator it = _zhuYinMap.find(heads[i]);
      if(it != _zhuYinMap.end())
        it -> second -> push_back(word);
      else{
        vector<zhWord>* list = new vector<zhWord>;
        list -> push_back(word);
        _zhuYinMap[heads[i]] = list;
      }
    }
  }
  file.close();
  return true;
}

bool wordMgr::printMap(string fileName)
{
  fstream file;
  file.open(fileName.c_str(),ios::out);
  if(!file.is_open())
    return false;

  set<string> usedWords;
  for(map<string,vector<zhWord>*>::iterator it = _zhuYinMap.begin(); it != _zhuYinMap.end(); ++it){
    file << it -> first << " ";
    vector<zhWord>& list = *(it -> second);
    for(int i = 0; i < list.size();){
      file << list[i].getWord();
      if(++i != list.size())
        file << " ";
    }
    file << endl;
    for(int i = 0; i < list.size(); ++i){
      pair<set<string>::iterator,bool> ret = usedWords.insert(list[i].getWord());
      if(ret.second)
        file << list[i].getWord() << " " << list[i].getWord() << endl;
    }
  }
  file.close();
  return true;
}

void wordMgr::reset()
{
  for(map<string,vector<zhWord>*>::iterator it = _zhuYinMap.begin(); it != _zhuYinMap.end(); ++it)
    delete it -> second;
}

#endif