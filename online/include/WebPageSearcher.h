#ifndef _WEBPAGESEARCHER_H_
#define _WEBPAGESEARCHER_H_

#include "WebPage.h"
#include "WordSegmentation.h"

#include <string>
#include <vector>
#include <memory>
using namespace std;

class WebPageSearcher
{
public:
    WebPageSearcher();
    string doQuery(const string &query_word);

private:
    vector<double> getQueryWordsWeight(vector<string> &queryWords, const unordered_map<string, vector<pair<int, double>>> &invertIndex, size_t page_num);
    vector<int> getPages(vector<string> &queryWords, const unordered_map<string, vector<pair<int, double>>> &invertIndex);
    string returnNoAnswer();
    string createJson(vector<int> &docIdVec, const vector<string> &queryWords, const unordered_map<int, pair<size_t, size_t>> &offsetMap);

private:
    WordSegmentation _jieba;
};

#endif