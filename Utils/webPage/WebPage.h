#pragma ocne

#include "WordSegmentation.h"
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "simhash/Simhasher.hpp"

using std::cout;
using std::endl;
using std::istringstream;
using std::ostringstream;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

class WebPage
{
public:
    WebPage(int docid, string title, string link, string content, int size = 0, uint64_t simhashVal = 0);

    WebPage();

    string summary(const vector<string> &queryWords);
    int getSize();
    int getDocId();
    void setDocId(int id);
    string getTitle();
    string getContent();
    string getUrl();
    string getDoc() const;
    uint64_t getSimhash() const;
    void buildWordsMap(WordSegmentation &jieba);
    unordered_map<string, int> &getWordsMap();

private:
    size_t getBytes(const char);
    size_t length(const string &);
    string remove(const string &);
    bool isChinese(const string &str);
    bool isEnglish(const string &str);

public:
    int _docid;
    string _title;
    string _link;
    string _content;
    int _size;
    uint64_t _simhashVal;
    unordered_map<string, int> _wordsMap;
};

inline bool operator<(const WebPage &lhs, const WebPage &rhs)
{
    return lhs.getSimhash() < rhs.getSimhash();
}
inline bool operator==(const WebPage &lhs, const WebPage &rhs)
{
    return simhash::Simhasher::isEqual(lhs.getSimhash(), rhs.getSimhash());
}