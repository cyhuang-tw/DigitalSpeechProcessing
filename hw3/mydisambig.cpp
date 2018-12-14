#include <string>
#include "dabMgr.h"

using namespace std;

dabMgr mgr;

int main(int argc,char *argv[])
{
  mgr.loadMap(string(argv[2]));
  mgr.loadLM(string(argv[3]));
  mgr.disambig(string(argv[1]));
  return 0;
}