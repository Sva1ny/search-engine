#ifndef _KEYRECOMMANDER_H_
#define _KEYRECOMMANDER_H_

#include <queue>
#include <string>
#include <vector>
#include <map>
#include <set>
using namespace std;

// 存储查询结果
class MyResult
{
public:
    MyResult(string word, int freq, int dist = 999)
        : _word(word), _freq(freq), _dist(dist) {}

    string getWord() const { return _word; }
    int getFreq() const { return _freq; }
    int getDist() const { return _dist; }
    void setDist(int dist) { _dist = dist; }

private:
    string _word; // 关键词
    int _freq;    // 频率
    int _dist;    // 距离
};

// 优先队列中的比较规则
struct MyCompare
{
    bool operator()(const MyResult &lhs, const MyResult &rhs)
    {
        if (lhs.getDist() > rhs.getDist())
        {
            return true;
        }
        else if (lhs.getDist() == rhs.getDist())
        {
            if (lhs.getFreq() < rhs.getFreq())
            {
                return true;
            }
            else if (lhs.getFreq() == rhs.getFreq())
            {
                if (lhs.getWord() > rhs.getWord())
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
};

class KeyRecommander
{
public:
    // 构造函数, 初始化词典和索引
    KeyRecommander(const string &dictPath, const string &indexPath);
    // 根据查询词进行推荐,返回结果的JSON
    string doQuery(const string &query_word);

private:
    // 加载词典
    void loadDict(const string &dictPath);
    // 加载索引
    void loadIndex(const string &indexPath);

    // 计算一个字符的编码字节数
    // 1. 求取一个字符占据的字节数
    size_t nBytesCode(const char ch);

    // 计算一个字符串的长度，用于处理中英文混合的字符串
    // 2. 求取一个字符串的字符长度
    size_t length(const string &str);

    // 计算两个字符串之间的最小编辑距离
    // 3. 中英文通用的最小编辑距离算法
    int editDistance(const string &lhs, const string &rhs);

    // 返回三个数中的最小值
    int triple_min(const int &a, const int &b, const int &c);

    // 根据查询词统计匹配的索引ID和相关结果，存储在resultQue中
    void statistic(const string &queryWord, set<int> &indexId, priority_queue<MyResult, vector<MyResult>, MyCompare> &resultQue);

    // 将查询结果转换为JSON字符串格式
    string createJson(const vector<string> &results);

    // 当没有找到匹配结果时，返回一个标准的无答案JSON字符串
    string returnNoAnswer();

private:
    vector<pair<string, int>> _dict;
    map<string, set<int>> _index;
};

#endif