#include "KeyRecommander.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <algorithm>

#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;

// 构造函数
KeyRecommander::KeyRecommander(const string &dict_path, const string &index_path)
{
    loadDict(dict_path);
    loadIndex(index_path);
}

// 加载词典
void KeyRecommander::loadDict(const string &dict_path)
{
    ifstream ifs(dict_path);
    if (!ifs.is_open())
    {
        cerr << "open " << dict_path << " failed!" << endl;
        return;
    }
    string line;
    while (getline(ifs, line))
    {
        string word;
        int freq;
        istringstream iss(line);
        iss >> word >> freq;
        _dict.emplace_back(word, freq);
    }
}
// 加载索引
void KeyRecommander::loadIndex(const string &index_path)
{
    ifstream ifs(index_path);
    if (!ifs.is_open())
    {
        cerr << "open " << index_path << " failed!" << endl;
        return;
    }
    string line;
    while (getline(ifs, line))
    {
        string word;
        int line_no;
        istringstream iss(line);
        iss >> word;
        while (iss >> line_no)
        {
            _index[word].insert(line_no);
        }
    }
}
// 计算一个字符的编码字节数
size_t KeyRecommander::nBytesCode(const char ch)
{
    // 检查字符的最高位是否为1，如果为1则可能是多字节编码字符
    if (ch & (1 << 7))
    {
        // 初始化字节数为1，因为已经确定最高位为1
        int nBytes = 1;
        // 从第6位开始向左检查连续的1的个数
        for (int idx = 0; idx != 6; ++idx)
        {
            // 检查第(6 - idx)位是否为1
            if (ch & (1 << (6 - idx)))
            {
                ++nBytes;
            }
            else
            {
                break;
            }
        }
        return nBytes;
    }
    // 若最高位为0，说明是单字节字符，返回1
    return 1;
}
// 计算一个字符串的长度，处理中英文混合的字符串
size_t KeyRecommander::length(const string &str)
{
    size_t len = 0;
    for (size_t idx = 0; idx != str.size(); ++idx)
    {
        // 计算当前字符的字节数
        int nBytes = nBytesCode(str[idx]);
        // 根据字节数调整索引位置
        idx += (nBytes - 1);
        ++len;
    }
    return len;
}
// 计算三个数中的最小值, 用于计算最小编辑距离
int KeyRecommander::triple_min(const int &a, const int &b, const int &c)
{
    return min(a, min(b, c));
}
// 动态规划求计算编辑距离
int KeyRecommander::editDistance(const string &lhs, const string &rhs)
{
    if (lhs == rhs)
    {
        return 0;
    }
    size_t lhs_len = length(lhs);
    size_t rhs_len = length(rhs);
    int edit_dist[lhs_len + 1][rhs_len + 1];
    for (size_t idx = 0; idx != lhs_len + 1; ++idx)
    {
        edit_dist[idx][0] = idx;
    }
    for (size_t idx = 0; idx != rhs_len + 1; ++idx)
    {
        edit_dist[0][idx] = idx;
    }
    string sub_lhs, sub_rhs;
    for (size_t dist_i = 1, lhs_idx = 0; dist_i != lhs_len + 1; ++dist_i, ++lhs_idx)
    {
        size_t nBytes = nBytesCode(lhs[lhs_idx]);
        sub_lhs = lhs.substr(lhs_idx, nBytes);
        lhs_idx += (nBytes - 1);
        for (size_t dist_j = 1, rhs_idx = 0; dist_j != rhs_len + 1; ++dist_j, ++rhs_idx)
        {
            nBytes = nBytesCode(rhs[rhs_idx]);
            sub_rhs = rhs.substr(rhs_idx, nBytes);
            rhs_idx += (nBytes - 1);
            if (sub_lhs == sub_rhs)
            {
                edit_dist[dist_i][dist_j] = edit_dist[dist_i - 1][dist_j - 1];
            }
            else
            {
                edit_dist[dist_i][dist_j] = triple_min(
                    edit_dist[dist_i][dist_j - 1] + 1,
                    edit_dist[dist_i - 1][dist_j] + 1,
                    edit_dist[dist_i - 1][dist_j - 1] + 1);
            }
        }
    }
    return edit_dist[lhs_len][rhs_len];
}
//
void KeyRecommander::statistic(const string &query_word, set<int> &indexId, priority_queue<MyResult, vector<MyResult>, MyCompare> &resultQue)
{
    for (auto it = indexId.begin(); it != indexId.end(); ++it)
    {
        MyResult result(_dict[*it].first, _dict[*it].second);
        int dist = editDistance(query_word, _dict[*it].first);
        result.setDist(dist);
        resultQue.push(result);
    }
}
// 根据查询词，查找索引文件，统计查询词与字典词的编辑距离，并返回最接近的关键词
string KeyRecommander::doQuery(const string &query_word)
{
    // 存储与查询词部分字符相关的词典条目索引
    set<int> indexId;
    vector<string> partialWords;

    for (size_t idx = 0; idx != query_word.size(); ++idx)
    {
        size_t nBytes = nBytesCode(query_word[idx]);
        string index = query_word.substr(idx, nBytes);
        partialWords.push_back(index);
        idx += (nBytes - 1);
        auto it = _index.find(index);
        if (it != _index.end())
        {
            indexId.insert(it->second.begin(), it->second.end());
        }
    }

    if (indexId.empty())
    {
        return returnNoAnswer();
    }

    priority_queue<MyResult, vector<MyResult>, MyCompare> resultQue;
    statistic(query_word, indexId, resultQue);

    vector<string> result;
    const int kTop = 5;
    while (!resultQue.empty() && result.size() < kTop)
    {
        string candidate = resultQue.top().getWord();
        resultQue.pop();
        bool relevant = false;
        for (const auto &word : partialWords)
        {
            if (candidate.find(word) != string::npos)
            {
                relevant = true;
                break;
            }
        }
        if (relevant)
        {
            result.push_back(candidate);
        }
    }
    return result.empty() ? returnNoAnswer() : createJson(result);
}
string KeyRecommander::createJson(const vector<string> &results)
{
    json root;
    root["msgID"] = 1;

    json msg = results;
    root["msg"] = msg;

    return root.dump(4);
}
string KeyRecommander::returnNoAnswer()
{
    json root;
    root["msgID"] = 1;

    vector<string> result = {"未能找到相关关键词"};
    json msg = result;
    root["msg"] = msg;

    return root.dump(4);
}