#ifndef __DABMGR_H__
#define __DABMGR_H__

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>
#include <cfloat>
#include "Ngram.h"
#include "util.h"

class dabMgr;

class dabMgr
{
public:
  dabMgr() { init(); }
  ~dabMgr() { reset(); }

  bool loadMap(string);
  bool loadLM(string);
  void disambig(string);
  vector<string> viterbi(vector<string>);

private:
  void init();
  void reset();
  double getUnigramProb(const char*);
  double getBigramProb(const char*,const char*);

  class deltaUnit
  {
  public:
    deltaUnit(string w,double p,deltaUnit* prev = 0): _word(w), _prob(p), _prev(prev) {}
    ~deltaUnit() {}
    string getWord() { return _word; }
    deltaUnit* getPrev() { return _prev; }
    double delta() { return _prob; }

  private:
    string _word;
    double _prob;
    deltaUnit* _prev;
  };

  map<string,vector<string> > _wordMap;
  int _nGram;
  Ngram* _lm;
  Vocab* _voc;
};

void dabMgr::init()
{
  _nGram = 2;
  _voc = new Vocab;
  _lm = new Ngram(*_voc,_nGram);
}

void dabMgr::reset()
{
  delete _voc;
  delete _lm;
}

bool dabMgr::loadMap(string fileName)
{
  fstream file;
  file.open(fileName.c_str(),ios::in);
  if(!file.is_open())
    return false;

  string str = "";
  while(getline(file,str)){
    if(str.empty())
      continue;
    vector<string> wordVec;
    lexOptions(str,wordVec,' ');

    string word = wordVec[0];
    wordVec.front() = std::move(wordVec.back());
    wordVec.pop_back();

    _wordMap[word] = wordVec;
  }
  file.close();
  return true;
}

bool dabMgr::loadLM(string fileName)
{
  File lmFile(fileName.c_str(),"r");
  _lm -> read(lmFile);
  lmFile.close();
  return true;
}

void dabMgr::disambig(string fileName)
{
  fstream file;
  file.open(fileName.c_str(),ios::in);
  if(!file.is_open())
    return;
  string str = "";
  while(getline(file,str)){
    if(str.empty())
      continue;
    vector<string> input;
    lexOptions(str,input,' ');
    vector<string> ans = viterbi(input);
    cout << "<s>" << " ";
    for(int i = 0; i < ans.size(); ++i)
      cout << ans[i] << " ";
    cout << "</s>" << endl;
  }
  file.close();
}

vector<string> dabMgr::viterbi(vector<string> input)
{
  vector<string> str;
  if(input.empty())
    return str;
  vector<vector<deltaUnit*> > deltas;
  {
    vector<string>& wordList = _wordMap[input[0]];
    vector<deltaUnit*> deltaList;

    for(int i = 0; i < wordList.size(); ++i){
      double prob = getUnigramProb(wordList[i].c_str());
      deltaUnit* delta = new deltaUnit(wordList[i],prob);
      deltaList.push_back(delta);
    }
    deltas.push_back(deltaList);
  }

  for(int n = 1; n < input.size(); ++n){
    vector<deltaUnit*>& prevList = deltas[deltas.size()-1];
    vector<string>& wordList = _wordMap[input[n]];
    vector<deltaUnit*> deltaList;

    for(int i = 0; i < wordList.size(); ++i){
      double maxProb = -DBL_MAX;
      deltaUnit* ptr = 0;
      for(int j = 0; j < prevList.size(); ++j){
        string prevWord = prevList[j] -> getWord();
        double prevDelta = prevList[j] -> delta();
        double prob = getBigramProb(prevWord.c_str(),wordList[i].c_str()) + prevDelta;
        if(prob > maxProb){
          maxProb = prob;
          ptr = prevList[j];
        }
      }
      deltaUnit* cur = new deltaUnit(wordList[i],maxProb,ptr);
      deltaList.push_back(cur);
    }
    deltas.push_back(deltaList);
  }

  vector<deltaUnit*>& list = deltas[deltas.size()-1];
  double maxProb = -DBL_MAX;
  deltaUnit* ptr = 0;
  for(int i = 0; i < list.size(); ++i){
    if(list[i] -> delta() > maxProb){
      maxProb = list[i] -> delta();
      ptr = list[i];
    }
  }

  while(ptr){
    str.push_back(ptr -> getWord());
    ptr = ptr -> getPrev();
  }
  reverse(str.begin(),str.end());

  for(int i = 0; i < deltas.size(); ++i){
    for(int j = 0; j < deltas[i].size(); ++j){
      delete deltas[i][j];
    }
  }

  return str;
}

double dabMgr::getUnigramProb(const char* w)
{
  VocabIndex wid = _voc -> getIndex(w);
  if(wid == Vocab_None)
    wid = _voc -> getIndex(Vocab_Unknown);
  VocabIndex context[] = {Vocab_None};
  return _lm -> wordProb(wid,context);
}

double dabMgr::getBigramProb(const char* w1,const char* w2)
{
  VocabIndex wid1 = _voc -> getIndex(w1);
  VocabIndex wid2 = _voc -> getIndex(w2);
  if(wid1 == Vocab_None)
    wid1 = _voc -> getIndex(Vocab_Unknown);
  if(wid2 == Vocab_None)
    wid2 = _voc -> getIndex(Vocab_Unknown);

  VocabIndex context[] = {wid1,Vocab_None};
  return _lm -> wordProb(wid2,context);
}

#endif