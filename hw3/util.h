#ifndef __UTIL_H__
#define __UTIL_H__

#include <string>
#include <vector>

using namespace std;

size_t strGetTok(const string& str, string& tok, size_t pos = 0, const char del = '/')
{
   size_t begin = str.find_first_not_of(del, pos);
   if (begin == string::npos) { tok = ""; return begin; }
   size_t end = str.find_first_of(del, begin);
   tok = str.substr(begin, end - begin);
   return end;
}

void lexOptions(const string& option, vector<string>& tokens, const char del = '/')
{
   string token;
   size_t n = strGetTok(option, token, 0, del);
   while (token.size()) {
      tokens.push_back(token);
      n = strGetTok(option, token, n, del);
   }
   return;
}

#endif