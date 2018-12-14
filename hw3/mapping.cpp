#include <iostream>
#include <string>
#include "wordMgr.h"

using namespace std;

wordMgr mgr;

int main(int argc,char *argv[])
{
  string inName = argv[1];
  string outName = argv[2];

  mgr.readFile(inName);
  mgr.printMap(outName);
  return 0;
}